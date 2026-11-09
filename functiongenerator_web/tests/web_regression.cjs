const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const path=require('node:path');
function page(fetch=()=>new Promise(()=>{})){
  const elements=new Map(),buttons=[[],[]],points=[];
  const ctx=new Proxy({lineTo(x,y){points.push(y)}},{get:(o,k)=>o[k]||(()=>{})});
  function element(id){
    if(!elements.has(id))elements.set(id,{value:'1.28',hidden:false,dataset:{},classList:{toggle(){},add(){},remove(){}},listeners:{},addEventListener(type,fn){this.listeners[type]=fn},querySelectorAll(){return []},width:700,height:220,getContext:()=>ctx});
    return elements.get(id);
  }
  for(let i=0;i<2;i++)for(let j=0;j<6;j++){const b=element(`button${i}${j}`);b.dataset.command='a'+j;buttons[i].push(b)}
  const document={getElementById:element,querySelectorAll(selector){const match=selector.match(/data-channel="(\d)"/);return match?buttons[+match[1]]:[]}};
  const context=vm.createContext({document,URLSearchParams,fetch,setTimeout:()=>1,clearTimeout(){},setInterval(){},clearInterval(){},requestAnimationFrame(){}});
  const source=fs.readFileSync(path.join(__dirname,'../include/wavegen_page.h'),'utf8').match(/<script>([\s\S]*?)<\/script>/)[1];
  vm.runInContext(source.replace('loadState();loadEvents();setInterval(loadState,5000);setInterval(loadEvents,2000);',''),context);
  return {run:s=>vm.runInContext(s,context),element,buttons,points};
}
test('controls edit current channel objects after polling or presets replace them',()=>{
  const p=page();p.run('channels=channels.map(c=>({...c}))');
  for(let i=0;i<2;i++){
    p.element('enable'+i).listeners.change({target:{checked:true}});
    assert.equal(p.run(`channels[${i}].enabled`),true);
    for(const [id,field,value] of [['freq','frequency',2.5],['amp','amplitude',65],['offset','offset',40]]){
      p.element(id+i).listeners.input({target:{value:String(value)}});
      assert.equal(p.run(`channels[${i}].${field}`),value);
    }
    p.buttons[i][0].listeners.click();assert.equal(p.run(`channels[${i}].waveform`),'sine');
  }
});
test('preview cycle spacing depends on frequency and enabled phase advances with time',()=>{
  const p=page();p.run("channels[0]={...channels[0],enabled:true,waveform:'sine',frequency:1}");
  function crossings(ys){return ys.slice(1).filter((y,i)=>(y-110)*(ys[i]-110)<0).length}
  p.points.length=0;p.run('draw(0)');const low=crossings(p.points);
  p.points.length=0;p.run('channels[0].frequency=3;draw(0)');assert(crossings(p.points)>low);
  p.run('animatePreview(0);animatePreview(100)');assert(Math.abs(p.run('previewPhase[0]')-.3)<1e-8);assert.equal(p.run('previewPhase[1]'),0);
  p.run("channels[0].waveform='dc';animatePreview(200)");assert(Math.abs(p.run('previewPhase[0]')-.3)<1e-8);
});
test('stale PWM poll cannot reselect a pin after disabling capture',async()=>{
  let resolvePoll;
  const p=page(url=>url==='/api/pwm'?new Promise(resolve=>{resolvePoll=resolve}):Promise.resolve({ok:true}));
  p.run("scopeMode='digital';scopeSelected.digital.add(4)");
  const poll=p.run('loadPwm()');p.run('scopeSelected.digital.delete(4)');await p.run('setPwmCapture(0,false)');
  resolvePoll({ok:true,json:async()=>({channels:[{pin:4,enabled:true}]})});await poll;
  assert.equal(p.run('scopeSelected.digital.has(4)'),false);
});
test('rapid PWM changes are serialized and failed disable restores selection',async()=>{
  const pending=[],p=page((url,options)=>new Promise(resolve=>pending.push({resolve,body:options.body})));
  const enable=p.run('setPwmCapture(0,true)'),disable=p.run('setPwmCapture(0,false)');
  await new Promise(setImmediate);assert.equal(pending.length,1);assert.equal(new URLSearchParams(pending[0].body).get('enabled'),'1');
  pending[0].resolve({ok:true});await enable;await new Promise(setImmediate);
  assert.equal(pending.length,2);assert.equal(new URLSearchParams(pending[1].body).get('enabled'),'0');
  pending[1].resolve({ok:false,status:500});await disable;
  assert.equal(p.run('scopeSelected.digital.has(4)'),true);assert.equal(p.run('pwmBusy[0]'),false);
});
