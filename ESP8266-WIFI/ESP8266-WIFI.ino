#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "index_html.h"
#include <Arduino.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"
#include "audio.h"

// ======= CONFIG =======
const char* ssid     = "Mov";
const char* password = "12345678";

const int audioPin   = 3;
const int ledESP     = 14;
const int ojoIzqPin  = 4;    // ojo izquierdo
const int ojoDerPin  = 16;   // ojo derecho
const int cabezaPin  = 12;   // sirena

// ======= PWM =======
const int PWM_MAX = 1023;
const int PWM_MIN = 0;

// ======= OPTIMIZED CONSTANTS =======
const size_t MAX_SERIAL_BUF   = 128;
const float  JOY_L_THRESHOLD  = 0.05f;
const uint32_t DC_SEND_INTERVAL  = 50;
const uint32_t BROADCAST_INTERVAL = 20;
const uint8_t  DC_THRESHOLD      = 5;
const uint32_t SERIAL_TIMEOUT    = 1000;


// ======= SIRENA STATE =======
struct SirenState {
  bool     active     = false;
  uint32_t startTime  = 0;
  uint16_t freqLo     = 800;
  uint16_t freqHi     = 2000;
  uint32_t period     = 2000; // ms para barrido completo
} siren;

// ======= BLINK PATTERNS OJOS =======
const uint8_t  N_PATTERNS    = 4;
const uint8_t  MAX_BLINK_LEN = 8;
const uint32_t blinkPatterns[N_PATTERNS][MAX_BLINK_LEN] = {
  {  80,200,   0,0,0,0,0,0 },  // rápido
  { 150, 50,150,200, 0,0,0,0 }, // doble
  { 200,500,   0,0,0,0,0,0 },   // lento
  {  80, 80,120,120, 90,300,0,0 } // mixto
};
uint8_t  curPatternIdx = 0;
uint8_t  curBlinkStep  = 0;
uint32_t nextBlinkTime = 0;
bool     blinkOn       = false;

// ======= NON-BLOCKING PAUSE =======
uint32_t pauseEnd = 0;
void delayFreePause(uint32_t ms) { pauseEnd = millis() + ms; }
bool inPause() { return millis() < pauseEnd; }

// ======= FLAG PARA MANTENER OJOS ENCENDIDOS =======
bool holdEyesOn = false;
bool blinkPatternFinished = false;

// ======= SIRENA PWM PARA CABEZA =======
struct HeadFadeState {
  int     currentValue;
  bool    increasing;
  uint32_t lastUpdate;
  bool    inPause;
  uint32_t pauseEnd;
} headFade;

// Cada ciclo completo (subida + bajada) tarda 2000 ms
const uint32_t HEAD_PERIOD    = 2000;
const uint32_t HEAD_STEP_MS   = 20;
const int      HEAD_STEP      = PWM_MAX / (HEAD_PERIOD / (2 * HEAD_STEP_MS));

void handleHeadSirenBlink() {
  uint32_t now = millis();
  if (now - headFade.lastUpdate < HEAD_STEP_MS) return;
  headFade.lastUpdate = now;
  if (headFade.increasing) {
    headFade.currentValue = min(headFade.currentValue + HEAD_STEP, PWM_MAX);
    if (headFade.currentValue >= PWM_MAX) headFade.increasing = false;
  } else {
    headFade.currentValue = max(headFade.currentValue - HEAD_STEP, PWM_MIN);
    if (headFade.currentValue <= PWM_MIN) headFade.increasing = true;
  }
  analogWrite(cabezaPin, headFade.currentValue);
}

// ======= PWM FADE STATE (para breathing cuando no blink en ojos) =======
struct PWMFadeState {
  int currentValue = PWM_MIN;
  int targetValue  = PWM_MIN;
  uint32_t lastUpdate = 0;
  bool isActive    = false;
} pwmState;

void setPWMTarget(int target) {
  pwmState.targetValue = constrain(target, PWM_MIN, PWM_MAX);
  pwmState.isActive    = true;
  pwmState.lastUpdate  = millis();
}

