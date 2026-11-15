const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const os=require('node:os');
const path=require('node:path');
const {execFileSync}=require('node:child_process');

test('LED heartbeat produces 72 pulses per minute without timing drift or DAC state',()=>{
  const source=fs.readFileSync(path.join(__dirname,'../src/main.cpp'),'utf8');
  const functions=['gaussian','ppgWaveform','onHeartbeatTimer'].map(name=>{
    const match=source.match(new RegExp(`static (?:float|void) ${name}\\([^]*?\\n}`));
    assert(match,`${name} must be present`);
    return match[0];
  }).join('\n');
  const bpm=source.match(/static constexpr uint32_t HEARTBEAT_BPM = \d+;/)[0];
  const pwm=source.match(/static constexpr uint8_t HEARTBEAT_PWM_CHANNEL = \d+;/)[0];
  const program=`
#include <cassert>
#include <cmath>
#include <cstdint>
${bpm}
${pwm}
static int64_t nowUs;
static uint32_t brightness;
static int64_t esp_timer_get_time() { return nowUs; }
static void ledcWrite(uint8_t channel, uint32_t duty) {
  assert(channel == HEARTBEAT_PWM_CHANNEL);
  assert(duty >= 10 && duty <= 255);
  brightness = duty;
}
${functions}
int main() {
  int pulses = 0;
  bool wasBright = false;
  for (nowUs = 0; nowUs < 60000000; nowUs += 10000) {
    onHeartbeatTimer(nullptr);
    const bool bright = brightness > 200;
    if (bright && !wasBright) ++pulses;
    wasBright = bright;
  }
  assert(pulses == 72);
  for (int64_t time = 0; time < 2500000; time += 10000) {
    nowUs = time;
    onHeartbeatTimer(nullptr);
    const uint32_t expected = brightness;
    nowUs = time + 2500000;
    onHeartbeatTimer(nullptr);
    assert(brightness == expected);
    nowUs = time + 60000000LL * 60 * 24 * 365;
    onHeartbeatTimer(nullptr);
    assert(brightness == expected);
  }
}
`;
  const directory=fs.mkdtempSync(path.join(os.tmpdir(),'esp32-heartbeat-'));
  try{
    const executable=path.join(directory,'heartbeat');
    execFileSync(process.env.CXX||'c++',['-std=c++11','-x','c++','-o',executable,'-'],{input:program});
    execFileSync(executable);
  }finally{
    fs.rmSync(directory,{recursive:true,force:true});
  }
});
