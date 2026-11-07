#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"WAVEGENHTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <meta name="theme-color" content="#f4f7fb">
  <title>ESP32-S2 Wave Generator</title>
  <style>
    :root{font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;color:#162238;background:#f4f7fb;font-synthesis:none}
    *{box-sizing:border-box}body{margin:0;min-width:320px}.wrap{max-width:1050px;margin:auto;padding:28px 18px 48px}
    header{display:flex;justify-content:space-between;align-items:center;gap:20px;margin:0 0 24px}
    h1{font-size:clamp(1.5rem,4vw,2.2rem);margin:0;letter-spacing:-.04em}.sub{color:#607089;margin:7px 0 0;font-size:.95rem}
    .badge{display:flex;align-items:center;gap:8px;padding:9px 12px;border:1px solid #dce4ef;border-radius:999px;background:#fff;color:#53637b;font-size:.82rem;white-space:nowrap}
    .dot{width:9px;height:9px;border-radius:50%;background:#d39135}.dot.online{background:#27a776;box-shadow:0 0 0 4px #e4f6ef}
    .notice{padding:12px 15px;background:#fff8e8;border:1px solid #f4dfb5;border-radius:12px;color:#755721;font-size:.9rem;line-height:1.5;margin-bottom:18px}
    .grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:18px}.card{background:white;border:1px solid #e0e7f0;border-radius:18px;padding:18px;box-shadow:0 8px 26px #1a315008}
    .cardhead{display:flex;justify-content:space-between;align-items:center;margin-bottom:12px}.cardhead h2{margin:0;font-size:1.13rem}.pin{font-size:.8rem;color:#61718a;background:#f0f4f9;border-radius:7px;padding:5px 8px}
    canvas{display:block;width:100%;height:150px;border:1px solid #e5ebf3;border-radius:12px;background:linear-gradient(#fff,#fbfdff)}
    .controls{display:grid;grid-template-columns:1fr 1fr;gap:14px 16px;margin-top:16px}.field label{display:flex;justify-content:space-between;align-items:center;gap:8px;font-size:.84rem;font-weight:600;color:#45556d;margin-bottom:7px}.value{font-variant-numeric:tabular-nums;color:#17253c;font-weight:700}
    select{width:100%;height:40px;border:1px solid #cfd8e5;border-radius:9px;padding:0 10px;background:#fff;color:#18263c;font:inherit}
    input[type=range]{width:100%;accent-color:#426de0;margin:7px 0 0}.rangeends{display:flex;justify-content:space-between;color:#8a98aa;font-size:.72rem;margin-top:2px}
    .toggle{display:flex;align-items:center;gap:9px;font-size:.84rem;font-weight:600;color:#45556d}.toggle input{width:18px;height:18px;accent-color:#258e68}.presetbar{display:flex;align-items:center;gap:12px;flex-wrap:wrap;margin:0 0 18px}.presetbar button{border:0;border-radius:9px;padding:10px 14px;background:#263b61;color:white;font:600 .88rem system-ui;cursor:pointer}.presetbar button:hover{background:#345181}.presetbar span{font-size:.82rem;color:#687890}input:disabled{opacity:.45;cursor:not-allowed}.wave-buttons{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:7px}.wavebtn{border:1px solid #cfd8e5;border-radius:8px;padding:9px 6px;background:#fff;color:#43536b;font:600 .78rem system-ui;cursor:pointer}.wavebtn.active{background:#426de0;border-color:#426de0;color:#fff}.field.wide{grid-column:1/-1}
    .foot{margin-top:18px;color:#687890;font-size:.82rem;line-height:1.55}.foot strong{color:#465770}
    @media(max-width:720px){header{align-items:flex-start;flex-direction:column}.grid{grid-template-columns:1fr}.controls{gap:12px}}
  </style>
</head>
<body>
  <main class="wrap">
    <header>
      <div><h1>ESP32-S2 Wave Generator</h1><p class="sub">Two independently controlled DAC outputs, including a synthetic PPG pulse.</p></div>
      <div class="badge"><span class="dot" id="connectionDot"></span><span id="connectionText">Connecting…</span></div>
    </header>
    <div class="notice"><strong>ESP32-S2 outputs:</strong> GPIO17 (DAC1) and GPIO18 (DAC2). Output is unipolar and nominally 0–3.3 V. The built-in DAC is 8-bit and is for low-frequency signal testing, not a calibrated instrument output.</div>
    <div class="presetbar"><button id="dcPreset" type="button">Load DC midpoint test</button><span>GPIO17 = 128/255; GPIO18 = 64/255</span></div>
    <section class="grid" id="channels"></section>
    <p class="foot"><strong>Wiring:</strong> connect GPIO17 and/or GPIO18 to a high-impedance ADC input on the receiving ESP32, and connect both boards’ GND pins together. Do not connect the output to equipment that expects negative voltage, voltage above 3.3 V, or a low-impedance load. Hold the ESP32-S2 BOOT button to pause waveform updates. USB Serial commands: `ch1` or `ch2` selects a channel, then `a0`–`a5` selects its waveform.</p>
  </main>
<script>
const channelDefaults=[
  {name:'Channel 1',pin:17,waveform:'ppg',frequency:1.2,amplitude:90,offset:50,enabled:true},
  {name:'Channel 2',pin:18,waveform:'ppg',frequency:1.2,amplitude:45,offset:50,enabled:true}
];
let channels=channelDefaults.map(c=>({...c}));
const shapes=['sine','square','triangle','sawtooth','ppg','dc'];
const shapeLabel={sine:'Sine',square:'Square',triangle:'Triangle',sawtooth:'Sawtooth',ppg:'PPG pulse',dc:'DC level'};
const commandWave={a0:'sine',a1:'square',a2:'triangle',a3:'sawtooth',a4:'ppg',a5:'dc'};
const card=(c,i)=>`<article class="card" data-channel="${i}">
  <div class="cardhead"><h2>${c.name}</h2><span class="pin">GPIO${c.pin} · DAC</span></div>
  <canvas id="plot${i}" width="700" height="220" aria-label="${c.name} waveform preview"></canvas>
  <div class="controls">
    <div class="field wide"><label>Choose waveform · command IDs</label><div class="wave-buttons">${shapes.map((s,j)=>`<button type="button" class="wavebtn" data-command="a${j}">a${j} · ${shapeLabel[s]}</button>`).join('')}</div></div>
    <div class="field"><label for="freq${i}">Frequency <span class="value"><span id="freqv${i}">${c.frequency.toFixed(1)}</span> Hz <span id="bpm${i}"></span></span></label><input id="freq${i}" type="range" min="0.5" max="10" step="0.1" value="${c.frequency}"><div class="rangeends"><span>0.5 Hz</span><span>10 Hz</span></div></div>
    <div class="field"><label for="amp${i}">Amplitude <span class="value"><span id="ampv${i}">${c.amplitude}</span>% Vpp</span></label><input id="amp${i}" type="range" min="0" max="100" step="1" value="${c.amplitude}"><div class="rangeends"><span>0% Vpp</span><span>100% Vpp</span></div></div>
    <div class="field"><label for="offset${i}">Offset <span class="value"><span id="offsetv${i}">${c.offset}</span>%</span></label><input id="offset${i}" type="range" min="0" max="100" step="1" value="${c.offset}"><div class="rangeends"><span>0 V</span><span>~3.3 V</span></div></div>
    <label class="toggle"><input id="enable${i}" type="checkbox" ${c.enabled?'checked':''}> Output enabled</label>
  </div>
</article>`;
document.getElementById('channels').innerHTML=channels.map(card).join('');
function gaussian(x,c,w){const z=(x-c)/w;return Math.exp(-.5*z*z)}
function normWave(shape,p){if(shape==='dc')return .5;if(shape==='square')return p<.5?1:0;if(shape==='triangle')return 1-Math.abs(2*p-1);if(shape==='sawtooth')return p;if(shape==='ppg'){const pulse=Math.max(0,gaussian(p,.18,.065)+.18*gaussian(p,.43,.025)-.07*gaussian(p,.48,.03));return .05+.95*Math.min(1,pulse)}return .5+.5*Math.sin(2*Math.PI*p)}
function draw(i){const c=channels[i],canvas=document.getElementById(`plot${i}`),ctx=canvas.getContext('2d'),w=canvas.width,h=canvas.height;ctx.clearRect(0,0,w,h);ctx.strokeStyle='#e7edf5';ctx.lineWidth=1;for(let y=30;y<h;y+=40){ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke()}ctx.strokeStyle=c.enabled?'#426de0':'#aab5c4';ctx.lineWidth=3;ctx.beginPath();for(let x=0;x<w;x++){const p=(x/w*2)%1,v=.5+(normWave(c.waveform,p)-.5)*(c.amplitude/100);const y=(1-v)*h*.72+h*.14;if(x===0)ctx.moveTo(x,y);else ctx.lineTo(x,y)}ctx.stroke();ctx.fillStyle='#64748b';ctx.font='14px system-ui';const label=c.waveform==='dc'?'DC level':c.waveform==='ppg'?`${c.frequency.toFixed(1)} Hz · ${Math.round(c.frequency*60)} bpm · synthetic PPG`:`${c.frequency.toFixed(1)} Hz · ${shapeLabel[c.waveform]}`;ctx.fillText(c.enabled?label:'Output off',12,20)}
function render(i){const c=channels[i],dc=c.waveform==='dc';document.getElementById(`freq${i}`).disabled=dc;document.getElementById(`amp${i}`).disabled=dc;document.getElementById(`freqv${i}`).textContent=dc?'—':c.frequency.toFixed(1);document.getElementById(`ampv${i}`).textContent=dc?'—':Math.round(c.amplitude);document.getElementById(`offsetv${i}`).textContent=Math.round(c.offset);document.getElementById(`bpm${i}`).textContent=c.waveform==='ppg'?`(${Math.round(c.frequency*60)} bpm)`:'';document.querySelectorAll(`[data-channel="${i}"] .wavebtn`).forEach(b=>b.classList.toggle('active',commandWave[b.dataset.command]===c.waveform));draw(i)}
let timers=[];
async function send(i,command=null){const c=channels[i],params=new URLSearchParams({channel:String(i),frequency:c.frequency.toFixed(1),amplitude:String(Math.round(c.amplitude)),offset:String(Math.round(c.offset)),enabled:c.enabled?'1':'0'});params.set(command?'command':'waveform',command||c.waveform);try{const r=await fetch('/api/channel?'+params.toString(),{method:'POST'});if(!r.ok)throw new Error('HTTP '+r.status);setOnline(true)}catch(e){setOnline(false)}}
function changed(i,immediate=false,command=null){render(i);clearTimeout(timers[i]);if(immediate)send(i,command);else timers[i]=setTimeout(()=>send(i,command),140)}
channels.forEach((c,i)=>{document.querySelectorAll(`[data-channel="${i}"] .wavebtn`).forEach(button=>button.addEventListener('click',()=>{c.waveform=commandWave[button.dataset.command];changed(i,true,button.dataset.command)}));document.getElementById(`freq${i}`).addEventListener('input',e=>{c.frequency=Number(e.target.value);changed(i)});document.getElementById(`amp${i}`).addEventListener('input',e=>{c.amplitude=Number(e.target.value);changed(i)});document.getElementById(`offset${i}`).addEventListener('input',e=>{c.offset=Number(e.target.value);changed(i)});document.getElementById(`enable${i}`).addEventListener('change',e=>{c.enabled=e.target.checked;changed(i,true)});render(i)});
function setOnline(online){document.getElementById('connectionDot').classList.toggle('online',online);document.getElementById('connectionText').textContent=online?'ESP32 connected':'Connection lost'}
async function loadState(){try{const r=await fetch('/api/state');if(!r.ok)throw new Error();const state=await r.json();state.channels.forEach((s,i)=>{channels[i]={...channels[i],...s};document.getElementById(`freq${i}`).value=channels[i].frequency;document.getElementById(`amp${i}`).value=channels[i].amplitude;document.getElementById(`offset${i}`).value=channels[i].offset;document.getElementById(`enable${i}`).checked=channels[i].enabled;render(i)});setOnline(true)}catch(e){setOnline(false)}}
document.getElementById('dcPreset').addEventListener('click',()=>{channels[0]={...channels[0],waveform:'dc',amplitude:0,offset:50,enabled:true};channels[1]={...channels[1],waveform:'dc',amplitude:0,offset:25,enabled:true};channels.forEach((c,i)=>{document.getElementById(`amp${i}`).value=c.amplitude;document.getElementById(`offset${i}`).value=c.offset;document.getElementById(`enable${i}`).checked=c.enabled;changed(i,true,'a5')})});
loadState();setInterval(loadState,5000);
</script>
</body>
</html>
)WAVEGENHTML";