void handlePWMFade() {
  if (!pwmState.isActive) return;
  uint32_t now = millis();
  if (now - pwmState.lastUpdate < 20) return;
  if (pwmState.currentValue != pwmState.targetValue) {
    if (pwmState.currentValue < pwmState.targetValue) {
      pwmState.currentValue = min(pwmState.currentValue + 20, pwmState.targetValue);
    } else {
      pwmState.currentValue = max(pwmState.currentValue - 20, pwmState.targetValue);
    }
    analogWrite(ojoIzqPin, pwmState.currentValue);
    analogWrite(ojoDerPin, pwmState.currentValue);
    pwmState.lastUpdate = now;
  } else {
    pwmState.isActive = false;
  }
}
// ======= STATE STRUCT (ACTUALIZADA) =======
struct __attribute__((packed)) State {
  uint8_t flags;       // bit0: espOn, bit1: unoOn, bit2: dcOn, bit3: playingMelody, bit4: playingAudio
  uint8_t servo1, servo2;
  int16_t motorVel;
  uint16_t motorTime;
  uint8_t motorSel;
  uint8_t dcValue;
  int16_t joyLX_i, joyLY_i, joyRX_i, joyRY_i;
  uint8_t trig6_i, trig7_i;
  uint16_t btnState;
  int16_t pitch_i, roll_i, yaw_i;
  int16_t accelX_i, accelY_i, accelZ_i;
  int16_t gyroX_i, gyroY_i, gyroZ_i;  // ¡AGREGADO!
  int16_t temperature_i;
  uint16_t ultrasonic_i;
  int16_t setPoint_i;
  uint16_t kp_i, ki_i, kd_i;
  int16_t pidError_i, pidOutput_i;
  uint16_t pidAngle;
  int16_t currentRPM_i;
} state;

inline bool getEspOn()        { return state.flags & 0x01; }
inline bool getUnoOn()        { return state.flags & 0x02; }
inline bool getDcOn()         { return state.flags & 0x04; }
inline bool isPlayingMelody() { return state.flags & 0x08; }
inline bool isPlayingAudio()  { return state.flags & 0x10; }
inline void setEspOn(bool v)        { state.flags = (state.flags & ~0x01) | (v ? 0x01 : 0); }
inline void setUnoOn(bool v)        { state.flags = (state.flags & ~0x02) | (v ? 0x02 : 0); }
inline void setDcOn(bool v)         { state.flags = (state.flags & ~0x04) | (v ? 0x04 : 0); }
inline void setPlayingMelody(bool v){ state.flags = (state.flags & ~0x08) | (v ? 0x08 : 0); }
inline void setPlayingAudio(bool v) { state.flags = (state.flags & ~0x10) | (v ? 0x10 : 0); }

// ======= GLOBALS =======
String serialBuf;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

// ======= SERIAL BUFFER =======
void appendSerialBuf(const String &s) {
  serialBuf += s;
  if (serialBuf.length() > MAX_SERIAL_BUF) {
    serialBuf = serialBuf.substring(serialBuf.length() - MAX_SERIAL_BUF);
  }
}

// ======= JOYSTICK STATE =======
struct JoystickState {
  float lastFX = 0.0f, lastFY = 0.0f;
  uint32_t lastUpdate = 0;
} joyState;

// ======= DC CONTROL STATE =======
struct DCState {
  uint32_t lastSent = 0;
  uint8_t  lastValue = 0;
} dcState;

// ======= AUDIO PLAYBACK =======
AudioGeneratorWAV*      wavPlayer;
AudioFileSourcePROGMEM* wavFile;
AudioOutputI2SNoDAC*    wavOut;

void initAudio() {
  wavOut    = new AudioOutputI2SNoDAC();
  wavPlayer = new AudioGeneratorWAV();
}

void playAudio() {
  if (isPlayingAudio()) return;
  if (wavFile) { delete wavFile; wavFile = nullptr; }
  wavFile = new AudioFileSourcePROGMEM(audio, sizeof(audio));
  wavPlayer->begin(wavFile, wavOut);
  setPlayingAudio(true);
}

void handleAudio() {
  if (!isPlayingAudio()) return;
  if (!wavPlayer->loop()) {
    wavPlayer->stop();
    setPlayingAudio(false);
  }
}

// ======= ROBOT-STYLE MELODY =======
const uint16_t melodyData[] PROGMEM = {
  1000,100, 800,100, 600,100, 400,100,
   400,100, 600,100, 800,100,1000,200
};
const uint8_t melodyLen = 8;
struct MelodyState {
  uint8_t  noteIndex   = 0;
  uint32_t noteEndTime = 0;
} melodyState;

