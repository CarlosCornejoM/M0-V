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
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    body {
      font-family: Arial, sans-serif;
      background: #0a0a0a;
      color: #fff;
      overflow-x: hidden;
    }
    .container {
      max-width: 1400px;
      margin: 0 auto;
      padding: 1rem;
    }
    .header {
      text-align: center;
      margin-bottom: 1rem;
    }
    .header h1 {
      font-size: 1.5rem;
      font-weight: 600;
      color: #00ff41;
      margin-bottom: .5rem;
    }
    .header .connection {
      font-size: .9rem;
      color: #ccc;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 1rem;
      min-height: 80vh;
    }
    .card {
      background: #1a1a2e;
      border: 1px solid #333;
      border-radius: 8px;
      padding: 1rem;
    }

    /* Gamepad Card */
    .gamepad {
      display: flex;
      flex-direction: column;
      gap: 1rem;
      align-items: center;
      text-align: center;
      position: relative;
    }
    .gamepad select {
      background: #2a2a3e;
      color: #fff;
      border: 1px solid #444;
      border-radius: 4px;
      padding: .5rem;
      width: 100%;
    }
    .controller {
      position: relative;
      width: 100%;
      max-width: 500px;
    }
    .controller-body {
      width: 100%;
      height: 350px;
      background: #2a2a3e;
      border: 2px solid #444;
      border-radius: 60px 60px 40px 40px;
      position: relative;
    }
    .controller-body::before {
      content: '';
      position: absolute;
      top: 20px;
      left: 50%;
      transform: translateX(-50%);
      width: 80px;
      height: 8px;
      background: #333;
      border-radius: 4px;
    }
    .btn {
      position: absolute;
      width: 32px;
      height: 32px;
      border-radius: 50%;
      border: 2px solid #555;
      background: #2a2a3e;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 11px;
      font-weight: bold;
      color: #ccc;
    }
    .btn.active {
      background: #00ff41;
      border-color: #00ff41;
      color: #000;
    }
    .btn.face { right: 50px; }
    .btn.face.triangle { top: 50px; }
    .btn.face.square { top: 90px; right: 80px; }
    .btn.face.circle { top: 90px; right: 20px; }
    .btn.face.cross { top: 130px; right: 50px; }
    .btn.dpad { left: 50px; }
    .btn.dpad.up { top: 50px; }
    .btn.dpad.left { top: 90px; left: 20px; }
    .btn.dpad.right { top: 90px; left: 80px; }
    .btn.dpad.down { top: 130px; left: 50px; }
    .btn.shoulder {
      top: 5px;
      width: 50px;
      height: 20px;
      border-radius: 8px;
      font-size: 10px;
    }
    .btn.shoulder.l1 { left: 60px; }
    .btn.shoulder.r1 { right: 60px; }
    .btn.center {
      top: 110px;
      left: 50%;
      transform: translateX(-50%);
      width: 28px;
      height: 28px;
      font-size: 9px;
    }
    .btn.center.share { left: 40%; }
    .btn.center.options { left: 60%; }
    .stick {
      position: absolute;
      width: 70px;
      height: 70px;
      border: 3px solid #555;
      border-radius: 50%;
      background: #444;
    }
    .stick.left {
      bottom: 60px;
      left: 80px;
    }
    .stick.right {
      bottom: 60px;
      right: 80px;
    }
    .stick-dot {
      position: absolute;
      width: 14px;
      height: 14px;
      border-radius: 50%;
      background: #00ff41;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
    }
    .triggers {
      position: absolute;
      top: -15px;
      left: 50%;
      transform: translateX(-50%);
      width: 320px;
      height: 40px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .trigger-container {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .trigger-label {
      font-size: 10px;
      color: #888;
      margin-bottom: 2px;
    }
    .trigger-bar {
      width: 60px;
      height: 8px;
      background: #333;
      border-radius: 4px;
      overflow: hidden;
      border: 1px solid #555;
      position: relative;
    }
    .trigger-fill {
      height: 100%;
      background: #00ff41;
      border-radius: 3px;
      width: 0%;
    }

    .gamepad-controls {
      display: flex;
      gap: 1rem;
      justify-content: center;
      margin-top: 1rem;
    }

    .gamepad .gamepad-section {
      margin-top: 1rem;
    }

    .btn-action,
    .perf-toggle {
      background: #00ff41;
      color: #000;
      border: none;
      border-radius: 6px;
      padding: .5rem 1rem;
      cursor: pointer;
      font-weight: 600;
    }
    .servo-indicator {
      position: absolute;
      bottom: 10px;
      right: 10px;
      background: rgba(0,255,65,0.2);
      border: 1px solid #00ff41;
      border-radius: 4px;
      padding: 4px 8px;
      font-size: 10px;
      color: #00ff41;
      pointer-events: none;
    }

    /* Controls & other cards */
    .controls, .serial-section {
      display: flex;
      flex-direction: column;
      gap: 1rem;
    }
    .section {
      background: rgba(255,255,255,.05);
      border-radius: 8px;
      padding: 1rem;
    }
    .section h3 {
      font-size: 1rem;
      color: #00ff41;
      margin-bottom: .5rem;
    }
    .slider-group {
      display: flex;
      align-items: center;
      gap: .5rem;
      margin: .75rem 0;
    }
    .slider {
      flex: 1;
      height: 8px;
      border-radius: 4px;
      background: #333;
      outline: none;
    }
    .status {
      font-family: monospace;
      background: #000;
      border-radius: 6px;
      padding: 1rem;
      font-size: .8rem;
      max-height: 200px;
      overflow-y: auto;
      white-space: pre-wrap;
    }
    .sensor-data {
      background: rgba(255,255,255,.05);
      border-radius: 8px;
      padding: 1rem;
      margin-top: 1rem;
    }
    .sensor-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 1rem;
    }
    .sensor-item {
      background: rgba(0,0,0,.3);
      border-radius: 6px;
      padding: .75rem;
      border: 1px solid #333;
    }
    .mapping-section, .perf-section {
      display: none;
      background: rgba(255,255,255,.05);
      border-radius: 8px;
      padding: 1rem;
      margin-top: 1rem;
    }
    .mapping-table {
      width: 100%;
      border-collapse: collapse;
      margin-top: .5rem;
    }
    .mapping-table th, .mapping-table td {
      padding: .4rem;
      border: 1px solid #444;
    }

    @media (max-width: 1024px) {
      .controller-body { height: 300px; }
      .grid { grid-template-columns: 1fr; }
    }
    @media (max-width: 640px) {
      .controller-body { height: 280px; }
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
      <!-- Gamepad Card -->
      <div class="card gamepad">
        <select id="gpSelect">
          <option value="">(ninguno)</option>
        </select>
        <div class="controller">
          <div class="controller-body">
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
              <div class="trigger-container">
                <div class="trigger-label">L2</div>
                <div class="trigger-bar"><div class="trigger-fill" id="trigL"></div></div>
              </div>
              <div class="trigger-container">
                <div class="trigger-label">R2</div>
                <div class="trigger-bar"><div class="trigger-fill" id="trigR"></div></div>
              </div>
            </div>
          </div>
        </div>



        <div class="gamepad-section section">
          <h3>Audio & LEDs</h3>
          <button class="btn-action" onclick="sendCmd('playMelody')">🎵 Play Melody</button>
          <button class="btn-action" onclick="sendCmd('playAudio')">🔊 Play Audio</button>
          <button class="btn-action" onclick="sendCmd('espOn')">ESP ON</button>
          <button class="btn-action" onclick="sendCmd('espOff')">ESP OFF</button>
          <button class="btn-action" onclick="sendCmd('unoOn')">UNO ON</button>
          <button class="btn-action" onclick="sendCmd('unoOff')">UNO OFF</button>
          <button class="btn-action" onclick="sendCmd('dcOn')">DC ON</button>
          <button class="btn-action" onclick="sendCmd('dcOff')">DC OFF</button>
        </div>




        <div class="gamepad-controls">
          <button id="btnTogglePerf" class="perf-toggle">Mostrar Performance</button>
          <button id="btnToggleMapping" class="perf-toggle">Mostrar Mapping</button>
        </div>
        
        <!-- Performance Monitor -->
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

        <!-- Mapping Section -->
        <div class="card mapping-section" id="mappingSection">
          <h4>Button Mapping</h4>
          <table class="mapping-table" id="mappingTable">
            <thead>
              <tr><th>Button</th><th>Action</th><th>Type</th></tr>
            </thead>
            <tbody></tbody>
          </table>
          <button class="btn-action" onclick="saveMapping()">Save Mapping</button>
        </div>


        




        <div class="servo-indicator">Right Stick → Servos</div>
      </div>

      <!-- Controls Card -->
      <div class="card controls">
        
        <div class="section">
          <h3>Servos (Right Stick Auto)</h3>
          <div class="slider-group">
            <span>S1:</span>
            <input class="slider" id="s1" type="range" min="0" max="180" value="90" oninput="updateServo()">
            <span id="s1v">90°</span>
          </div>
          <div class="slider-group">
            <span>S2:</span>
            <input class="slider" id="s2" type="range" min="0" max="180" value="90" oninput="updateServo()">
            <span id="s2v">90°</span>
          </div>
          <button class="btn-action" onclick="sendServos()">Send Servos</button>
        </div>
        <div class="section">
          <h3>Nema17 Control</h3>
          <div class="slider-group">
            <span>Max RPM:</span>
            <input class="slider" id="maxRpm" type="range" min="0" max="60" value="30" oninput="updateNemaLabels()">
            <span id="maxRpmv">30</span> RPM
          </div>
          <button class="btn-action" onclick="sendNema17()">Set Max RPM</button>
          <div class="motor-info" style="margin-top:1rem; background:rgba(255,255,255,.05); padding:.75rem; border-radius:6px;">
            <p>Current RPM: <span id="currentRPM">0.0</span></p>
            <button class="btn-action" onclick="toggleRpmGraph()" id="btnToggleRpmGraph">Mostrar Gráfico RPM</button>
            <canvas id="rpmGraph" class="latency-graph" style="display:none; width:100%; height:200px;"></canvas>
          </div>
        </div>
        <div class="section">
          <h3>PID Tuning</h3>
          <div class="slider-group">
            <span>SP:</span>
            <input class="slider" id="sp" type="range" min="-100" max="0" value="-86" oninput="updatePIDLabels()">
            <span id="spv">25</span> cm
          </div>
          <div class="slider-group">
            <span>Kp:</span>
            <input class="slider" id="kp" type="range" min="0" max="1000" value="0" oninput="updatePIDLabels()">
            <span id="kpv">1.00</span>
          </div>
          <div class="slider-group">
            <span>Ki:</span>
            <input class="slider" id="ki" type="range" min="0" max="1000" value="0" oninput="updatePIDLabels()">
            <span id="kiv">0.02</span>
          </div>
          <div class="slider-group">
            <span>Kd:</span>
            <input class="slider" id="kd" type="range" min="0" max="1000" value="0" oninput="updatePIDLabels()">
            <span id="kdv">0.50</span>
          </div>
          <button class="btn-action" onclick="sendPID()">Send PID</button>
        </div>
      </div>

      <!-- Serial Monitor Card -->
      <div class="card serial-section">
        <h3>
          Serial Monitor
          <button onclick="document.getElementById('ser').textContent='-- Inicio de registro --\n';">Clear</button>
        </h3>
        <div class="status" id="ser">-- Inicio de registro --\n</div>
        <div class="sensor-data">
          <h3>Datos del Sensor</h3>
          <div class="sensor-grid">
            <div class="sensor-item">
              <h4>Ángulos (°)</h4>
              <div class="sensor-value">
                Pitch: <span id="pitch">0.00</span><br>
                Roll: <span id="roll">0.00</span><br>
                Yaw: <span id="yaw">0.00</span>
              </div>
            </div>
            <div class="sensor-item">
              <h4>Aceleración (g)</h4>
              <div class="sensor-value">
                X: <span id="accelX">0.000</span><br>
                Y: <span id="accelY">0.000</span><br>
                Z: <span id="accelZ">0.000</span>
              </div>
            </div>
            <div class="sensor-item">
              <h4>Temperatura (°C)</h4>
              <div class="sensor-value">T: <span id="temp">0.00</span></div>
            </div>
            <div class="sensor-item">
              <h4>Ultrasonido</h4>
              <div class="sensor-value">Distancia: <span id="usDist">0.00</span> cm</div>
            </div>


            <div class="sensor-item" style="grid-column: 1 / -1;">
  <h4>Control PID</h4>
  <div class="sensor-value">
    Error: <span id="pidErr">0.00</span> cm<br>
    Output: <span id="pidOut">0.00</span><br>
    Ángulo: <span id="pidAng">90</span>°
  </div>
  <button class="btn-action" onclick="togglePidGraph()" id="btnTogglePidGraph">Mostrar Gráfico PID</button>
  <div id="pidGraphContainer" style="display:none; margin-top:1rem;">
    <div style="margin-bottom:0.5rem;">
      <label><input type="checkbox" id="showError" checked> Error</label>
      <label><input type="checkbox" id="showOutput" checked> Output</label>
      <label><input type="checkbox" id="showAngle" checked> Ángulo</label>
    </div>
    <canvas id="pidGraph" class="latency-graph" style="width:100%; height:200px;"></canvas>
  </div>
</div>


          </div>
        </div>
      </div>



    </div>
  </div>


<script>
  // --- Variables globales ---
  let lastJoyLSent = 0;
  const JOY_L_SEND_INTERVAL = 50; // ms entre envíos
  let lastJoyL = { x: 0.00, y: 0.00 };

  const MAX_SERIAL_LINES = 20;
  let serialLines = [];

  // --- Consola serial ---
  function handleSerialLines(txt) {
    txt.trim().split(/[\r\n]+/).forEach(line => {
      serialLines.push(line);
      if (serialLines.length > MAX_SERIAL_LINES) {
        serialLines.shift();  // descartamos la más antigua
      }
    });
    const ser = document.getElementById('ser');
    ser.textContent = serialLines.join('\n');
    ser.scrollTop = ser.scrollHeight;
  }

  // --- WebSocket ---
  const ws = new WebSocket(`ws://${location.hostname}:81/`);
  let lastSent = 0;
  let latencies = [], points = [];
  let rpmData = [], rpmPoints = [];
  let pidData = { error: [], output: [], angle: [] };
  let pidPoints = { error: [], output: [], angle: [] };

  // --- Estado de los servos y gamepad ---
  let userIsAdjustingSliders = false;
  let lastUserSliderUpdate = 0;
  let lastServoSent = 0;
  const SERVO_SEND_INTERVAL = 50;
  let lastServoAngles = { a1: 90, a2: 90 };

  let buttonMappings = Array(16).fill().map(() => ({ action: '', type: 'onPress' }));
  buttonMappings[0] = { action: 'playMelody', type: 'onPress' };
  buttonMappings[1] = { action: 'espOff',     type: 'onPress' };
  buttonMappings[2] = { action: 'espOn',      type: 'onPress' };
  buttonMappings[3] = { action: 'playAudio',  type: 'onPress' };

  let prevButtonState = Array(16).fill(false);
  let state = {
    lx: 0, ly: 0,
    rx: 0, ry: 0,
    tl: 0, tr: 0,
    btn: Array(16).fill(false)
  };

  const btnElems    = Array(16).fill(null);
  const sensorElems = {};
  const sliderElems = {};

  // --- Inicialización de elementos DOM ---
  function initElements() {
    for (let i = 0; i < 16; i++) {
      btnElems[i] = document.getElementById(`btn${i}`);
    }
    [
      'pitch','roll','yaw',
      'accelX','accelY','accelZ',
      'temp','usDist','pidErr',
      'pidOut','pidAng'
    ].forEach(id => {
      sensorElems[id] = document.getElementById(id);
    });
    [
      's1','s2','mv','mt',
      's1v','s2v','mvv','mtv',
      'dotL','dotR','trigL','trigR'
    ].forEach(id => {
      sliderElems[id] = document.getElementById(id);
    });
  }

  // --- Toggle secciones ---
  function togglePerf() {
    const perf = document.getElementById('perfSection');
    const btn  = document.getElementById('btnTogglePerf');
    const isHidden = perf.style.display === 'none' || perf.style.display === '';
    perf.style.display = isHidden ? 'block' : 'none';
    btn.textContent     = isHidden ? 'Ocultar Performance' : 'Mostrar Performance';
  }

  function toggleMapping() {
    const mapping = document.getElementById('mappingSection');
    const btn     = document.getElementById('btnToggleMapping');
    const isHidden = mapping.style.display === 'none' || mapping.style.display === '';
    mapping.style.display = isHidden ? 'block' : 'none';
    btn.textContent       = isHidden ? 'Ocultar Mapping' : 'Mostrar Mapping';
  }

  function toggleRpmGraph() {
    const graph = document.getElementById('rpmGraph');
    const btn   = document.getElementById('btnToggleRpmGraph');
    const isHidden = graph.style.display === 'none' || graph.style.display === '';
    graph.style.display = isHidden ? 'block' : 'none';
    btn.textContent     = isHidden ? 'Ocultar Gráfico RPM' : 'Mostrar Gráfico RPM';
  }

  function togglePidGraph() {
  const container = document.getElementById('pidGraphContainer');
  const btn = document.getElementById('btnTogglePidGraph');
  const isHidden = container.style.display === 'none' || container.style.display === '';
  container.style.display = isHidden ? 'block' : 'none';
  btn.textContent = isHidden ? 'Ocultar Gráfico PID' : 'Mostrar Gráfico PID';
}

  function clearPerf() {
    latencies = [];
    points    = [];
    document.getElementById('avg').textContent     = '0';
    document.getElementById('std').textContent     = '0';
    document.getElementById('current').textContent = '0';
    const ctx = document.getElementById('latency').getContext('2d');
    ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
  }

  // --- Envío de línea a la consola ---
  function appendSerial(line) {
    const ser = document.getElementById('ser');
    ser.textContent += line + '\n';
    ser.scrollTop = ser.scrollHeight;
  }

  // --- Mapeo de sticks a servos ---
  function mapStickToServo(val) {
    return Math.round(((val + 1) / 2) * 180);
  }

  function updateServosFromStick() {
    const now = Date.now();
    if (now - lastServoSent < SERVO_SEND_INTERVAL) return;

    const a1 = mapStickToServo(state.rx);
    const a2 = mapStickToServo(-state.ry);

    if (Math.abs(a1 - lastServoAngles.a1) > 2 ||
        Math.abs(a2 - lastServoAngles.a2) > 2) {
      userIsAdjustingSliders = false;
      sliderElems.s1.value   = a1;
      sliderElems.s2.value   = a2;
      sliderElems.s1v.textContent = a1 + '°';
      sliderElems.s2v.textContent = a2 + '°';
      ws.send(JSON.stringify({ cmd: 'setServos', a1, a2 }));
      lastServoSent = now;
      lastServoAngles = { a1, a2 };
    }
  }

  function updateJoyLFromStick() {
    const now = Date.now();
    if (now - lastJoyLSent < JOY_L_SEND_INTERVAL) return;

    let fx = state.lx;
    let fy = state.ly;
    const DZ = 0.1;
    if (Math.abs(fx) < DZ) fx = 0;
    if (Math.abs(fy) < DZ) fy = 0;
    fx = parseFloat(fx.toFixed(2));
    fy = parseFloat(fy.toFixed(2));

    if (fx !== lastJoyL.x || fy !== lastJoyL.y) {
      ws.send(JSON.stringify({ cmd: 'JOY_L', x: fx, y: fy }));
      lastJoyL = { x: fx, y: fy };
      lastJoyLSent = now;
    }
  }

  // --- Tabla de mapeo de botones ---
  function createMappingTable() {
    const tbody = document.querySelector('#mappingTable tbody');
    tbody.innerHTML = '';
    const labels = ['□','○','✕','△','L1','R1','L2','R2','⚏','☰','L3','R3','↑','↓','←','→'];
    buttonMappings.forEach((bm, i) => {
      const row = tbody.insertRow();
      const cBtn = row.insertCell();
      const cA   = row.insertCell();
      const cT   = row.insertCell();
      cBtn.textContent = `${i}: ${labels[i]}`;

      const selA = document.createElement('select');
      ['', 'playMelody','playAudio','espOn','espOff','unoOn','unoOff','dcOn','dcOff']
        .forEach(v => {
          const o = document.createElement('option');
          o.value = v;
          o.textContent = v || '(none)';
          selA.appendChild(o);
        });
      selA.value = bm.action;
      selA.onchange = () => bm.action = selA.value;
      cA.appendChild(selA);

      const selT = document.createElement('select');
      ['onPress','onHold'].forEach(v => {
        const o = document.createElement('option');
        o.value = v;
        o.textContent = v;
        selT.appendChild(o);
      });
      selT.value = bm.type;
      selT.onchange = () => bm.type = selT.value;
      cT.appendChild(selT);
    });
  }

  function saveMapping() {
    console.log('Mappings saved', buttonMappings);
  }

  // --- Lista de gamepads disponibles ---
  function updateGamepadList() {
    const sel = document.getElementById('gpSelect');
    sel.innerHTML = '<option value="">(ninguno)</option>';
    navigator.getGamepads().forEach((g, i) => {
      if (g) {
        sel.insertAdjacentHTML(
          'beforeend',
          `<option value="${i}">${i}: ${g.id.substring(0,25)}</option>`
        );
      }
    });
  }

  // --- Lectura continua del gamepad ---
  function pollGamepad() {
    const idx = parseInt(document.getElementById('gpSelect').value);
    if (isNaN(idx)) return;
    const g = navigator.getGamepads()[idx];
    if (!g) return;

    g.buttons.forEach((b, i) => {
      if (i < 16) {
        const pressed = b.pressed;
        if (pressed !== prevButtonState[i]) {
          prevButtonState[i] = pressed;
          const bm = buttonMappings[i];
          const btnEl = btnElems[i];
          if (pressed) {
            btnEl.classList.add('active');
            if (bm.action && bm.type === 'onPress') sendCmd(bm.action);
          } else {
            btnEl.classList.remove('active');
          }
        }
        state.btn[i] = pressed;
      }
    });

    [ 'lx','ly','rx','ry' ].forEach((axis, idx) => {
      state[axis] = g.axes[idx] || 0;
    });
    state.tl = g.buttons[6]?.value || 0;
    state.tr = g.buttons[7]?.value || 0;

    updateVisuals();
    updateServosFromStick();
    updateJoyLFromStick();
    ws.send(JSON.stringify({ cmd: 'gamepad', ...state }));
  }

  function updateVisuals() {
    const r = 125;
    sliderElems.dotL.style.transform = `translate(${-50 + state.lx * r}%,${-50 + state.ly * r}%)`;
    sliderElems.dotR.style.transform = `translate(${-50 + state.rx * r}%,${-50 + state.ry * r}%)`;
    sliderElems.trigL.style.width   = `${state.tl * 100}%`;
    sliderElems.trigR.style.width   = `${state.tr * 100}%`;
  }

  // --- Gráficos de latencia y RPM ---
  function drawLatency() {
    const c = document.getElementById('latency');
    const ctx = c.getContext('2d');
    c.width = c.offsetWidth;
    c.height = 200;
    ctx.clearRect(0, 0, c.width, c.height);
    if (points.length < 2) return;

    const p = 30, w = c.width - 2*p, h = c.height - 2*p;
    const maxY = Math.max(...points.map(o => o.y), 50);
    const maxX = points.length - 1;

    // Ejes
    ctx.strokeStyle = '#555';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(p, p);
    ctx.lineTo(p, p+h);
    ctx.lineTo(p+w, p+h);
    ctx.stroke();

    // Líneas de referencia y etiquetas
    ctx.fillStyle = '#888';
    ctx.font = '10px monospace';
    ctx.textAlign = 'right';
    for (let i = 0; i <= 4; i++) {
      const y = p + h - (i/4)*h;
      const v = Math.round((i/4)*maxY);
      ctx.fillText(v + 'ms', p - 5, y+3);
      if (i > 0) {
        ctx.strokeStyle = '#333';
        ctx.beginPath();
        ctx.moveTo(p, y);
        ctx.lineTo(p+w, y);
        ctx.stroke();
      }
    }

    // Trazado de la curva
    ctx.textAlign = 'center';
    ctx.fillText('Time', p + w/2, c.height - 5);
    ctx.strokeStyle = '#00ff41';
    ctx.lineWidth = 2;
    ctx.beginPath();
    points.forEach((o, i) => {
      const x = p + (i/maxX)*w;
      const y = p + h - (o.y/maxY)*h;
      i ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
    });
    ctx.stroke();

    // Puntos
    ctx.fillStyle = '#00ff41';
    points.forEach((o, i) => {
      const x = p + (i/maxX)*w;
      const y = p + h - (o.y/maxY)*h;
      ctx.beginPath();
      ctx.arc(x, y, 2, 0, 2*Math.PI);
      ctx.fill();
    });
  }

  function drawRPMGraph() {
    const c = document.getElementById('rpmGraph');
    const ctx = c.getContext('2d');
    ctx.clearRect(0, 0, c.width, c.height);
    if (rpmPoints.length < 2) return;

    const p = 30, w = c.width - 2*p, h = c.height - 2*p;
    const minY = -30, maxY = 30, maxX = rpmPoints.length - 1;

    // Ejes
    ctx.strokeStyle = '#555';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(p, p);
    ctx.lineTo(p, p+h);
    ctx.lineTo(p+w, p+h);
    ctx.stroke();

    // Líneas de referencia
    ctx.fillStyle = '#888';
    ctx.font = '10px monospace';
    ctx.textAlign = 'right';
    for (let i = 0; i <= 4; i++) {
      const y = p + h - (i/4)*h;
      const v = Math.round(minY + (i/4)*(maxY - minY));
      ctx.fillText(v + 'rpm', p - 5, y+3);
      if (i > 0) {
        ctx.strokeStyle = '#333';
        ctx.beginPath();
        ctx.moveTo(p, y);
        ctx.lineTo(p+w, y);
        ctx.stroke();
      }
    }

    // Curva RPM
    ctx.textAlign = 'center';
    ctx.fillText('Time', p + w/2, c.height - 5);
    ctx.strokeStyle = '#00ff41';
    ctx.lineWidth = 2;
    ctx.beginPath();
    rpmPoints.forEach((o, i) => {
      const x = p + (i/maxX)*w;
      const y = p + h - ((o.y - minY)/(maxY - minY))*h;
      i ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
    });
    ctx.stroke();

    // Puntos
    ctx.fillStyle = '#00ff41';
    rpmPoints.forEach((o, i) => {
      const x = p + (i/maxX)*w;
      const y = p + h - ((o.y - minY)/(maxY - minY))*h;
      ctx.beginPath();
      ctx.arc(x, y, 2, 0, 2*Math.PI);
      ctx.fill();
    });
  }

  function drawPIDGraph() {
  const c = document.getElementById('pidGraph');
  const ctx = c.getContext('2d');
  ctx.clearRect(0, 0, c.width, c.height);
  
  const showError = document.getElementById('showError').checked;
  const showOutput = document.getElementById('showOutput').checked;
  const showAngle = document.getElementById('showAngle').checked;
  
  const maxPoints = Math.max(pidPoints.error.length, pidPoints.output.length, pidPoints.angle.length);
  if (maxPoints < 2) return;

  const p = 30, w = c.width - 2*p, h = c.height - 2*p;
  const maxX = maxPoints - 1;

  // Ejes
  ctx.strokeStyle = '#555';
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(p, p);
  ctx.lineTo(p, p+h);
  ctx.lineTo(p+w, p+h);
  ctx.stroke();

  // Encontrar rangos para normalización
  const allValues = [];
  if (showError) allValues.push(...pidPoints.error.map(p => p.y));
  if (showOutput) allValues.push(...pidPoints.output.map(p => p.y));
  if (showAngle) allValues.push(...pidPoints.angle.map(p => p.y));
  
  const minY = Math.min(...allValues, 0);
  const maxY = Math.max(...allValues, 100);

  // Líneas de referencia
  ctx.fillStyle = '#888';
  ctx.font = '10px monospace';
  ctx.textAlign = 'right';
  for (let i = 0; i <= 4; i++) {
    const y = p + h - (i/4)*h;
    const v = Math.round(minY + (i/4)*(maxY - minY));
    ctx.fillText(v, p - 5, y+3);
    if (i > 0) {
      ctx.strokeStyle = '#333';
      ctx.beginPath();
      ctx.moveTo(p, y);
      ctx.lineTo(p+w, y);
      ctx.stroke();
    }
  }

  // Función para dibujar línea
  function drawLine(points, color) {
    if (points.length < 2) return;
    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.beginPath();
    points.forEach((point, i) => {
      const x = p + (i/maxX)*w;
      const y = p + h - ((point.y - minY)/(maxY - minY))*h;
      i ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
    });
    ctx.stroke();
  }

  // Dibujar líneas según checkboxes
  if (showError) drawLine(pidPoints.error, '#ff4444');
  if (showOutput) drawLine(pidPoints.output, '#44ff44');
  if (showAngle) drawLine(pidPoints.angle, '#4444ff');

  // Leyenda
  ctx.fillStyle = '#888';
  ctx.font = '10px monospace';
  ctx.textAlign = 'left';
  let legendY = 15;
  if (showError) {
    ctx.fillStyle = '#ff4444';
    ctx.fillText('Error', 10, legendY);
    legendY += 12;
  }
  if (showOutput) {
    ctx.fillStyle = '#44ff44';
    ctx.fillText('Output', 10, legendY);
    legendY += 12;
  }
  if (showAngle) {
    ctx.fillStyle = '#4444ff';
    ctx.fillText('Ángulo', 10, legendY);
  }
}

  // --- Actualiza sliders y envíos manuales ---
  function updateServo() {
    userIsAdjustingSliders = true;
    lastUserSliderUpdate  = Date.now();
    sliderElems.s1v.textContent = sliderElems.s1.value + '°';
    sliderElems.s2v.textContent = sliderElems.s2.value + '°';
    setTimeout(() => userIsAdjustingSliders = false, 1000);
  }

  function updateMotor() {
    userIsAdjustingSliders = true;
    lastUserSliderUpdate  = Date.now();
    sliderElems.mvv.textContent = sliderElems.mv.value;
    sliderElems.mtv.textContent = sliderElems.mt.value + 'ms';
    setTimeout(() => userIsAdjustingSliders = false, 1000);
  }

  function sendCmd(cmd) {
    ws.send(JSON.stringify({ cmd }));
  }
  function sendServos() {
    ws.send(JSON.stringify({
      cmd: 'setServos',
      a1: +sliderElems.s1.value,
      a2: +sliderElems.s2.value
    }));
  }
  function sendMotor() {
    ws.send(JSON.stringify({
      cmd: 'setMotor',
      vel: +sliderElems.mv.value,
      time: +sliderElems.mt.value,
      sel: +document.getElementById('ms').value
    }));
  }

  // --- PID & Nema17 helpers ---
  function updatePIDLabels() {
    const sp = +document.getElementById('sp').value;
    document.getElementById('spv').textContent = sp;
    const kp = (document.getElementById('kp').value / 100).toFixed(2);
    const ki = (document.getElementById('ki').value / 100).toFixed(2);
    const kd = (document.getElementById('kd').value / 100).toFixed(2);
    document.getElementById('kpv').textContent = kp;
    document.getElementById('kiv').textContent = ki;
    document.getElementById('kdv').textContent = kd;
  }

  function sendPID() {
    const payload = {
      cmd: 'setPID',
      sp: +document.getElementById('sp').value,
      kp: +((document.getElementById('kp').value / 100).toFixed(2)),
      ki: +((document.getElementById('ki').value / 100).toFixed(2)),
      kd: +((document.getElementById('kd').value / 100).toFixed(2))
    };
    ws.send(JSON.stringify(payload));
  }

  function updateNemaLabels() {
    const v = document.getElementById('maxRpm').value;
    document.getElementById('maxRpmv').textContent = v;
  }

  function sendNema17() {
    const rpm = parseInt(document.getElementById('maxRpm').value, 10);
    ws.send(JSON.stringify({ cmd: 'setMRPM', rpm }));
  }

  // --- WebSocket eventos ---
  ws.onopen = () => {
    initElements();
    updateGamepadList();
    createMappingTable();
    updatePIDLabels();

    document.getElementById('btnTogglePerf').onclick        = togglePerf;
    document.getElementById('btnToggleMapping').onclick     = toggleMapping;
    document.getElementById('btnToggleRpmGraph').onclick    = toggleRpmGraph;
    document.getElementById('showError').onchange = drawPIDGraph;
    document.getElementById('showOutput').onchange = drawPIDGraph;
    document.getElementById('showAngle').onchange = drawPIDGraph;
    

    setInterval(pollGamepad, 16);
    setInterval(() => {
      lastSent = Date.now();
      ws.send(JSON.stringify({ cmd: 'ping' }));
    }, 1000);
    setInterval(drawLatency, 100);

    setInterval(() => {
    // Solo dibuja si el gráfico está visible
    if (document.getElementById('rpmGraph').style.display !== 'none') {
      drawRPMGraph();
    }
  }, 100);


  setInterval(() => {
      // Solo dibuja si el gráfico PID está visible
      if (document.getElementById('pidGraphContainer').style.display !== 'none') {
        drawPIDGraph();
      }
    }, 100);

  
  };

  ws.onmessage = e => {
    const obj = JSON.parse(e.data);

    // RPM
    if (obj.currentRPM !== undefined) {
      document.getElementById('currentRPM').textContent = obj.currentRPM.toFixed(1);
      rpmData.push(obj.currentRPM);
      rpmPoints.push({ y: obj.currentRPM });
      if (rpmData.length > 50) { rpmData.shift(); rpmPoints.shift(); }
      if (document.getElementById('rpmGraph').style.display !== 'none') {
        drawRPMGraph();
      }
    }

    // Pong / Latencia
    if (obj.cmd === 'pong') {
      const lat = Date.now() - lastSent;
      latencies.push(lat);
      points.push({ y: lat });
      if (latencies.length > 50) { latencies.shift(); points.shift(); }
      const avg = (latencies.reduce((a, b) => a + b, 0) / latencies.length).toFixed(0);
      const std = Math.sqrt(latencies
        .reduce((a, b) => a + Math.pow(b - avg, 2), 0) / latencies.length
      ).toFixed(0);

      document.getElementById('avg').textContent     = avg;
      document.getElementById('std').textContent     = std;
      document.getElementById('current').textContent = lat;
      if (document.getElementById('perfSection').style.display !== 'none') {
        drawLatency();
      }
    }

    // Serial
    if (obj.serial) handleSerialLines(obj.serial);

    // Auto-update sliders y controles
    const since     = Date.now() - lastUserSliderUpdate;
    const canUpdate = !userIsAdjustingSliders &&
                      obj.cmd !== 'gamepad' &&
                      since > 2000 &&
                      (obj.servo1   !== undefined ||
                       obj.servo2   !== undefined ||
                       obj.motorVel !== undefined ||
                       obj.motorTime!== undefined ||
                       obj.motorSel !== undefined);

    if (canUpdate) {
      ['servo1','servo2','motorVel','motorTime','motorSel'].forEach(k => {
        if (obj[k] !== undefined) {
          const id = k === 'servo1'   ? 's1'
                   : k === 'servo2'   ? 's2'
                   : k === 'motorVel' ? 'mv'
                   : k === 'motorTime'? 'mt'
                   :                    'ms';
          const el = document.getElementById(id);
          if (el && el.value != obj[k]) {
            el.value = obj[k];
            if (k === 'servo1' || k === 'servo2') {
              document.getElementById(id + 'v').textContent = obj[k] + '°';
            }
            if (k === 'motorVel') {
              document.getElementById('mvv').textContent = obj[k];
            }
            if (k === 'motorTime') {
              document.getElementById('mtv').textContent = obj[k] + 'ms';
            }
          }
        }
      });
    }

    // Ángulos
    if (obj.pitch !== undefined) {
      document.getElementById('pitch').textContent = obj.pitch.toFixed(2);
      document.getElementById('roll') .textContent = obj.roll .toFixed(2);
      document.getElementById('yaw')  .textContent = obj.yaw  .toFixed(2);
    }

    // Aceleración
    if (obj.accelX !== undefined) {
      document.getElementById('accelX').textContent = obj.accelX.toFixed(3);
      document.getElementById('accelY').textContent = obj.accelY.toFixed(3);
      document.getElementById('accelZ').textContent = obj.accelZ.toFixed(3);
    }

    // Temperatura
    if (obj.temperature !== undefined) {
      document.getElementById('temp').textContent = obj.temperature.toFixed(2);
    }

    // Ultrasonido
    if (obj.ultrasonic !== undefined) {
      document.getElementById('usDist').textContent = obj.ultrasonic.toFixed(2);
    }

    // Control PID
    if (obj.pidError !== undefined) {
      document.getElementById('pidErr').textContent = obj.pidError.toFixed(2);
      document.getElementById('pidOut').textContent = obj.pidOutput.toFixed(2);
      document.getElementById('pidAng').textContent = obj.pidAngle;
      
      // Almacenar datos para gráfico
      pidData.error.push(obj.pidError);
      pidData.output.push(obj.pidOutput);
      pidData.angle.push(obj.pidAngle);
      
      pidPoints.error.push({ y: obj.pidError });
      pidPoints.output.push({ y: obj.pidOutput });
      pidPoints.angle.push({ y: obj.pidAngle });
      
      // Limitar datos a 50 puntos
      if (pidData.error.length > 50) {
        pidData.error.shift();
        pidData.output.shift();
        pidData.angle.shift();
        pidPoints.error.shift();
        pidPoints.output.shift();
        pidPoints.angle.shift();
      }
    }


  };

  // --- Eventos de Gamepad ---
  window.addEventListener('gamepadconnected',    updateGamepadList);
  window.addEventListener('gamepaddisconnected', updateGamepadList);

// --- PID sliders helper ---
function updatePIDLabels() {
  const sp = document.getElementById('sp').value;
  document.getElementById('spv').textContent = sp;
  const kp = (document.getElementById('kp').value / 100).toFixed(2);
  const ki = (document.getElementById('ki').value / 100).toFixed(2);
  const kd = (document.getElementById('kd').value / 100).toFixed(2);
  document.getElementById('kpv').textContent = kp;
  document.getElementById('kiv').textContent = ki;
  document.getElementById('kdv').textContent = kd;
}



// Actualiza etiqueta del slider de Max RPM
function updateNemaLabels() {
  const v = document.getElementById('maxRpm').value;
  document.getElementById('maxRpmv').textContent = v;
}

// Envía el comando setMRPM al ESP8266
function sendNema17() {
  const rpm = parseInt(document.getElementById('maxRpm').value, 10);
  ws.send(JSON.stringify({ cmd: 'setMRPM', rpm }));
}



function sendPID() {
  const payload = {
    cmd: 'setPID',
    sp:  parseFloat(document.getElementById('sp').value),
    kp:  parseFloat((document.getElementById('kp').value / 100).toFixed(2)),
    ki:  parseFloat((document.getElementById('ki').value / 100).toFixed(2)),
    kd:  parseFloat((document.getElementById('kd').value / 100).toFixed(2))
  };
  console.log("Enviando setPID →", payload);
  ws.send(JSON.stringify(payload));

}

// Inicializa los labels al cargar la página
document.addEventListener('DOMContentLoaded', () => {
  updatePIDLabels();
});
</script>
</body>
</html>
)rawliteral";

#endif
