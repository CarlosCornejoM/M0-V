// ------------------ index_html.h ------------------
#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>ESP8266 WS Control</title>
  <style>
    *{margin:0;padding:0;box-sizing:border-box}
    body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#0a0a0a;color:#fff;overflow-x:hidden}
    .container{max-width:1400px;margin:0 auto;padding:1rem}
    .header{text-align:center;margin-bottom:1rem}
    .header h1{font-size:1.5rem;font-weight:600;color:#00ff41;margin-bottom:.5rem}
    .header .connection{font-size:.9rem;color:#ccc;margin-bottom:1rem}

    .grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:1rem;min-height:80vh}
    .card{background:linear-gradient(135deg,#1a1a2e,#16213e);border:1px solid #333;border-radius:12px;padding:1rem;backdrop-filter:blur(10px)}

    .gamepad{display:grid;grid-template-rows:auto 1fr auto auto auto;gap:1rem;text-align:center}
    .gamepad select{background:#2a2a3e;color:#fff;border:1px solid #444;border-radius:6px;padding:.5rem;width:100%}
    .controller{position:relative;width:100%;max-width:500px;margin:0 auto;padding:2rem}
    .controller-body{width:100%;height:350px;background:linear-gradient(135deg,#2a2a3e,#1a1a2e);border:2px solid #444;border-radius:60px 60px 40px 40px;position:relative;box-shadow:0 8px 32px rgba(0,0,0,.5)}
    .controller-body::before{content:'';position:absolute;top:20px;left:50%;transform:translateX(-50%);width:80px;height:8px;background:#333;border-radius:4px}
    .btn{position:absolute;width:32px;height:32px;border-radius:50%;border:2px solid #555;background:linear-gradient(135deg,#3a3a4e,#2a2a3e);transition:all .1s;cursor:pointer;display:flex;align-items:center;justify-content:center;font-size:11px;font-weight:bold;color:#ccc}
    .btn.active{background:linear-gradient(135deg,#00ff41,#00cc33);border-color:#00ff41;box-shadow:0 0 15px rgba(0,255,65,.6);color:#000}

    .btn.face{right:50px}
    .btn.face.triangle{top:50px}
    .btn.face.square{top:90px;right:80px}
    .btn.face.circle{top:90px;right:20px}
    .btn.face.cross{top:130px;right:50px}

    .btn.dpad{left:50px}
    .btn.dpad.up{top:50px}
    .btn.dpad.left{top:90px;left:20px}
    .btn.dpad.right{top:90px;left:80px}
    .btn.dpad.down{top:130px;left:50px}

    .btn.shoulder{top:5px;width:50px;height:20px;border-radius:8px;font-size:10px}
    .btn.shoulder.l1{left:60px}
    .btn.shoulder.r1{right:60px}

    .btn.center{top:110px;left:50%;transform:translateX(-50%);width:28px;height:28px;font-size:9px}
    .btn.center.share{left:40%}
    .btn.center.options{left:60%}

    .stick{position:absolute;width:70px;height:70px;border:3px solid #555;border-radius:50%;background:radial-gradient(circle,#444,#2a2a2e)}
    .stick.left{bottom:60px;left:80px}
    .stick.right{bottom:60px;right:80px}
    .stick-dot{position:absolute;width:14px;height:14px;border-radius:50%;background:#00ff41;top:50%;left:50%;transform:translate(-50%,-50%);transition:all .1s;box-shadow:0 0 8px rgba(0,255,65,.5)}

    .triggers{position:absolute;top:-15px;left:50%;transform:translateX(-50%);width:320px;height:40px;display:flex;justify-content:space-between;align-items:center}
    .trigger-container{display:flex;flex-direction:column;align-items:center}
    .trigger-label{font-size:10px;color:#888;margin-bottom:2px}
    .trigger-bar{width:60px;height:8px;background:#333;border-radius:4px;position:relative;overflow:hidden;border:1px solid #555}
    .trigger-fill{height:100%;background:linear-gradient(90deg,#00ff41,#00cc33);border-radius:3px;transition:width .1s}

    .controls{display:grid;gap:1rem}
    .section{background:rgba(255,255,255,.05);border-radius:8px;padding:1rem}
    .section h3{font-size:1rem;margin-bottom:.5rem;color:#00ff41}
    .slider-group{display:flex;align-items:center;gap:.5rem;margin:.75rem 0}
    .slider-group span{min-width:35px;font-size:.9rem}
    .slider{flex:1;height:8px;border-radius:4px;background:#333;appearance:none;outline:none;margin:0 .5rem}
    .slider::-webkit-slider-thumb{appearance:none;width:20px;height:20px;border-radius:50%;background:#00ff41;cursor:pointer;box-shadow:0 0 5px rgba(0,255,65,.5)}
    .slider::-moz-range-thumb{width:20px;height:20px;border-radius:50%;background:#00ff41;cursor:pointer;border:none;box-shadow:0 0 5px rgba(0,255,65,.5)}
    .btn-action{background:linear-gradient(135deg,#00ff41,#00cc33);color:#000;border:none;border-radius:6px;padding:.5rem 1rem;cursor:pointer;font-weight:600;transition:all .2s;margin:.2rem}
    .btn-action:hover{transform:translateY(-2px);box-shadow:0 4px 15px rgba(0,255,65,.3)}

    .serial-section{background:rgba(255,255,255,.05);border-radius:8px;padding:1rem;display:flex;flex-direction:column}
    .serial-section h3{margin-bottom:.5rem;color:#00ff41;display:flex;justify-content:space-between;align-items:center}
    .serial-section h3 button{background:#00ff41;border:none;padding:.2rem .5rem;border-radius:4px;cursor:pointer;color:#000;font-weight:600;}
    .status{flex:1;font-family:monospace;background:#000;border-radius:6px;padding:1rem;font-size:.8rem;max-height:200px;overflow-y:auto;white-space:pre-wrap;margin-bottom:1rem}

    .perf-toggle{background:linear-gradient(135deg,#00ff41,#00cc33);color:#000;border:none;border-radius:6px;padding:.5rem 1rem;cursor:pointer;font-weight:600;transition:all .2s;width:100%}
    .perf-toggle:hover{transform:translateY(-2px);box-shadow:0 4px 15px rgba(0,255,65,.3)}

    .perf-section{display:none;background:rgba(255,255,255,.05);border-radius:8px;padding:1rem;grid-column:1/-1}
    .perf-section h3{margin-bottom:.5rem;color:#00ff41;display:flex;justify-content:space-between;align-items:center}
    .perf-section h3 button{background:#00ff41;border:none;padding:.2rem .5rem;border-radius:4px;cursor:pointer;color:#000;font-weight:600;}
    .latency-graph{width:100%;height:300px;background:#111;border-radius:6px;margin:.5rem 0}
    .latency-info{display:flex;justify-content:space-between;align-items:center;font-size:.85rem}
    .latency-current{color:#00ff41;font-weight:600}

    .mapping-section{display:none;margin-top:1rem}
    .mapping-table{width:100%;border-collapse:collapse;margin:.5rem 0;font-size:.8rem}
    .mapping-table th,.mapping-table td{padding:.4rem;border:1px solid #444;text-align:left}
    .mapping-table th{background:#333;color:#00ff41}
    .mapping-table select{background:#2a2a3e;color:#fff;border:1px solid #555;border-radius:4px;padding:.2rem;width:100%}

    @media (max-width:1024px){
      .grid{grid-template-columns:1fr;gap:.5rem}
      .controller{max-width:350px}
      .controller-body{height:300px}
      .btn{width:28px;height:28px;font-size:9px}
      .stick{width:60px;height:60px}
    }
    @media (max-width:640px){
      .container{padding:.5rem}
      .controller{max-width:300px}
      .controller-body{height:280px}
      .btn{width:24px;height:24px;font-size:8px}
      .stick{width:50px;height:50px}
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>ESP8266 Control Interface</h1>
      <div class="connection">Connection Status: Connected to ESP8266</div>
    </div>
    <div class="grid">
      <div class="card gamepad">
        <select id="gpSelect"><option value="">(ninguno)</option></select>
        <div class="controller"><div class="controller-body">
            <div class="btn face triangle" id="btn3">△</div>
            <div class="btn face square" id="btn0">□</div>
            <div class="btn face circle" id="btn1">○</div>
            <div class="btn face cross" id="btn2">✕</div>
            <div class="btn dpad up" id="btn12">↑</div>
            <div class="btn dpad left" id="btn14">←</div>
            <div class="btn dpad right" id="btn15">→</div>
            <div class="btn dpad down" id="btn13">↓</div>
            <div class="btn shoulder l1" id="btn4">L1</div>
            <div class="btn shoulder r1" id="btn5">R1</div>
            <div class="btn center share" id="btn8">⚏</div>
            <div class="btn center options" id="btn9">☰</div>
            <div class="stick left" id="stickL"><div class="stick-dot" id="dotL"></div></div>
            <div class="stick right" id="stickR"><div class="stick-dot" id="dotR"></div></div>
            <div class="triggers">
              <div class="trigger-container"><div class="trigger-label">L2</div><div class="trigger-bar"><div class="trigger-fill" id="trigL"></div></div></div>
              <div class="trigger-container"><div class="trigger-label">R2</div><div class="trigger-bar"><div class="trigger-fill" id="trigR"></div></div></div>
            </div>
        </div></div>
      </div>

      <div class="card controls">
        <div class="section">
          <h3>Audio & LEDs</h3>
          <button class="btn-action" onclick="sendCmd('playMelody')">🎵 Play Melody</button>
          <button class="btn-action" onclick="sendCmd('espOn')">ESP ON</button>
          <button class="btn-action" onclick="sendCmd('espOff')">ESP OFF</button>
          <button class="btn-action" onclick="sendCmd('unoOn')">UNO ON</button>
          <button class="btn-action" onclick="sendCmd('unoOff')">UNO OFF</button>
        </div>
        <div class="section">
          <h3>Servos</h3>
          <div class="slider-group"><span>S1:</span><input class="slider" id="s1" type="range" min="0" max="180" value="90" oninput="updateServo()"><span id="s1v">90°</span></div>
          <div class="slider-group"><span>S2:</span><input class="slider" id="s2" type="range" min="0" max="180" value="90" oninput="updateServo()"><span id="s2v">90°</span></div>
          <button class="btn-action" onclick="sendServos()">Send Servos</button>
        </div>
        <div class="section">
          <h3>Motors</h3>
          <div class="slider-group"><span>Vel:</span><input class="slider" id="mv" type="range" min="-255" max="255" value="0" oninput="updateMotor()"><span id="mvv">0</span></div>
          <div class="slider-group"><span>Time:</span><input class="slider" id="mt" type="range" min="100" max="5000" step="100" value="1000" oninput="updateMotor()"><span id="mtv">1000ms</span></div>
          <select id="ms"><option value="1">Motor 1</option><option value="2">Motor 2</option><option value="3">Both</option></select>
          <button class="btn-action" onclick="sendMotor()">Send Motor</button>
        </div>
      </div>

      <div class="card serial-section">
        <h3>
          Serial Monitor
          <button onclick="document.getElementById('ser').textContent='-- Inicio de registro --\n';">Clear</button>
        </h3>
        <div class="status" id="ser">-- Inicio de registro --\n</div>
        <div>
          <button class="perf-toggle" id="btnTogglePerf">Mostrar Performance</button>
          <button class="perf-toggle" id="btnToggleMapping">Mostrar Mapping</button>
        </div>
      </div>

      <div class="card perf-section" id="perfSection">
        <h3>
          Performance Monitor
          <button onclick="clearPerf()">Clear</button>
        </h3>
        <canvas id="latency" class="latency-graph" width="300" height="300"></canvas>
        <div class="latency-info">
          <span>Avg: <span id="avg">0</span>ms | Std: <span id="std">0</span>ms</span>
          <span class="latency-current">Current: <span id="current">0</span>ms</span>
        </div>
      </div>

      <div class="card">
        <div class="mapping-section" id="mappingSection">
          <h4>Button Mapping</h4>
          <table class="mapping-table" id="mappingTable">
            <thead><tr><th>Button</th><th>Action</th><th>Type</th></tr></thead>
            <tbody></tbody>
          </table>
          <button class="btn-action" onclick="saveMapping()">Save Mapping</button>
        </div>
      </div>

    </div>
  </div>

<script>
const ws = new WebSocket(`ws://${location.hostname}:81/`);
let lastSent=0, latencies=[], points=[];
let userIsAdjustingSliders = false;
let lastUserSliderUpdate = 0;

document.getElementById('btnTogglePerf').onclick = ()=>{
  const perf = document.getElementById('perfSection');
  const btn = document.getElementById('btnTogglePerf');
  if(perf.style.display === 'none' || perf.style.display === '') {
    perf.style.display = 'block';
    btn.textContent = 'Ocultar Performance';
  } else {
    perf.style.display = 'none';
    btn.textContent = 'Mostrar Performance';
  }
};

document.getElementById('btnToggleMapping').onclick = ()=>{
  const mapping = document.getElementById('mappingSection');
  const btn = document.getElementById('btnToggleMapping');
  if(mapping.style.display === 'none' || mapping.style.display === '') {
    mapping.style.display = 'block';
    btn.textContent = 'Ocultar Mapping';
  } else {
    mapping.style.display = 'none';
    btn.textContent = 'Mostrar Mapping';
  }
};

function clearPerf(){
  latencies=[]; points=[];
  document.getElementById('avg').textContent='0';
  document.getElementById('std').textContent='0';
  document.getElementById('current').textContent='0';
  const ctx = document.getElementById('latency').getContext('2d');
  ctx.clearRect(0,0,ctx.canvas.width,ctx.canvas.height);
}

function appendSerial(line){
  const ser = document.getElementById('ser');
  ser.textContent += line + '\n';
  ser.scrollTop = ser.scrollHeight;
}

let buttonMappings = Array(16).fill().map(()=>({action:'',type:'onPress'}));
buttonMappings[0] = {action:'playMelody',type:'onPress'};
buttonMappings[2] = {action:'espOn',type:'onPress'};
buttonMappings[1] = {action:'espOff',type:'onPress'};

let prevButtonState = Array(16).fill(false);
let state={lx:0,ly:0,rx:0,ry:0,tl:0,tr:0,btn:Array(16).fill(false)};

function createMappingTable(){
  const tbody = document.querySelector('#mappingTable tbody');
  tbody.innerHTML='';
  buttonMappings.forEach((bm,i)=>{
    const row=tbody.insertRow();
    const cBtn=row.insertCell(), cA=row.insertCell(), cT=row.insertCell();
    cBtn.textContent = `${i}: ${['□','○','✕','△','L1','R1','L2','R2','⚏','☰','L3','R3','↑','↓','←','→'][i]}`;
    const selA=document.createElement('select');
    ['', 'playMelody','espOn','espOff','unoOn','unoOff'].forEach(v=>{
      const o=document.createElement('option');o.value=v;o.textContent=v||'(none)'; selA.appendChild(o);
    });
    selA.value=bm.action; selA.onchange=()=>bm.action=selA.value; cA.appendChild(selA);
    const selT=document.createElement('select');
    ['onPress','onHold'].forEach(v=>{
      const o=document.createElement('option');o.value=v;o.textContent=v; selT.appendChild(o);
    });
    selT.value=bm.type; selT.onchange=()=>bm.type=selT.value; cT.appendChild(selT);
  });
}

function saveMapping(){ console.log('Mappings saved', buttonMappings); }

function updateGamepadList(){
  const sel = document.getElementById('gpSelect');
  sel.innerHTML = '<option value="">(ninguno)</option>';
  navigator.getGamepads().forEach((g,i)=>{
    if(g) sel.insertAdjacentHTML('beforeend',`<option value="${i}">${i}: ${g.id.substring(0,25)}</option>`);
  });
}

function pollGamepad(){
  const idx = parseInt(document.getElementById('gpSelect').value);
  if(isNaN(idx)) return;
  const g = navigator.getGamepads()[idx];
  if(!g) return;
  g.buttons.forEach((b,i)=>{
    if(i<16){
      const pressed = b.pressed;
      if(pressed !== prevButtonState[i]){
        prevButtonState[i] = pressed;
        let eff = i===0?2 : i===2?0 : i;
        const bm = buttonMappings[eff];
        const btnEl = document.getElementById(`btn${eff}`);
        if(pressed){
          btnEl.classList.add('active');
          if(bm.action && bm.type==='onPress') sendCmd(bm.action);
        } else {
          btnEl.classList.remove('active');
        }
      }
      state.btn[i] = pressed;
    }
  });
  state.lx = g.axes[0]||0; state.ly = g.axes[1]||0;
  state.rx = g.axes[2]||0; state.ry = g.axes[3]||0;
  state.tl = g.buttons[6]?.value||0; state.tr = g.buttons[7]?.value||0;
  updateVisuals(); 
  
  ws.send(JSON.stringify({cmd:'gamepad',...state}));
}

function updateVisuals(){
  const r = 125;
  document.getElementById('dotL').style.transform =
    `translate(${-50+state.lx*r}%,${-50+state.ly*r}%)`;
  document.getElementById('dotR').style.transform =
    `translate(${-50+state.rx*r}%,${-50+state.ry*r}%)`;
  document.getElementById('trigL').style.width = `${state.tl*100}%`;
  document.getElementById('trigR').style.width = `${state.tr*100}%`;
}

function drawLatency(){
  const c=document.getElementById('latency'),ctx=c.getContext('2d');
  ctx.clearRect(0,0,c.width,c.height);
  if(points.length<2) return;
  const p=30,w=c.width-2*p,h=c.height-2*p;
  const maxY=Math.max(...points.map(o=>o.y),50), maxX=points.length-1;
  ctx.strokeStyle='#555';ctx.lineWidth=1;ctx.beginPath();
  ctx.moveTo(p,p);ctx.lineTo(p,p+h);ctx.lineTo(p+w,p+h);ctx.stroke();
  ctx.fillStyle='#888';ctx.font='10px monospace';ctx.textAlign='right';
  for(let i=0;i<=4;i++){
    const y=p+h-(i/4)*h, v=Math.round((i/4)*maxY);
    ctx.fillText(v+'ms',p-5,y+3);
    if(i>0){ctx.strokeStyle='#333';ctx.beginPath();ctx.moveTo(p,y);ctx.lineTo(p+w,y);ctx.stroke();}
  }
  ctx.textAlign='center';ctx.fillText('Time',p+w/2,c.height-5);
  ctx.strokeStyle='#00ff41';ctx.lineWidth=2;ctx.beginPath();
  points.forEach((o,i)=>{const x=p+(i/maxX)*w,y=p+h-(o.y/maxY)*h;i?ctx.lineTo(x,y):ctx.moveTo(x,y);});
  ctx.stroke();ctx.fillStyle='#00ff41';
  points.forEach((o,i)=>{const x=p+(i/maxX)*w,y=p+h-(o.y/maxY)*h;ctx.beginPath();ctx.arc(x,y,2,0,2*Math.PI);ctx.fill();});
}

function updateServo(){
  userIsAdjustingSliders = true;
  lastUserSliderUpdate = Date.now();
  document.getElementById('s1v').textContent=document.getElementById('s1').value+'°';
  document.getElementById('s2v').textContent=document.getElementById('s2').value+'°';
  setTimeout(() => userIsAdjustingSliders = false, 1000);
}

function updateMotor(){
  userIsAdjustingSliders = true;
  lastUserSliderUpdate = Date.now();
  document.getElementById('mvv').textContent=document.getElementById('mv').value;
  document.getElementById('mtv').textContent=document.getElementById('mt').value+'ms';
  setTimeout(() => userIsAdjustingSliders = false, 1000);
}

document.addEventListener('DOMContentLoaded', function() {
  const sliders = ['s1', 's2', 'mv', 'mt', 'ms'];
  sliders.forEach(id => {
    const element = document.getElementById(id);
    if(element) {
      element.addEventListener('input', () => {
        userIsAdjustingSliders = true;
        lastUserSliderUpdate = Date.now();
      });
      element.addEventListener('mousedown', () => {
        userIsAdjustingSliders = true;
        lastUserSliderUpdate = Date.now();
      });
      element.addEventListener('touchstart', () => {
        userIsAdjustingSliders = true;
        lastUserSliderUpdate = Date.now();
      });
    }
  });
});

function sendCmd(cmd){ws.send(JSON.stringify({cmd}));}
function sendServos(){ws.send(JSON.stringify({cmd:'setServos',a1:+document.getElementById('s1').value,a2:+document.getElementById('s2').value}));}
function sendMotor(){ws.send(JSON.stringify({cmd:'setMotor',vel:+document.getElementById('mv').value,time:+document.getElementById('mt').value,sel:+document.getElementById('ms').value}));}

ws.onopen=()=>{
  updateGamepadList();createMappingTable();
  setInterval(pollGamepad,16);
  setInterval(()=>{
    lastSent=Date.now();ws.send(JSON.stringify({cmd:'ping'}));
  },1000);
  setInterval(drawLatency,100);
};

ws.onmessage=e=>{
  const obj=JSON.parse(e.data);
  if(obj.cmd==='pong'){
    const lat=Date.now()-lastSent;latencies.push(lat);points.push({y:lat});
    if(latencies.length>50){latencies.shift();points.shift();}
    document.getElementById('avg').textContent=(latencies.reduce((a,b)=>a+b,0)/latencies.length).toFixed(0);
    document.getElementById('std').textContent=Math.sqrt(latencies.reduce((a,b)=>a+Math.pow(b-(latencies.reduce((a,b)=>a+b,0)/latencies.length),2),0)/latencies.length).toFixed(0);
    document.getElementById('current').textContent=lat;
    if(document.getElementById('perfSection').style.display!=='none') drawLatency();
  }
  if(obj.serial){obj.serial.trim().split(/[\r\n]+/).forEach(l=>appendSerial(l));}
  
  const timeSinceLastUserUpdate = Date.now() - lastUserSliderUpdate;
  const canUpdateSliders = !userIsAdjustingSliders && 
                          obj.cmd !== 'gamepad' && 
                          timeSinceLastUserUpdate > 2000 &&
                          (obj.servo1 !== undefined || obj.servo2 !== undefined || 
                           obj.motorVel !== undefined || obj.motorTime !== undefined || 
                           obj.motorSel !== undefined);
  
  if(canUpdateSliders) {
    ['servo1','servo2','motorVel','motorTime','motorSel'].forEach(k=>{
      if(obj[k]!==undefined){
        const id=k==='servo1'?'s1':k==='servo2'?'s2':k==='motorVel'?'mv':k==='motorTime'?'mt':'ms';
        const element = document.getElementById(id);
        if(element && element.value != obj[k]) {
          element.value=obj[k];
          if(k==='servo1'||k==='servo2')document.getElementById(id+'v').textContent=obj[k]+'°';
          if(k==='motorVel')document.getElementById('mvv').textContent=obj[k];
          if(k==='motorTime')document.getElementById('mtv').textContent=obj[k]+'ms';
        }
      }
    });
  }
};

window.addEventListener('gamepadconnected',updateGamepadList);
window.addEventListener('gamepaddisconnected',updateGamepadList);
</script>
</body>
</html>
)rawliteral";

#endif