void playMelodyAsync() {
  setPlayingMelody(true);
  melodyState.noteIndex   = 0;
  melodyState.noteEndTime = millis();
}

void handleMelody() {
  if (!isPlayingMelody()) return;
  uint32_t now = millis();
  if (now < melodyState.noteEndTime) return;
  if (melodyState.noteIndex >= melodyLen) {
    noTone(audioPin);
    setPlayingMelody(false);
    return;
  }
  uint16_t freq = pgm_read_word(&melodyData[melodyState.noteIndex*2]);
  uint16_t dur  = pgm_read_word(&melodyData[melodyState.noteIndex*2+1]);
  tone(audioPin, freq, dur);
  melodyState.noteEndTime = now + dur + 50;
  melodyState.noteIndex++;
}

// ======= DC UPDATE =======
void updateDCFromTrigger() {
  uint32_t now = millis();
  if (now - dcState.lastSent < DC_SEND_INTERVAL) return;
  uint8_t dcVal = state.trig7_i;
  if (abs(dcVal - dcState.lastValue) > DC_THRESHOLD) {
    char buf[16];
    float df = dcVal/255.0f;
    snprintf_P(buf, sizeof(buf), PSTR("[DC,%.2f]"), df);
    Serial.println(buf);
    //Serial1.println(buf);
    appendSerialBuf(buf);
    setDcOn(dcVal > 0);
    state.dcValue    = dcVal;
    dcState.lastValue = dcVal;
    dcState.lastSent  = now;
  }
}

// ======= OPTIMIZED SERIAL INPUT =======
void handleSerial() {
  static String lineBuf;
  static uint32_t lastActivity = 0;
  uint32_t now = millis();
  
  while (Serial.available()) {
    char c = Serial.read();
    lastActivity = now;
    
    if (c == '\n' || c == '\r') {
      if (lineBuf.length() > 0) {
        processSerialLine(lineBuf+ "\n");
        lineBuf = "";
      }
    } else {
      lineBuf += c;
      // Prevent buffer overflow
      if (lineBuf.length() > 64) {
        lineBuf = "";
      }
    }
  }
  
  // Clear incomplete lines after timeout
  if (lineBuf.length() > 0 && (now - lastActivity > SERIAL_TIMEOUT)) {
    lineBuf = "";
  }
}
// Llama a processSerialLine(line) cada vez que recibas una línea completa de Serial
void processSerialLine(const String& line) {
  if (line.length() < 3) return;
  char type = line.charAt(1);
  const char* data = line.c_str();

  // Siempre guarda la línea cruda para mostrarla en la web
  appendSerialBuf(line);

  switch (type) {
    case 'A':  // ANGLES o ACCEL
      if (line.startsWith("[ANGLES,")) {
        float p, r, y;
        if (sscanf(data + 8, "%f,%f,%f", &p, &r, &y) == 3) {
          state.pitch_i = (int16_t)round(p * 100);
          state.roll_i  = (int16_t)round(r * 100);
          state.yaw_i   = (int16_t)round(y * 100);
        }
      }
      else if (line.startsWith("[ACCEL,")) {
        float ax, ay, az;
        if (sscanf(data + 7, "%f,%f,%f", &ax, &ay, &az) == 3) {
          state.accelX_i = (int16_t)round(ax * 1000);
          state.accelY_i = (int16_t)round(ay * 1000);
          state.accelZ_i = (int16_t)round(az * 1000);
        }
      }
      break;

    case 'G':  // GYRO
      if (line.startsWith("[GYRO,")) {
        float gx, gy, gz;
        if (sscanf(data + 6, "%f,%f,%f", &gx, &gy, &gz) == 3) {
          state.gyroX_i = (int16_t)round(gx * 100);
          state.gyroY_i = (int16_t)round(gy * 100);
          state.gyroZ_i = (int16_t)round(gz * 100);
        }
      }
      break;

    case 'T':  // TEMP
      if (line.startsWith("[TEMP,")) {
        float T;
        if (sscanf(data + 6, "%f", &T) == 1) {
          state.temperature_i = (int16_t)round(T * 100);
        }
      }
      break;

    case 'U':  // ULTRASONIC
      if (line.startsWith("[US,")) {
        float d;
        if (sscanf(data + 4, "%f", &d) == 1) {
          state.ultrasonic_i = (uint16_t)round(d * 100);
        }
      }
      break;

    case 'P':  // PID feedback
      if (line.startsWith("[PID,")) {
        float e, o; int a;
        if (sscanf(data + 5, "%f,%f,%d", &e, &o, &a) == 3) {
          state.pidError_i  = (int16_t)round(e * 100);
          state.pidOutput_i = (int16_t)round(o * 100);
          state.pidAngle    = (uint16_t)a;
        }
      }
      break;

    case 'R':  // RPM
      if (line.startsWith("[RPM,")) {
        float rpm;
        if (sscanf(data + 5, "%f", &rpm) == 1) {
          state.currentRPM_i = (int16_t)round(rpm * 10.0f);
        }
      }
      break;

    default:
      // Otros tipos de mensajes: los mostramos crudos pero no parseamos
      break;
  }
}




