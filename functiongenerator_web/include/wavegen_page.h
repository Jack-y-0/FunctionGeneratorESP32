#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"WAVEGENHTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <meta name="theme-color" content="#f4f7fb">
  <title>ESP32 Wave Generator</title>
  <style>
    :root{font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;color:#162238;background:#f4f7fb;font-synthesis:none}
    *{box-sizing:border-box}body{margin:0;min-width:320px}.wrap{width:100%;max-width:none;margin:auto;padding:24px clamp(14px,2.2vw,36px) 48px}#generatorView{max-width:1050px;margin:0 auto}
    header{display:flex;justify-content:space-between;align-items:center;gap:20px;margin:0 0 24px}
    h1{font-size:clamp(1.5rem,4vw,2.2rem);margin:0;letter-spacing:-.04em}.sub{color:#607089;margin:7px 0 0;font-size:.95rem}
    .badge{display:flex;align-items:center;flex-wrap:wrap;max-width:100%;gap:8px;padding:9px 12px;border:1px solid #dce4ef;border-radius:16px;background:#fff;color:#53637b;font-size:.82rem;white-space:nowrap}
    .dot{width:9px;height:9px;border-radius:50%;background:#d39135}.dot.online{background:#27a776;box-shadow:0 0 0 4px #e4f6ef}
    .viewtabs{display:flex;gap:10px;margin:0 0 18px}.viewtabs button{border:1px solid #ced8e6;background:#fff;color:#42536e;border-radius:10px;padding:10px 16px;font-weight:700}.viewtabs button.active{background:#29416e;color:#fff;border-color:#29416e}
    .scopegrid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}.wide-panel{background:#fff;border:1px solid #dce4ef;border-radius:16px;padding:16px;margin:0 0 18px}.wide-panel canvas{width:100%;height:min(52vh,520px);min-height:300px;border:1px solid #e1e8f1;border-radius:10px;display:block}.plot-tools{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap;margin:8px 0 12px}.plot-tools label{font-weight:650;color:#45556d}.plot-tools select{width:auto;min-width:130px}.channel-picks{display:flex;gap:8px;flex-wrap:wrap;margin:8px 0 14px}.channel-pick{display:inline-flex;align-items:center;gap:6px;border:1px solid #d7e0ec;border-radius:9px;background:#f9fbfd;padding:7px 10px;color:#43536b;font-size:.84rem}.channel-pick input{accent-color:#426de0}.legend{display:flex;gap:14px;flex-wrap:wrap;color:#53637b;font-size:.83rem;margin-top:8px}.legend span{display:inline-flex;align-items:center;gap:6px}.legend i{width:12px;height:4px;border-radius:3px;display:inline-block}.scope-mode{display:flex;gap:8px;margin:12px 0}.scope-mode button{border:1px solid #ced8e6;background:#fff;color:#42536e;border-radius:9px;padding:8px 13px;font-weight:700}.scope-mode button.active{background:#29416e;color:#fff;border-color:#29416e}.scope-card{background:#fff;border:1px solid #dce4ef;border-radius:16px;padding:16px;min-width:0}.scope-card h3{margin:0 0 8px}.scope-card canvas{display:block;width:100%;height:150px;border:1px solid #e1e8f1;border-radius:10px;background:linear-gradient(#fff,#fbfdff)}.scope-meta{color:#64748b;font-size:.9rem;margin-top:8px}.scope-enable{display:flex;align-items:center;gap:8px;margin:8px 0}.scope-section-title{margin:22px 0 8px}
    @media(max-width:760px){.scopegrid{grid-template-columns:1fr}.viewtabs{position:sticky;top:0;background:#f4f7fb;padding:8px 0;z-index:2}}
    .notice{padding:12px 15px;background:#fff8e8;border:1px solid #f4dfb5;border-radius:12px;color:#755721;font-size:.9rem;line-height:1.5;margin-bottom:18px}
    .grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:18px}.card{background:white;border:1px solid #e0e7f0;border-radius:18px;padding:18px;box-shadow:0 8px 26px #1a315008}
    .cardhead{display:flex;justify-content:space-between;align-items:center;margin-bottom:12px}.cardhead h2{margin:0;font-size:1.13rem}.pin{font-size:.8rem;color:#61718a;background:#f0f4f9;border-radius:7px;padding:5px 8px}
    canvas{display:block;width:100%;height:150px;border:1px solid #e5ebf3;border-radius:12px;background:linear-gradient(#fff,#fbfdff)}
    .controls{display:grid;grid-template-columns:1fr 1fr;gap:14px 16px;margin-top:16px}.field label{display:flex;justify-content:space-between;align-items:center;gap:8px;font-size:.84rem;font-weight:600;color:#45556d;margin-bottom:7px}.value{font-variant-numeric:tabular-nums;color:#17253c;font-weight:700}
    select{width:100%;height:40px;border:1px solid #cfd8e5;border-radius:9px;padding:0 10px;background:#fff;color:#18263c;font:inherit}
    input[type=range]{width:100%;accent-color:#426de0;margin:7px 0 0}.rangeends{display:flex;justify-content:space-between;color:#8a98aa;font-size:.72rem;margin-top:2px}
    .toggle{display:flex;align-items:center;gap:9px;font-size:.84rem;font-weight:600;color:#45556d}.toggle input{width:18px;height:18px;accent-color:#258e68}.presetbar{display:flex;align-items:center;gap:12px;flex-wrap:wrap;margin:0 0 18px}.presetbar button{border:0;border-radius:9px;padding:10px 14px;background:#263b61;color:white;font:600 .88rem system-ui;cursor:pointer}.presetbar button:hover{background:#345181}.presetbar span{font-size:.82rem;color:#687890}input:disabled{opacity:.45;cursor:not-allowed}.wave-buttons{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:7px}.wavebtn{border:1px solid #cfd8e5;border-radius:8px;padding:9px 6px;background:#fff;color:#43536b;font:600 .78rem system-ui;cursor:pointer}.wavebtn.active{background:#426de0;border-color:#426de0;color:#fff}.field.wide{grid-column:1/-1}
    .fault-actions{display:flex;gap:8px;align-items:center;margin-top:8px}.fault-actions input{width:92px;height:38px;border:1px solid #cfd8e5;border-radius:8px;padding:0 8px}.fault-actions button{border:0;border-radius:8px;padding:10px 12px;background:#a34c37;color:white;font:600 .82rem system-ui;cursor:pointer}.fault-status{font-size:.78rem;color:#687890;margin-top:6px;min-height:1em}.eventlog{margin-top:18px}.eventlog ol{padding-left:22px;margin-bottom:0;color:#52627a;font-size:.82rem;line-height:1.8}.eventlog li{font-variant-numeric:tabular-nums}
    .foot{margin-top:18px;color:#687890;font-size:.82rem;line-height:1.55}.foot strong{color:#465770}
    .device-info-button{border:1px solid #ced8e6;border-radius:8px;background:#fff;color:#29416e;padding:6px 10px;cursor:pointer;font:inherit}
    .splash{width:min(520px,calc(100% - 28px));max-height:calc(100dvh - 28px);overflow:auto;border:1px solid #dce4ef;border-radius:20px;padding:clamp(20px,5vw,32px);color:#162238;box-shadow:0 24px 80px #16223830}
    .splash::backdrop{background:#16223899}.splash h2{font-size:1.7rem;letter-spacing:-.03em;margin:0 0 8px}.splash dl{margin:24px 0}.splash dt{color:#607089;font-size:.82rem;margin-top:16px}.splash dd{margin:4px 0 0;font-weight:650;overflow-wrap:anywhere;white-space:pre-wrap}.splash .sub{line-height:1.5}.splash button{border:0;border-radius:10px;background:#29416e;color:#fff;padding:12px 18px;font:600 .95rem system-ui;cursor:pointer;width:100%}
    @media(max-width:720px){header{align-items:flex-start;flex-direction:column}.grid{grid-template-columns:1fr}.controls{gap:12px}}
  </style>
</head>
<body>
  <dialog class="splash" id="deviceSplash" aria-labelledby="splashTitle" aria-describedby="splashStatus">
    <h2 id="splashTitle">ESP32 Wave Generator</h2>
    <p class="sub" id="splashStatus" role="status" aria-live="polite">Reading device information…</p>
    <dl>
      <dt>Program name</dt><dd id="splashProgram">Loading…</dd>
      <dt>Patch version</dt><dd id="splashVersion">Loading…</dd>
      <dt>Date compiled</dt><dd id="splashCompiled">Loading…</dd>
      <dt>MAC address (Wi-Fi station)</dt><dd id="splashMac">Loading…</dd>
      <dt>Connected Wi-Fi</dt><dd id="splashWifi">Loading…</dd>
    </dl>
    <form method="dialog"><button autofocus type="submit">Continue to generator</button></form>
  </dialog>
  <main class="wrap">
    <header>
      <div><h1>ESP32 Wave Generator</h1><p class="sub">Two independently controlled DAC outputs, including a synthetic PPG pulse.</p></div>
      <div class="badge"><span class="dot" id="connectionDot"></span><span id="connectionText">Connecting…</span><span id="firmwareBuild"></span><button class="device-info-button" id="deviceInfo" type="button">Device info</button></div>
    </header>
    <nav class="viewtabs" aria-label="Device view">
      <button id="generatorTab" class="active" type="button">Wave generator</button>
      <button id="scopeTab" type="button">Oscilloscope inputs</button>
    </nav>
    <section id="generatorView">
    <div class="notice"><strong>DAC outputs:</strong> Pin numbers depend on the selected PlatformIO board target and are shown on each channel card. Output is unipolar and nominally 0–3.3 V. The built-in DAC is 8-bit and is for low-frequency signal testing, not a calibrated instrument output.</div>
    <div class="presetbar"><button id="dcPreset" type="button">Load DC midpoint test</button><span>DAC1 = 128/255; DAC2 = 64/255 · enable each channel below</span></div>
    <section class="grid" id="channels"></section>
    <section class="card eventlog"><h2>Fault event log</h2><ol id="eventLog"><li>No fault events yet.</li></ol><p class="sub">Times are elapsed since device boot; the log is kept in RAM and clears after reboot.</p></section>
    <p class="foot"><strong>Wiring:</strong> connect the DAC pins shown on the channel cards to high-impedance ADC inputs on the receiving board, and connect both boards’ GND pins together. Do not connect the output to equipment that expects negative voltage, voltage above 3.3 V, or a low-impedance load. USB Serial commands: `ch1` or `ch2` selects a channel, then `a0`–`a5` selects its waveform.</p>
    </section>
    <section id="scopeView" hidden>
      <div class="notice"><strong>Input limits:</strong> ADC inputs accept 0–3.3 V maximum and are not 5 V tolerant. Never connect negative voltage or a signal above 3.3 V directly. Use a properly protected attenuator/buffer for external signals. ADC plots show raw 12-bit counts; voltage is approximate and uncalibrated.</div>
      <h2 class="scope-section-title">Input scope · selectable channels</h2>
      <p class="sub">Choose analog ADC1 or digital PWM inputs, then select one or more pins to overlay on the full-width display. ADC traces use raw 12-bit counts and are sampled sequentially at about 200 frames/s per pin. PWM traces are reconstructed from edge timing and are intended for low-frequency checks.</p>
      <div class="scope-mode"><button id="analogMode" class="active" type="button">Analog ADC1</button><button id="digitalMode" type="button">Digital PWM</button></div>
      <section class="wide-panel">
        <div class="plot-tools"><div id="scopeSelectorLabel">Select ADC inputs to display</div><label for="scopeTimebase">Time window <select id="scopeTimebase"><option value="0.1">0.1 s</option><option value="0.25">0.25 s</option><option value="0.5">0.5 s</option><option value="1" selected>1 s</option><option value="1.28">1.28 s</option><option value="2">2 s</option><option value="5">5 s</option></select></label></div>
        <div class="channel-picks" id="scopeChannelPicks"></div>
        <canvas id="inputScopePlot" width="1600" height="520" aria-label="Selected analog or PWM input traces"></canvas>
        <div class="legend" id="scopeLegend"></div>
      </section>
      <p class="foot" id="scopeStatus">Waiting for scope samples…</p>
    </section>
  </main>
<script>
const deviceSplash=document.getElementById('deviceSplash');
function updateDeviceInfo(state){
  const fields={splashProgram:state.programName,splashVersion:state.firmware,splashCompiled:state.compiledAt,splashMac:state.macAddress};
  Object.entries(fields).forEach(([id,value])=>{document.getElementById(id).textContent=typeof value==='string'&&value?value:'Not reported by firmware'});
  document.getElementById('splashWifi').textContent=state.wifiConnected===true?(state.wifiSsid||'Connected (SSID unavailable)'):state.wifiConnected===false?'Not connected':'Not reported by firmware';
  document.getElementById('splashStatus').textContent='Device information · compile time uses the build machine’s clock.';
}
function deviceInfoUnavailable(message){
  ['splashProgram','splashVersion','splashCompiled','splashMac','splashWifi'].forEach(id=>{document.getElementById(id).textContent='Unavailable'});
  document.getElementById('splashStatus').textContent=message;
}
document.getElementById('deviceInfo').addEventListener('click',()=>{deviceSplash.showModal();loadState()});
deviceSplash.showModal();
const channelDefaults=[
  {name:'Channel 1',pin:0,waveform:'ppg',frequency:1.2,amplitude:90,offset:50,enabled:false},
  {name:'Channel 2',pin:0,waveform:'ppg',frequency:1.2,amplitude:45,offset:50,enabled:false}
];
let channels=channelDefaults.map(c=>({...c}));
let pendingUpdates=[false,false],channelRevision=[0,0];
const shapes=['sine','square','triangle','sawtooth','ppg','dc'];
const shapeLabel={sine:'Sine',square:'Square',triangle:'Triangle',sawtooth:'Sawtooth',ppg:'PPG pulse',dc:'DC level'};
const commandWave={a0:'sine',a1:'square',a2:'triangle',a3:'sawtooth',a4:'ppg',a5:'dc'};
const card=(c,i)=>`<article class="card" data-channel="${i}">
  <div class="cardhead"><h2>${c.name}</h2><span class="pin" id="pinLabel${i}">Loading pin…</span></div>
  <canvas id="plot${i}" width="700" height="220" aria-label="${c.name} waveform preview"></canvas>
  <div class="controls">
    <div class="field wide"><label>Choose waveform · command IDs</label><div class="wave-buttons">${shapes.map((s,j)=>`<button type="button" class="wavebtn" data-command="a${j}">a${j} · ${shapeLabel[s]}</button>`).join('')}</div></div>
    <div class="field"><label for="freq${i}">Frequency <span class="value"><span id="freqv${i}">${c.frequency.toFixed(1)}</span> Hz <span id="bpm${i}"></span></span></label><input id="freq${i}" type="range" min="0.5" max="10" step="0.1" value="${c.frequency}"><div class="rangeends"><span>0.5 Hz</span><span>10 Hz</span></div></div>
    <div class="field"><label for="amp${i}">Amplitude <span class="value"><span id="ampv${i}">${c.amplitude}</span>% Vpp</span></label><input id="amp${i}" type="range" min="0" max="100" step="1" value="${c.amplitude}"><div class="rangeends"><span>0% Vpp</span><span>100% Vpp</span></div><div class="fault-status" id="amplitudeHint${i}"></div></div>
    <div class="field"><label for="offset${i}">Offset <span class="value"><span id="offsetv${i}">${c.offset}</span>%</span></label><input id="offset${i}" type="range" min="0" max="100" step="1" value="${c.offset}"><div class="rangeends"><span>0 V</span><span>~3.3 V</span></div></div>
    <label class="toggle"><input id="enable${i}" type="checkbox" ${c.enabled?'checked':''}> Output enabled</label><div class="fault-status" id="channelStatus${i}" role="status" aria-live="polite"></div><div class="fault-status" id="signalDiag${i}" aria-live="polite">Waiting for timer diagnostics…</div>
    <div class="field wide"><label for="fault${i}">Timed fault scenario</label><select id="fault${i}"><option value="none">Stop fault</option><option value="missing_pulse">Miss every 3rd PPG pulse</option><option value="reduced_amplitude">Reduce amplitude by 50%</option><option value="baseline_shift">Shift baseline upward by 10%</option><option value="dropout">Drop output to 0 V</option></select><div class="fault-actions"><input id="faultDuration${i}" type="number" min="1" max="600" value="10" aria-label="Fault duration in seconds"><span>seconds</span><button type="button" id="faultStart${i}">Start scenario</button></div><div class="fault-status" id="faultStatus${i}"></div></div>
  </div>
</article>`;
document.getElementById('channels').innerHTML=channels.map(card).join('');
function gaussian(x,c,w){const z=(x-c)/w;return Math.exp(-.5*z*z)}
function normWave(shape,p){if(shape==='dc')return .5;if(shape==='square')return p<.5?1:0;if(shape==='triangle')return 1-Math.abs(2*p-1);if(shape==='sawtooth')return p;if(shape==='ppg'){const pulse=Math.max(0,gaussian(p,.18,.065)+.18*gaussian(p,.43,.025)-.07*gaussian(p,.48,.03));return .05+.95*Math.min(1,pulse)}return .5+.5*Math.sin(2*Math.PI*p)}
const previewWindowSeconds=2,previewPhase=[0,0];
function draw(i){const c=channels[i],canvas=document.getElementById(`plot${i}`),ctx=canvas.getContext('2d'),w=canvas.width,h=canvas.height;ctx.clearRect(0,0,w,h);ctx.strokeStyle='#e7edf5';ctx.lineWidth=1;for(let y=30;y<h;y+=40){ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke()}ctx.strokeStyle=c.enabled?'#426de0':'#aab5c4';ctx.lineWidth=3;ctx.beginPath();for(let x=0;x<w;x++){const p=(previewPhase[i]+x/w*previewWindowSeconds*c.frequency)%1,rawV=c.offset/100+(normWave(c.waveform,p)-.5)*(c.amplitude/100),v=c.enabled?Math.max(0,Math.min(1,rawV)):0;const y=(1-v)*h*.72+h*.14;if(x===0)ctx.moveTo(x,y);else ctx.lineTo(x,y)}ctx.stroke();ctx.fillStyle='#64748b';ctx.font='14px system-ui';const label=c.waveform==='dc'?'DC level':c.waveform==='ppg'?`${c.frequency.toFixed(1)} Hz · ${Math.round(c.frequency*60)} bpm · synthetic PPG`:`${c.frequency.toFixed(1)} Hz · ${shapeLabel[c.waveform]}`;ctx.fillText(c.enabled?label:'Output off',12,20);ctx.font='12px system-ui';ctx.fillText('Simulated preview · 2 s window',12,h-8)}
function render(i){const c=channels[i],dc=c.waveform==='dc';document.getElementById(`freq${i}`).disabled=dc;document.getElementById(`amp${i}`).disabled=dc;document.getElementById(`freqv${i}`).textContent=dc?'—':c.frequency.toFixed(1);document.getElementById(`ampv${i}`).textContent=dc?'—':Math.round(c.amplitude);document.getElementById(`amplitudeHint${i}`).textContent=!dc&&c.amplitude===0?'0% amplitude: this waveform will be flat':'';document.getElementById(`offsetv${i}`).textContent=Math.round(c.offset);document.getElementById(`bpm${i}`).textContent=c.waveform==='ppg'?`(${Math.round(c.frequency*60)} bpm)`:'';document.querySelectorAll(`[data-channel="${i}"] .wavebtn`).forEach(b=>b.classList.toggle('active',commandWave[b.dataset.command]===c.waveform));draw(i)}
let timers=[],requestQueues=[Promise.resolve(),Promise.resolve()],dirtyFields=[new Set(),new Set()],previousTimerTicks=null;
const waveformCommand={sine:'a0',square:'a1',triangle:'a2',sawtooth:'a3',ppg:'a4',dc:'a5'};
async function send(i,fields){const c={...channels[i]},revision=channelRevision[i],params=new URLSearchParams({channel:String(i)});fields.forEach(field=>{if(field==='waveform')params.set('command',waveformCommand[c.waveform]);else if(field==='frequency')params.set(field,c.frequency.toFixed(1));else if(field==='amplitude')params.set(field,String(Math.round(c.amplitude)));else if(field==='offset')params.set(field,String(Math.round(c.offset)));else if(field==='enabled')params.set(field,c.enabled?'1':'0')});pendingUpdates[i]=true;const request=requestQueues[i].catch(()=>{}).then(async()=>{const r=await fetch('/api/channel',{credentials:'same-origin',method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:params.toString()});const data=await r.json().catch(()=>({}));if(!r.ok)throw new Error(data.error||('HTTP '+r.status));if(data.channel!==i)throw new Error('ESP32 replied for the wrong channel');for(const field of fields){if(field==='waveform'&&data.waveform!==c.waveform)throw new Error('Waveform was not applied');if(field==='frequency'&&Math.abs(Number(data.frequency)-Number(c.frequency.toFixed(1)))>0.051)throw new Error('Frequency was not applied');if(field==='amplitude'&&Number(data.amplitude)!==Math.round(c.amplitude))throw new Error('Amplitude was not applied');if(field==='offset'&&Number(data.offset)!==Math.round(c.offset))throw new Error('Offset was not applied');if(field==='enabled'&&data.enabled!==c.enabled)throw new Error('Output enable was not applied')}return data});requestQueues[i]=request.then(()=>undefined,()=>undefined);try{await request;if(revision===channelRevision[i]){pendingUpdates[i]=false;document.getElementById(`channelStatus${i}`).textContent='Settings applied'}}catch(e){if(revision===channelRevision[i]){pendingUpdates[i]=false;document.getElementById(`channelStatus${i}`).textContent='Update failed: '+e.message}setOnline(false)}}
function changed(i,immediate=false,fields=[]){channelRevision[i]++;fields.forEach(field=>dirtyFields[i].add(field));pendingUpdates[i]=true;render(i);clearTimeout(timers[i]);const flush=()=>{const batch=[...dirtyFields[i]];dirtyFields[i].clear();send(i,batch)};if(immediate)flush();else timers[i]=setTimeout(flush,140)}
channels.forEach((_,i)=>{document.querySelectorAll(`[data-channel="${i}"] .wavebtn`).forEach(button=>button.addEventListener('click',()=>{channels[i].waveform=commandWave[button.dataset.command];changed(i,true,['waveform'])}));document.getElementById(`freq${i}`).addEventListener('input',e=>{channels[i].frequency=Number(e.target.value);changed(i,false,['frequency'])});document.getElementById(`amp${i}`).addEventListener('input',e=>{channels[i].amplitude=Number(e.target.value);changed(i,false,['amplitude'])});document.getElementById(`offset${i}`).addEventListener('input',e=>{channels[i].offset=Number(e.target.value);changed(i,false,['offset'])});document.getElementById(`enable${i}`).addEventListener('change',e=>{channels[i].enabled=e.target.checked;changed(i,true,['enabled'])});document.getElementById(`faultStart${i}`).addEventListener('click',()=>sendFault(i));render(i)});
async function sendFault(i){const mode=document.getElementById(`fault${i}`).value,duration=document.getElementById(`faultDuration${i}`).value,params=new URLSearchParams({channel:String(i),mode,duration});try{const r=await fetch('/api/fault',{credentials:'same-origin',method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:params.toString()});const data=await r.json();if(!r.ok)throw new Error(data.error||('HTTP '+r.status));document.getElementById(`faultStatus${i}`).textContent=mode==='none'?'Fault stopped.':'Scenario started.';setOnline(true);loadEvents()}catch(e){document.getElementById(`faultStatus${i}`).textContent='Scenario failed: '+e.message;setOnline(false)}}
async function loadEvents(){try{const r=await fetch('/api/events',{credentials:'same-origin'});if(!r.ok)throw new Error();const data=await r.json(),list=document.getElementById('eventLog');if(!data.events.length){list.innerHTML='<li>No fault events yet.</li>';return}list.innerHTML=data.events.slice().reverse().map(e=>`<li>+${(e.uptimeMs/1000).toFixed(1)} s · Channel ${e.channel} · ${e.scenario.replaceAll('_',' ')} · ${e.action}</li>`).join('')}catch(e){}}
function setOnline(online){document.getElementById('connectionDot').classList.toggle('online',online);document.getElementById('connectionText').textContent=online?'ESP32 connected':'Connection lost'}
async function loadState(){const requestRevisions=channelRevision.slice();try{const r=await fetch('/api/state',{credentials:'same-origin'});if(r.status===401){deviceInfoUnavailable('Web login rejected; reload and sign in as admin.');document.getElementById('firmwareBuild').textContent='· web login rejected (401)';document.querySelectorAll('[id^=signalDiag]').forEach(el=>el.textContent='API authentication failed; reload the page and sign in as admin');setOnline(false);return}if(!r.ok)throw new Error('HTTP '+r.status);const state=await r.json();updateDeviceInfo(state);document.getElementById('firmwareBuild').textContent=state.firmware?'· '+state.firmware:'· firmware ID missing';const hadPriorTicks=previousTimerTicks!==null,timerAdvancing=hadPriorTicks&&Number(state.timerTicks)!==previousTimerTicks;previousTimerTicks=Number(state.timerTicks);state.channels.forEach((s,i)=>{document.getElementById(`pinLabel${i}`).textContent=`GPIO${state.pins[i]} · DAC`;if(pendingUpdates[i]||requestRevisions[i]!==channelRevision[i])return;channels[i]={...channels[i],...s,pin:state.pins[i]};document.getElementById(`freq${i}`).value=channels[i].frequency;document.getElementById(`amp${i}`).value=channels[i].amplitude;document.getElementById(`offset${i}`).value=channels[i].offset;document.getElementById(`enable${i}`).checked=channels[i].enabled;const diag=document.getElementById(`signalDiag${i}`);if(typeof state.timerRunning!=='boolean'||typeof state.timerTicks!=='number')diag.textContent='Firmware is missing signal diagnostics; upload the latest build';else if(!state.timerRunning)diag.textContent='DAC sample timer did not start';else if(!hadPriorTicks)diag.textContent='Timer started; checking activity…';else if(!timerAdvancing)diag.textContent='Timer is not advancing';else if(!s.enabled)diag.textContent='Timer running; output disabled (DAC code 0)';else if(s.waveform!=='dc'&&Number(s.amplitude)===0)diag.textContent='Timer running; amplitude is 0%, so this waveform is flat';else diag.textContent=`Timer running · GPIO${state.pins[i]} · latest DAC code ${state.dacCodes[i]}/255`;render(i)});setOnline(true)}catch(e){deviceInfoUnavailable('Device information unavailable; retrying automatically.');setOnline(false)}}
async function loadState(){const requestRevisions=channelRevision.slice();try{const r=await fetch('/api/state',{credentials:'same-origin'});if(!r.ok)throw new Error('HTTP '+r.status);const state=await r.json();document.getElementById('firmwareBuild').textContent=state.firmware?'· '+state.firmware:'· firmware ID missing';const hadPriorTicks=previousTimerTicks!==null,timerAdvancing=hadPriorTicks&&Number(state.timerTicks)!==previousTimerTicks;previousTimerTicks=Number(state.timerTicks);state.channels.forEach((s,i)=>{document.getElementById(`pinLabel${i}`).textContent=`GPIO${state.pins[i]} · DAC`;if(pendingUpdates[i]||requestRevisions[i]!==channelRevision[i])return;channels[i]={...channels[i],...s,pin:state.pins[i]};document.getElementById(`freq${i}`).value=channels[i].frequency;document.getElementById(`amp${i}`).value=channels[i].amplitude;document.getElementById(`offset${i}`).value=channels[i].offset;document.getElementById(`enable${i}`).checked=channels[i].enabled;const diag=document.getElementById(`signalDiag${i}`);if(typeof state.timerRunning!=='boolean'||typeof state.timerTicks!=='number')diag.textContent='Firmware is missing signal diagnostics; upload the latest build';else if(!state.timerRunning)diag.textContent='DAC sample timer did not start';else if(!hadPriorTicks)diag.textContent='Timer started; checking activity…';else if(!timerAdvancing)diag.textContent='Timer is not advancing';else if(!s.enabled)diag.textContent='Timer running; output disabled (DAC code 0)';else if(s.waveform!=='dc'&&Number(s.amplitude)===0)diag.textContent='Timer running; amplitude is 0%, so this waveform is flat';else diag.textContent=`Timer running · GPIO${state.pins[i]} · latest DAC code ${state.dacCodes[i]}/255`;render(i)});setOnline(true)}catch(e){setOnline(false)}}
const adcPins=[32,33,34,35,36,39],pwmPins=[4,13,14,16,17,18,19,21,22,23,27,32,33,34,35,36,39],scopeColors=['#2463eb','#d34f42','#15836b','#8b5cf6','#d58b13','#168ca4','#b23d87','#697a25','#187da1','#cc6c2b','#4c54ae','#208a57','#8753a2','#ac4444','#2c708d','#785c28','#546277'];
let scopePollTimer=null,pwmPollTimer=null,pwmBusy=Array(pwmPins.length).fill(false),pwmRevision=Array(pwmPins.length).fill(0),pwmQueues=pwmPins.map(()=>Promise.resolve()),scopeMode='analog',scopeCache=null,pwmCache=null;
const scopeSelected={analog:new Set([32]),digital:new Set()};
function renderScopePicks(){const pins=scopeMode==='analog'?adcPins:pwmPins,host=document.getElementById('scopeChannelPicks'),timebase=document.getElementById('scopeTimebase');document.getElementById('scopeSelectorLabel').textContent=scopeMode==='analog'?'Select ADC inputs to display':'Select PWM inputs to capture and display';timebase.querySelectorAll('option').forEach(option=>{option.disabled=scopeMode==='analog'&&Number(option.value)>1.28});if(scopeMode==='analog'&&Number(timebase.value)>1.28)timebase.value='1.28';host.innerHTML=pins.map(pin=>`<label class="channel-pick"><input type="checkbox" data-scope-pin="${pin}" ${scopeSelected[scopeMode].has(pin)?'checked':''}> GPIO${pin}</label>`).join('');host.querySelectorAll('input').forEach(input=>input.addEventListener('change',async e=>{const pin=Number(e.target.dataset.scopePin),index=pins.indexOf(pin);if(e.target.checked)scopeSelected[scopeMode].add(pin);else scopeSelected[scopeMode].delete(pin);if(scopeMode==='digital')await setPwmCapture(index,e.target.checked);drawInputScope()}));drawInputScope()}
function drawInputScope(){const canvas=document.getElementById('inputScopePlot'),ctx=canvas.getContext('2d'),w=canvas.width,h=canvas.height,timeWindow=Number(document.getElementById('scopeTimebase').value);ctx.clearRect(0,0,w,h);ctx.font='16px system-ui';ctx.lineWidth=1;ctx.strokeStyle='#e7edf5';for(let j=0;j<=10;j++){const x=j*w/10;ctx.beginPath();ctx.moveTo(x,0);ctx.lineTo(x,h);ctx.stroke();ctx.fillStyle='#718096';ctx.fillText(`${(j*timeWindow/10).toFixed(timeWindow<1?2:1)} s`,x+5,h-8)}const selected=[...scopeSelected[scopeMode]],legend=[];if(scopeMode==='analog'){for(let j=0;j<=4;j++){const y=24+j*(h-52)/4;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();ctx.fillStyle='#718096';ctx.fillText(String(Math.round(4095*(1-j/4))),5,y-5)}selected.forEach(pin=>{const channel=scopeCache?.channels?.find(c=>Number(c.pin)===pin),samples=channel?.samples||[];if(samples.length<2)return;const maxSamples=Math.max(2,Math.round(timeWindow*Number(scopeCache.sampleRateHzPerChannel||200))),visible=samples.slice(-maxSamples),color=scopeColors[adcPins.indexOf(pin)%scopeColors.length],padding=maxSamples-visible.length;ctx.strokeStyle=color;ctx.lineWidth=3;ctx.beginPath();visible.forEach((raw,j)=>{const x=(padding+j)*(w-1)/(maxSamples-1),y=24+(1-Number(raw)/4095)*(h-52);if(j===0)ctx.moveTo(x,y);else ctx.lineTo(x,y)});ctx.stroke();const lo=Math.min(...visible),hi=Math.max(...visible);legend.push(`<span><i style="background:${color}"></i>GPIO${pin} · ${visible[visible.length-1]} counts · range ${lo}–${hi}</span>`)})}else{const count=Math.max(1,selected.length),laneH=(h-54)/count;selected.forEach((pin,index)=>{const channel=pwmCache?.channels?.find(c=>Number(c.pin)===pin),color=scopeColors[pwmPins.indexOf(pin)%scopeColors.length],top=24+index*laneH;ctx.strokeStyle='#e7edf5';ctx.beginPath();ctx.moveTo(0,top+laneH);ctx.lineTo(w,top+laneH);ctx.stroke();ctx.fillStyle=color;ctx.fillText(`GPIO${pin}`,6,top+18);if(!channel?.valid){ctx.fillStyle='#718096';ctx.fillText('Waiting for PWM edges…',90,top+18);return}const freq=Number(channel.frequencyHz),duty=Math.max(0,Math.min(100,Number(channel.dutyPercent)))/100;ctx.strokeStyle=color;ctx.lineWidth=3;ctx.beginPath();for(let x=0;x<w;x++){const phase=((x/(w-1))*timeWindow*freq)%1,high=phase<duty,y=top+(high?laneH*.35:laneH*.8);if(x===0)ctx.moveTo(x,y);else ctx.lineTo(x,y)}ctx.stroke();legend.push(`<span><i style="background:${color}"></i>GPIO${pin} · ${freq.toFixed(2)} Hz · ${Number(channel.dutyPercent).toFixed(1)}% duty · reconstructed</span>`)})}document.getElementById('scopeLegend').innerHTML=legend.join('')||'Select one or more channels to display.';if(!selected.length){ctx.fillStyle='#64748b';ctx.fillText('Select one or more input pins above.',24,42)}}
async function loadScope(){try{const r=await fetch('/api/scope',{credentials:'same-origin'});if(!r.ok)throw new Error('HTTP '+r.status);scopeCache=await r.json();if(scopeMode==='analog'){document.getElementById('scopeStatus').textContent=`ADC1 · ${scopeCache.sampleRateHzPerChannel} frames/s per input · ${scopeCache.sampleCount}/${scopeCache.bufferCapacity} samples`;drawInputScope()}}catch(e){document.getElementById('scopeStatus').textContent='Scope read failed: '+e.message}}
async function loadPwm(){const revisions=pwmRevision.slice();try{const r=await fetch('/api/pwm',{credentials:'same-origin'});if(!r.ok)throw new Error('HTTP '+r.status);pwmCache=await r.json();if(scopeMode==='digital'){let added=false;pwmCache.channels.forEach(channel=>{const pin=Number(channel.pin);if(channel.enabled&&!pwmBusy[pwmPins.indexOf(pin)]&&revisions[pwmPins.indexOf(pin)]===pwmRevision[pwmPins.indexOf(pin)]&&!scopeSelected.digital.has(pin)){scopeSelected.digital.add(pin);added=true}});if(added)renderScopePicks();else drawInputScope()}}catch(e){document.getElementById('scopeStatus').textContent='PWM read failed: '+e.message}}
async function setPwmCapture(i,enabled){
  const revision=++pwmRevision[i];
  pwmBusy[i]=true;
  const request=pwmQueues[i].catch(()=>{}).then(async()=>{
    const body=new URLSearchParams({channel:String(i),enabled:enabled?'1':'0'});
    const r=await fetch('/api/pwm/config',{credentials:'same-origin',method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body.toString()});
    if(!r.ok)throw new Error('HTTP '+r.status);
  });
  pwmQueues[i]=request.catch(()=>{});
  try{
    await request;
    if(revision===pwmRevision[i])document.getElementById('scopeStatus').textContent=enabled?`PWM capture enabled on GPIO${pwmPins[i]}`:`PWM capture disabled on GPIO${pwmPins[i]}`;
  }catch(e){
    if(revision===pwmRevision[i]){
      if(enabled)scopeSelected.digital.delete(pwmPins[i]);
      else scopeSelected.digital.add(pwmPins[i]);
      renderScopePicks();
      document.getElementById('scopeStatus').textContent='Capture failed: '+e.message;
    }
  }finally{if(revision===pwmRevision[i])pwmBusy[i]=false}
}
function showDeviceView(scope){clearInterval(scopePollTimer);clearInterval(pwmPollTimer);scopePollTimer=pwmPollTimer=null;document.getElementById('generatorView').hidden=scope;document.getElementById('scopeView').hidden=!scope;document.getElementById('generatorTab').classList.toggle('active',!scope);document.getElementById('scopeTab').classList.toggle('active',scope);if(scope){renderScopePicks();loadScope();loadPwm();scopePollTimer=setInterval(loadScope,250);pwmPollTimer=setInterval(loadPwm,500)}}
document.getElementById('analogMode').addEventListener('click',()=>{scopeMode='analog';document.getElementById('analogMode').classList.add('active');document.getElementById('digitalMode').classList.remove('active');renderScopePicks();loadScope()});document.getElementById('digitalMode').addEventListener('click',()=>{scopeMode='digital';document.getElementById('digitalMode').classList.add('active');document.getElementById('analogMode').classList.remove('active');renderScopePicks();loadPwm()});
document.getElementById('scopeTimebase').addEventListener('change',drawInputScope);
document.getElementById('generatorTab').addEventListener('click',()=>showDeviceView(false));document.getElementById('scopeTab').addEventListener('click',()=>showDeviceView(true));
document.getElementById('dcPreset').addEventListener('click',()=>{channels[0]={...channels[0],waveform:'dc',amplitude:0,offset:50};channels[1]={...channels[1],waveform:'dc',amplitude:0,offset:25};channels.forEach((c,i)=>{document.getElementById(`amp${i}`).value=c.amplitude;document.getElementById(`offset${i}`).value=c.offset;document.getElementById(`enable${i}`).checked=c.enabled;changed(i,true,['waveform','amplitude','offset'])})});
let lastPreviewFrame=null;
function animatePreview(timestamp){
  if(lastPreviewFrame===null)lastPreviewFrame=timestamp;
  const elapsed=(timestamp-lastPreviewFrame)/1000;
  if(elapsed>=1/30){
    channels.forEach((c,i)=>{
      if(c.enabled&&c.waveform!=='dc')previewPhase[i]=(previewPhase[i]+elapsed*c.frequency)%1;
      if(!document.getElementById('generatorView').hidden)draw(i);
    });
    lastPreviewFrame=timestamp;
  }
  requestAnimationFrame(animatePreview);
}
requestAnimationFrame(animatePreview);
loadState();loadEvents();setInterval(loadState,5000);setInterval(loadEvents,2000);
</script>
</body>
</html>
)WAVEGENHTML";


