const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const os=require('node:os');
const path=require('node:path');
const {execFileSync}=require('node:child_process');

test('DMA producer preserves stereo, frequency, output state, and timed faults',()=>{
  const source=fs.readFileSync(path.join(__dirname,'../src/main.cpp'),'utf8');
  const extract=(pattern)=>{const match=source.match(pattern);assert(match,pattern);return match[0]};
  const types=[/enum FaultMode[^]*?};/,/struct FaultConfig[^]*?};/,/struct ChannelConfig[^]*?\n};/].map(extract).join('\n');
  const functions=['gaussian','ppgWaveform','normalizedWaveform','sampleToDacCode','waveformIndex','dacStreamTask'].map(name=>extract(new RegExp(`static (?:float|uint8_t|void) ${name}\\([^]*?\\n}`))).join('\n');
  const waveforms=extract(/static const char \*const WAVEFORMS\[\][^]*?;/);
  const program=`
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>
#include "dac_stream.h"
#define PI 3.14159265358979323846
#define portENTER_CRITICAL(mux) ((void)0)
#define portEXIT_CRITICAL(mux) ((void)0)
#define pdMS_TO_TICKS(value) (value)
static constexpr float DAC_FULL_SCALE_V = 3.3f;
static float constrain(float x,float lo,float hi) {return fminf(hi,fmaxf(lo,x));}
${types}
${waveforms}
static ChannelConfig channelConfig[2];
static FaultConfig faultConfig[2];
static float waveformTables[6][DAC_TABLE_SIZE];
static uint32_t sampleTicks, streamWriteErrors;
static uint8_t lastDacCodes[2];
static bool sampleTimerStarted;
static int configMux;
static constexpr int I2S_NUM_0=0, ESP_OK=0, I2S_DAC_CHANNEL_DISABLE=0;
using esp_err_t=int;
struct {template<typename... T> void printf(const char*,T...) {}} Serial;
static std::vector<uint32_t> output;
static unsigned blocks, limit;
static bool failWrite;
struct Finished {};
static int64_t esp_timer_get_time() {return static_cast<int64_t>(blocks)*DAC_BLOCK_FRAMES*1000000/DAC_SAMPLE_RATE_HZ;}
static int i2s_write(int,const void* data,size_t length,size_t* written,unsigned) {
  if(blocks==limit)throw Finished{};
  const auto* frames=static_cast<const uint32_t*>(data);
  output.insert(output.end(),frames,frames+length/4);
  ++blocks;
  *written=failWrite?length/2:length;
  return ESP_OK;
}
static void i2s_stop(int) {}
static void i2s_set_dac_mode(int) {}
static void dacWrite(uint8_t,uint8_t) {}
static const uint8_t DAC_PINS[]={25,26};
static void vTaskDelete(void*) {throw Finished{};}
${functions}
static void run(unsigned count) {
  output.clear(); blocks=0;limit=count;sampleTicks=0;streamWriteErrors=0;sampleTimerStarted=true;
  try {dacStreamTask(nullptr);} catch(const Finished&) {}
}
static uint8_t ch1(uint32_t frame) {return frame>>24;}
static uint8_t ch2(uint32_t frame) {return (frame>>8)&255;}
int main() {
  static_assert(DAC_SAMPLE_RATE_HZ >= 10*20000,"At least ten samples per cycle at 20 kHz");
  static_assert(DAC_MAX_FREQUENCY_HZ == 20000,"Audio endpoint");
  assert(packDacFrame(0x12,0x34)==0x12003400);
  for(float frequency : {.5f,1.2f,10.0f,440.0f,1000.0f,19999.9f,20000.0f}) {
    DacPhase phase;phase.setFrequency(frequency);
    unsigned wraps=0;
    const unsigned samples=4*DAC_SAMPLE_RATE_HZ;
    for(unsigned n=0;n<samples;++n) {assert(phase.index()<DAC_TABLE_SIZE);wraps+=phase.advance();}
    assert(std::abs(static_cast<double>(wraps)-frequency*4)<=1);
    assert(std::abs(static_cast<double>(phase.step)*DAC_SAMPLE_RATE_HZ/4294967296.0-frequency)<.00003);
  }
  for(unsigned s=0;s<6;++s)for(unsigned n=0;n<DAC_TABLE_SIZE;++n)
    waveformTables[s][n]=normalizedWaveform(WAVEFORMS[s],static_cast<float>(n)/DAC_TABLE_SIZE);
  channelConfig[0]={"dc",1000,100,50,false};
  channelConfig[1]={"dc",20000,100,25,false};
  run(4);for(auto frame:output)assert(frame==0);
  channelConfig[0].enabled=channelConfig[1].enabled=true;
  run(4);for(auto frame:output) {assert(ch1(frame)==128);assert(ch2(frame)==64);assert((frame&0x00ff00ff)==0);}
  channelConfig[0]={"square",1000,100,50,true};
  channelConfig[1]={"sine",20000,100,50,true};
  run(782);
  unsigned rises1=0,rises2=0;
  for(size_t n=1;n<output.size();++n) {
    rises1+=ch1(output[n])>128&&ch1(output[n-1])<=128;
    rises2+=ch2(output[n])>128&&ch2(output[n-1])<=128;
  }
  const double seconds=static_cast<double>(output.size())/DAC_SAMPLE_RATE_HZ;
  assert(std::abs(rises1-seconds*1000)<=1);
  assert(std::abs(rises2-seconds*20000)<=1);
  assert(sampleTicks==output.size());
  channelConfig[0]={"ppg",1000,100,50,true};
  channelConfig[1].enabled=false;
  for(FaultMode mode : {FAULT_NONE,FAULT_REDUCED_AMPLITUDE,FAULT_BASELINE_SHIFT,FAULT_DROPOUT,FAULT_MISSING_PULSE}) {
    faultConfig[0]={mode,1000000,0,true,false};
    run(8);
    for(size_t n=0;n<output.size();++n) {
      ChannelConfig expected=channelConfig[0];
      if(mode==FAULT_REDUCED_AMPLITUDE)expected.amplitudePercent*=.5f;
      if(mode==FAULT_BASELINE_SHIFT)expected.offsetPercent=60;
      DacPhase phase;phase.setFrequency(expected.frequencyHz);
      phase.value=static_cast<uint32_t>(static_cast<uint64_t>(phase.step)*n);
      const uint64_t beat=(static_cast<uint64_t>(phase.step)*n)>>32;
      const bool skip=mode==FAULT_MISSING_PULSE&&beat>0&&beat%3==0;
      const uint8_t value=mode==FAULT_DROPOUT?0:sampleToDacCode(expected,skip?.05f:waveformTables[4][phase.index()]);
      assert(ch1(output[n])==value);assert(ch2(output[n])==0);
    }
  }
  // Expired faults must not affect the following block snapshots.
  faultConfig[0]={FAULT_DROPOUT,1,0,true,false};
  run(4);
  for(unsigned n=0;n<DAC_BLOCK_FRAMES;++n)assert(ch1(output[n])==0);
  bool recovered=false;for(size_t n=DAC_BLOCK_FRAMES;n<output.size();++n)recovered|=ch1(output[n])>100;
  assert(recovered);
  failWrite=true;run(4);assert(!sampleTimerStarted);assert(streamWriteErrors==1);
}
`;
  const directory=fs.mkdtempSync(path.join(os.tmpdir(),'esp32-audio-'));
  try{
    const executable=path.join(directory,'audio');
    execFileSync(process.env.CXX||'c++',['-std=c++11','-O2','-I',path.join(__dirname,'../include'),'-x','c++','-o',executable,'-'],{input:program});
    execFileSync(executable);
  }finally{fs.rmSync(directory,{recursive:true,force:true})}
});