// ======= BROADCAST STATE (ACTUALIZADO) =======
void broadcastState() {
  handleSerial();
  DynamicJsonDocument doc(512);  // Aumentado el tamaño para datos adicionales
  
  doc["espOn"]      = getEspOn();
  doc["unoOn"]      = getUnoOn();
  doc["dcOn"]       = getDcOn();
  doc["servo1"]     = state.servo1;
  doc["servo2"]     = state.servo2;
  doc["motorVel"]   = state.motorVel;
  doc["motorTime"]  = state.motorTime;
  doc["motorSel"]   = state.motorSel;
  doc["dcValue"]    = state.dcValue;
  
  // Joystick
  doc["joyLX"]      = state.joyLX_i*(1.0f/32767.0f);
  doc["joyLY"]      = state.joyLY_i*(1.0f/32767.0f);
  doc["joyRX"]      = state.joyRX_i*(1.0f/32767.0f);
  doc["joyRY"]      = state.joyRY_i*(1.0f/32767.0f);
  doc["trig6"]      = state.trig6_i*(1.0f/255.0f);
  doc["trig7"]      = state.trig7_i*(1.0f/255.0f);
  
  // Sensores
  doc["ultrasonic"] = state.ultrasonic_i*0.01f;
  doc["temperature"]= state.temperature_i*0.01f;
  
  // IMU - Ángulos
  doc["pitch"]      = state.pitch_i*0.01f;
  doc["roll"]       = state.roll_i*0.01f;
  doc["yaw"]        = state.yaw_i*0.01f;
  
  // IMU - Acelerómetro
  doc["accelX"]     = state.accelX_i*0.001f;
  doc["accelY"]     = state.accelY_i*0.001f;
  doc["accelZ"]     = state.accelZ_i*0.001f;
  
  // IMU - Giroscopio ¡AGREGADO!
  doc["gyroX"]      = state.gyroX_i*0.01f;
  doc["gyroY"]      = state.gyroY_i*0.01f;
  doc["gyroZ"]      = state.gyroZ_i*0.01f;
  
  // PID
  doc["pidError"]   = state.pidError_i*0.01f;
  doc["pidOutput"]  = state.pidOutput_i*0.01f;
  doc["pidAngle"]   = state.pidAngle;
  doc["kp"]         = state.kp_i*0.01f;
  doc["ki"]         = state.ki_i*0.01f;
  doc["kd"]         = state.kd_i*0.01f;
  doc["currentRPM"] = state.currentRPM_i*0.1f;
  
  if (serialBuf.length()>0) {
    doc["serial"] = serialBuf;
  }
  
  String out; 
  out.reserve(768);  // Aumentado para datos adicionales
  serializeJson(doc,out);
  webSocket.broadcastTXT(out);
  serialBuf="";
}

// ======= SIRENA CONTROL =======
void startSiren() {
  siren.active    = true;
  siren.startTime = millis();
}
void stopSiren() {
  siren.active = false;
  noTone(cabezaPin);
}
void handleSiren() {
  if (!siren.active) return;
  uint32_t t = (millis() - siren.startTime) % siren.period;
  float phase = float(t) / siren.period;
  if (phase > 0.5f) phase = 1.0f - phase;
  uint16_t f = siren.freqLo + (siren.freqHi - siren.freqLo) * (phase * 2.0f);
  tone(cabezaPin, f);
}

// ======= BLINK CONTROL =======
void pickNewBlink() {
  curPatternIdx = random(N_PATTERNS);
  curBlinkStep  = 0;
  blinkOn       = true;
  nextBlinkTime = millis();
  blinkPatternFinished = false;
}

void handleBlinkPWM() {
  uint32_t now = millis();
  if (now < nextBlinkTime) return;
  
  blinkOn = !blinkOn;
  analogWrite(ojoIzqPin, blinkOn ? PWM_MAX : PWM_MIN);
  analogWrite(ojoDerPin, blinkOn ? PWM_MAX : PWM_MIN);
  
  uint32_t interval = blinkPatterns[curPatternIdx][curBlinkStep++];
  if (interval == 0) {
    // Patrón terminado - asegurar que los LEDs queden encendidos
    analogWrite(ojoIzqPin, PWM_MAX);
    analogWrite(ojoDerPin, PWM_MAX);
    blinkPatternFinished = true;
    delayFreePause(random(5000,12000));
    pickNewBlink();
  } else {
    nextBlinkTime = now + interval;
  }
}

// ======= WEBSOCKET CALLBACK =======
void onWsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (type != WStype_TEXT) return;
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc,payload,len)) return;
  const char* cmd = doc["cmd"];
  if (!cmd) return;

  if      (strcmp(cmd, "ping")       == 0) webSocket.sendTXT(num, "{\"cmd\":\"pong\"}");
  else if (strcmp(cmd, "playMelody") == 0) { appendSerialBuf("[MELODY]"); playMelodyAsync(); }
  else if (strcmp(cmd, "playAudio")  == 0) { appendSerialBuf("[AUDIO]");  playAudio(); }
  else if (strcmp(cmd, "startSiren") == 0) startSiren();
  else if (strcmp(cmd, "stopSiren")  == 0) stopSiren();
  else if (strcmp(cmd, "blinkNow")   == 0) pickNewBlink();
  else if (strcmp(cmd, "gamepad")    == 0) handleGamepadInput(doc);
  else if (strcmp(cmd, "unoOn")      == 0) { appendSerialBuf("[UNOON]"); setUnoOn(true); }
  else if (strcmp(cmd, "unoOff")     == 0) { appendSerialBuf("[UNOOFF]"); setUnoOn(false); }
  else if (strcmp(cmd, "dcOn")       == 0) { appendSerialBuf("[DCON]"); setDcOn(true); }
  else if (strcmp(cmd, "dcOff")      == 0) { appendSerialBuf("[DCOFF]"); setDcOn(false); }
  else if (strcmp(cmd, "setServos")  == 0) handleSetServos(doc);
  else if (strcmp(cmd, "setMRPM")    == 0) handleSetMRPM(doc);
  else if (strcmp(cmd, "setPID")     == 0) handleSetPID(doc);
}

// ======= GAMEPAD & COMMAND HANDLERS =======
void handleGamepadInput(JsonDocument& doc) {
  float fx = doc["lx"], fy = doc["ly"],
        frx= doc["rx"], fry= doc["ry"];
  float tl = doc["tl"], tr = doc["tr"];
  if (fabs(fx)<JOY_L_THRESHOLD) fx=0; if (fabs(fy)<JOY_L_THRESHOLD) fy=0;
  if (fabs(fx-joyState.lastFX)>JOY_L_THRESHOLD ||
      fabs(fy-joyState.lastFY)>JOY_L_THRESHOLD) {
    char buf[32];
    snprintf(buf,sizeof(buf), "[JOY_L:%.2f,%.2f]", fx, fy);
    Serial.println(buf); //Serial1.println(buf);
    appendSerialBuf(buf);
    joyState.lastFX=fx; joyState.lastFY=fy;
  }
  state.joyLX_i = fx*32767; state.joyLY_i = fy*32767;
  state.joyRX_i = frx*32767;state.joyRY_i = fry*32767;
  state.trig6_i = constrain((int)(tl*255),0,255);
  state.trig7_i = constrain((int)(tr*255),0,255);
  updateDCFromTrigger();
}

void handleSetServos(JsonDocument& doc) {
  uint8_t a1 = doc["a1"], a2 = doc["a2"];
  char buf[20];
  snprintf(buf,sizeof(buf), "[S,%u,%u]", a1, a2);
  Serial.println(buf); //Serial1.println(buf); 
  appendSerialBuf(buf);
  state.servo1 = a1; state.servo2 = a2;
}

void handleSetMRPM(JsonDocument& doc) {
  int rpm = doc["rpm"];
  char buf[20];
  snprintf(buf,sizeof(buf), "[MRPM,%d]", rpm);
  Serial.println(buf); //Serial1.println(buf); 
  appendSerialBuf(buf);
}

void handleSetPID(JsonDocument& doc) {
  float sp = doc["sp"], _kp=doc["kp"], _ki=doc["ki"], _kd=doc["kd"];
  state.setPoint_i=sp*100; state.kp_i=_kp*100; state.ki_i=_ki*100; state.kd_i=_kd*100;
  char buf[32];
  snprintf(buf,sizeof(buf), "[PIDT,%.2f,%.2f,%.2f,%.2f]", sp,_kp,_ki,_kd);
  Serial.println(buf); //Serial1.println(buf); 
  appendSerialBuf(buf);
}

// ======= SETUP & LOOP =======
void setup() {
  pinMode(ledESP, OUTPUT);
  pinMode(audioPin, OUTPUT);
  pinMode(ojoIzqPin, OUTPUT);
  pinMode(ojoDerPin, OUTPUT);
  pinMode(cabezaPin, OUTPUT);
  
  // Inicializar LEDs encendidos
  analogWrite(ojoIzqPin, PWM_MAX);
  analogWrite(ojoDerPin, PWM_MAX);
  analogWrite(cabezaPin, PWM_MIN);

  Serial.begin(115200);
  //Serial1.begin(115200);
  Serial.setRxBufferSize(256);
  //Serial1.setRxBufferSize(256);
  serialBuf.reserve(128);

  initAudio();
  memset(&state, 0, sizeof(state));
  state.servo1 = state.servo2 = 90;
  state.motorTime = 1000; 
  state.motorSel = 1;
  state.setPoint_i = 2500; 
  state.kp_i = 100; 
  state.ki_i = 2; 
  state.kd_i = 50;
  state.currentRPM_i = 0;
  
  // Inicializar valores de giroscopio
  state.gyroX_i = 0;
  state.gyroY_i = 0;
  state.gyroZ_i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 100) {
    delay(100); 
    attempts++;
  }

  server.on("/", [](){
    server.send_P(200, PSTR("text/html"), INDEX_HTML);
  });
  server.begin();
  analogWrite(ledESP, LOW);
  delay(250);
  analogWrite(ledESP, HIGH);
  delay(250);
  analogWrite(ledESP, LOW);
  delay(250);
  analogWrite(ledESP, HIGH);

  webSocket.begin();
  webSocket.onEvent(onWsEvent);

  randomSeed(analogRead(A0));
  curPatternIdx = random(N_PATTERNS);
  curBlinkStep  = 0;
  nextBlinkTime = millis();
  blinkPatternFinished = false;
  
  // Mensaje de depuración
  Serial.println("ESP8266 iniciado - Esperando datos del giroscopio...");
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // Parpadeo de ojos - modificado para mantener LEDs encendidos al final
  if (!inPause()) {
    uint32_t now = millis();
    if (now >= nextBlinkTime) {
      blinkOn = !blinkOn;
      analogWrite(ojoIzqPin, blinkOn ? PWM_MAX : PWM_MIN);
      analogWrite(ojoDerPin, blinkOn ? PWM_MAX : PWM_MIN);
      
      uint32_t interval = blinkPatterns[curPatternIdx][curBlinkStep++];
      if (interval == 0) {
        // Patrón terminado - asegurar que los LEDs queden encendidos
        analogWrite(ojoIzqPin, PWM_MAX);
        analogWrite(ojoDerPin, PWM_MAX);
        blinkPatternFinished = true;
        delayFreePause(random(5000,12000));
        curPatternIdx = random(N_PATTERNS);
        curBlinkStep  = 0;
      } else {
        nextBlinkTime = now + interval;
      }
    }
  } else {
    // Durante la pausa, mantener los LEDs encendidos si el patrón terminó
    if (blinkPatternFinished) {
      analogWrite(ojoIzqPin, PWM_MAX);
      analogWrite(ojoDerPin, PWM_MAX);
    } else {
      handlePWMFade();
    }
  }

  // Parpadeo de cabeza tipo sirena
  handleHeadSirenBlink();
  handleMelody();
  handleAudio();

  static uint32_t lastBroadcast = 0;
  uint32_t now = millis();
  if (now - lastBroadcast >= 20) {
    broadcastState();
    lastBroadcast = now;
  }

  yield();
}