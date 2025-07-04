// ------------------ ESP8266‑WiFi‑Optimized.ino ------------------

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "index_html.h"

// ======= AUDIO INCLUDES =======
#include <Arduino.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"
#include "audio.h"

// ======= CONFIG =======
const char* ssid = "Mov";
const char* password = "12345678";
const int audioPin = 3;
const int ledESP   = 14;

// ======= OPTIMIZED CONSTANTS =======
const size_t MAX_SERIAL_BUF = 128;  // Increased for better buffering
const float JOY_L_THRESHOLD = 0.05f;
const uint32_t DC_SEND_INTERVAL = 50;
const uint32_t BROADCAST_INTERVAL = 20;  // Reduced from 5ms to 20ms
const uint8_t DC_THRESHOLD = 5;
const uint32_t SERIAL_TIMEOUT = 1000;  // 1 second timeout for serial lines

// ======= STATE STRUCT =======
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
  int16_t temperature_i;
  uint16_t ultrasonic_i;

  int16_t setPoint_i;
  uint16_t kp_i, ki_i, kd_i;
  int16_t pidError_i, pidOutput_i;
  uint16_t pidAngle;
  int16_t currentRPM_i;
} state;

// ======= FLAGS =======
inline bool getEspOn()       { return state.flags & 0x01; }
inline bool getUnoOn()       { return state.flags & 0x02; }
inline bool getDcOn()        { return state.flags & 0x04; }
inline bool isPlayingMelody(){ return state.flags & 0x08; }
inline bool isPlayingAudio() { return state.flags & 0x10; }
inline void setEspOn(bool v)     { state.flags = (state.flags & ~0x01) | (v ? 0x01 : 0); }
inline void setUnoOn(bool v)     { state.flags = (state.flags & ~0x02) | (v ? 0x02 : 0); }
inline void setDcOn(bool v)      { state.flags = (state.flags & ~0x04) | (v ? 0x04 : 0); }
inline void setPlayingMelody(bool v){ state.flags = (state.flags & ~0x08) | (v ? 0x08 : 0); }
inline void setPlayingAudio(bool v) { state.flags = (state.flags & ~0x10) | (v ? 0x10 : 0); }

// ======= GLOBALS =======
String serialBuf;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

// ======= OPTIMIZED SERIAL BUFFER =======
void appendSerialBuf(const String &s) {
  serialBuf += s;
  if (serialBuf.length() > MAX_SERIAL_BUF) {
    serialBuf = serialBuf.substring(serialBuf.length() - MAX_SERIAL_BUF);
  }
}

// ======= OPTIMIZED JOYSTICK TRACKING =======
struct JoystickState {
  float lastFX = 0.0f;
  float lastFY = 0.0f;
  uint32_t lastUpdate = 0;
} joyState;

// ======= OPTIMIZED DC CONTROL =======
struct DCState {
  uint32_t lastSent = 0;
  uint8_t lastValue = 0;
} dcState;

// ======= AUDIO PLAYBACK =======
AudioGeneratorWAV*     wavPlayer;
AudioFileSourcePROGMEM* wavFile;
AudioOutputI2SNoDAC*   wavOut;

void initAudio() {
  wavOut    = new AudioOutputI2SNoDAC();
  wavPlayer = new AudioGeneratorWAV();
}

void playAudio() {
  if (isPlayingAudio()) return;
  if (wavFile) {
    delete wavFile;
    wavFile = nullptr;
  }
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

// ======= OPTIMIZED MELODY =======
const uint16_t melodyData[] PROGMEM = {
  262,300, 294,300, 330,300, 349,300, 392,300, 440,300, 494,300, 523,600
};
const uint8_t melodyLen = 8;

struct MelodyState {
  uint8_t noteIndex = 0;
  uint32_t noteEndTime = 0;
} melodyState;

void playMelodyAsync() {
  setPlayingMelody(true);
  melodyState.noteIndex = 0;
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
  
  uint16_t freq = pgm_read_word(&melodyData[melodyState.noteIndex * 2]);
  uint16_t dur  = pgm_read_word(&melodyData[melodyState.noteIndex * 2 + 1]);
  tone(audioPin, freq, dur);
  melodyState.noteEndTime = now + dur + 50;
  melodyState.noteIndex++;
}

// ======= OPTIMIZED DC CONTROL =======
void updateDCFromTrigger() {
  uint32_t now = millis();
  if (now - dcState.lastSent < DC_SEND_INTERVAL) return;
  
  uint8_t dcVal = state.trig7_i;
  if (abs(dcVal - dcState.lastValue) > DC_THRESHOLD) {
    char buf[16];
    float df = dcVal / 255.0f;
    snprintf_P(buf, sizeof(buf), PSTR("[DC,%.2f]"), df);
    Serial.println(buf);
    appendSerialBuf(buf);
    Serial1.println(buf);
    setDcOn(dcVal > 0);
    state.dcValue = dcVal;
    dcState.lastValue = dcVal;
    dcState.lastSent = now;
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
        processSerialLine(lineBuf);
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

void processSerialLine(const String& line) {
  if (line.length() < 3) return;
  
  char type = line.charAt(1);
  const char* data = line.c_str();
  
  switch (type) {
    case 'A':
      if (line.startsWith("[ANGLES,")) {
        float p, r, y;
        if (sscanf(data + 8, "%f,%f,%f", &p, &r, &y) == 3) {
          state.pitch_i = p * 100;
          state.roll_i = r * 100;
          state.yaw_i = y * 100;
        }
      } else if (line.startsWith("[ACCEL,")) {
        float ax, ay, az;
        if (sscanf(data + 7, "%f,%f,%f", &ax, &ay, &az) == 3) {
          state.accelX_i = ax * 1000;
          state.accelY_i = ay * 1000;
          state.accelZ_i = az * 1000;
        }
      }
      break;
      
    case 'T':
      if (line.startsWith("[TEMP,")) {
        float T;
        if (sscanf(data + 6, "%f", &T) == 1) {
          state.temperature_i = T * 100;
        }
      }
      break;
      
    case 'U':
      if (line.startsWith("[US,")) {
        float d;
        if (sscanf(data + 4, "%f", &d) == 1) {
          state.ultrasonic_i = d * 100;
        }
      }
      break;
      
    case 'P':
      if (line.startsWith("[PID,")) {
        float e, o;
        int a;
        if (sscanf(data + 5, "%f,%f,%d", &e, &o, &a) == 3) {
          state.pidError_i = e * 100;
          state.pidOutput_i = o * 100;
          state.pidAngle = a;
        }
      }
      break;
      
    case 'R':
      if (line.startsWith("[RPM,")) {
        float rpm;
        if (sscanf(data + 5, "%f", &rpm) == 1) {
          state.currentRPM_i = (int16_t)round(rpm * 10.0f);
          appendSerialBuf(line);
        }
      }
      break;
  }
}

// ======= OPTIMIZED BROADCAST STATE =======
void broadcastState() {
  handleSerial();
  
  // Use larger buffer for better performance
  DynamicJsonDocument doc(384);
  
  doc[F("espOn")]     = getEspOn();
  doc[F("unoOn")]     = getUnoOn();
  doc[F("dcOn")]      = getDcOn();
  doc[F("servo1")]    = state.servo1;
  doc[F("servo2")]    = state.servo2;
  doc[F("motorVel")]  = state.motorVel;
  doc[F("motorTime")] = state.motorTime;
  doc[F("motorSel")]  = state.motorSel;
  doc[F("dcValue")]   = state.dcValue;
  
  // Optimized floating point conversions
  doc[F("joyLX")]     = state.joyLX_i * (1.0f / 32767.0f);
  doc[F("joyLY")]     = state.joyLY_i * (1.0f / 32767.0f);
  doc[F("joyRX")]     = state.joyRX_i * (1.0f / 32767.0f);
  doc[F("joyRY")]     = state.joyRY_i * (1.0f / 32767.0f);
  doc[F("trig6")]     = state.trig6_i * (1.0f / 255.0f);
  doc[F("trig7")]     = state.trig7_i * (1.0f / 255.0f);
  
  doc[F("ultrasonic")] = state.ultrasonic_i * 0.01f;
  doc[F("pitch")]      = state.pitch_i * 0.01f;
  doc[F("roll")]       = state.roll_i * 0.01f;
  doc[F("yaw")]        = state.yaw_i * 0.01f;
  doc[F("accelX")]     = state.accelX_i * 0.001f;
  doc[F("accelY")]     = state.accelY_i * 0.001f;
  doc[F("accelZ")]     = state.accelZ_i * 0.001f;
  doc[F("temperature")] = state.temperature_i * 0.01f;
  doc[F("pidError")]   = state.pidError_i * 0.01f;
  doc[F("pidOutput")]  = state.pidOutput_i * 0.01f;
  doc[F("pidAngle")]   = state.pidAngle;
  doc[F("kp")]         = state.kp_i * 0.01f;
  doc[F("ki")]         = state.ki_i * 0.01f;
  doc[F("kd")]         = state.kd_i * 0.01f;
  doc[F("currentRPM")] = state.currentRPM_i * 0.1f;

  if (serialBuf.length() > 0) {
    doc[F("serial")] = serialBuf;
  }
  
  String out;
  out.reserve(512);
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);
  serialBuf = "";
}

// ======= OPTIMIZED WEBSOCKET CALLBACK =======
void onWsEvent(uint8_t num, WStype_t type, uint8_t* pl, size_t len) {
  if (type != WStype_TEXT) return;
  
  DynamicJsonDocument doc(192);
  if (deserializeJson(doc, pl, len)) return;
  
  const char* cmd = doc[F("cmd")];
  if (!cmd) return;

  // Use hash for faster string comparison
  switch (cmd[0]) {
    case 'p':
      if (strcmp_P(cmd, PSTR("ping")) == 0) {
        webSocket.sendTXT(num, "{\"cmd\":\"pong\"}");
      } else if (strcmp_P(cmd, PSTR("playMelody")) == 0) {
        appendSerialBuf(F("[MELODY]"));
        playMelodyAsync();
      } else if (strcmp_P(cmd, PSTR("playAudio")) == 0) {
        appendSerialBuf(F("[AUDIO]"));
        playAudio();
      }
      break;
      
    case 'g':
      if (strcmp_P(cmd, PSTR("gamepad")) == 0) {
        handleGamepadInput(doc);
      }
      break;
      
    case 'e':
      if (strcmp_P(cmd, PSTR("espOn")) == 0) {
        appendSerialBuf(F("[ESPON]"));
        Serial1.println(F("ESP ON"));
        digitalWrite(ledESP, HIGH);
        setEspOn(true);
      } else if (strcmp_P(cmd, PSTR("espOff")) == 0) {
        appendSerialBuf(F("[ESPOFF]"));
        Serial1.println(F("ESP OFF"));
        digitalWrite(ledESP, LOW);
        setEspOn(false);
      }
      break;
      
    case 'u':
      if (strcmp_P(cmd, PSTR("unoOn")) == 0) {
        appendSerialBuf(F("[UNOON]"));
        Serial.println("[UNOON]");
        Serial1.println(F("UNO ON"));
        setUnoOn(true);
      } else if (strcmp_P(cmd, PSTR("unoOff")) == 0) {
        appendSerialBuf(F("[UNOOFF]"));
        Serial.println("[UNOOFF]");
        Serial1.println(F("UNO OFF"));
        setUnoOn(false);
      }
      break;
      
    case 'd':
      if (strcmp_P(cmd, PSTR("dcOn")) == 0) {
        appendSerialBuf(F("[DCON]"));
        Serial1.println(F("DC ON"));
        setDcOn(true);
      } else if (strcmp_P(cmd, PSTR("dcOff")) == 0) {
        appendSerialBuf(F("[DCOFF]"));
        Serial1.println(F("DC OFF"));
        setDcOn(false);
      }
      break;
      
    case 's':
      if (strcmp_P(cmd, PSTR("setServos")) == 0) {
        handleSetServos(doc);
      } else if (strcmp_P(cmd, PSTR("setMRPM")) == 0) {
        handleSetMRPM(doc);
      } else if (strcmp_P(cmd, PSTR("setPID")) == 0) {
        handleSetPID(doc);
      }
      break;
  }
}

void handleGamepadInput(JsonDocument& doc) {
  float fx = doc[F("lx")].as<float>();
  float fy = doc[F("ly")].as<float>();
  float frx = doc[F("rx")].as<float>();
  float fry = doc[F("ry")].as<float>();
  float tl = doc[F("tl")].as<float>();
  float tr = doc[F("tr")].as<float>();

  // Apply deadzone
  if (fabs(fx) < JOY_L_THRESHOLD) fx = 0.0f;
  if (fabs(fy) < JOY_L_THRESHOLD) fy = 0.0f;
  
  // Only send if significant change
  if (fabs(fx - joyState.lastFX) > JOY_L_THRESHOLD || 
      fabs(fy - joyState.lastFY) > JOY_L_THRESHOLD) {
    char bufJL[32];
    snprintf_P(bufJL, sizeof(bufJL), PSTR("[JOY_L:%.2f,%.2f]"), fx, fy);
    Serial.println(bufJL);
    Serial1.println(bufJL);
    appendSerialBuf(bufJL);
    joyState.lastFX = fx;
    joyState.lastFY = fy;
  }
  
  // Update state
  state.joyLX_i = fx * 32767;
  state.joyLY_i = fy * 32767;
  state.joyRX_i = frx * 32767;
  state.joyRY_i = fry * 32767;
  state.trig6_i = constrain((int)(tl * 255), 0, 255);
  state.trig7_i = constrain((int)(tr * 255), 0, 255);
  
  updateDCFromTrigger();
}

void handleSetServos(JsonDocument& doc) {
  uint8_t a1 = doc[F("a1")];
  uint8_t a2 = doc[F("a2")];
  char buf[20];
  snprintf_P(buf, sizeof(buf), PSTR("[S,%u,%u]"), a1, a2);
  Serial.println(buf);
  Serial1.println(buf);
  appendSerialBuf(buf);
  state.servo1 = a1;
  state.servo2 = a2;
}

void handleSetMRPM(JsonDocument& doc) {
  int rpm = doc[F("rpm")];
  char buf[20];
  snprintf(buf, sizeof(buf), "[MRPM,%d]", rpm);
  Serial.println(buf);
  Serial1.println(buf);
  appendSerialBuf(buf);
}

void handleSetPID(JsonDocument& doc) {
  float sp = doc[F("sp")];
  float _kp = doc[F("kp")];
  float _ki = doc[F("ki")];
  float _kd = doc[F("kd")];
  
  state.setPoint_i = sp * 100;
  state.kp_i = _kp * 100;
  state.ki_i = _ki * 100;
  state.kd_i = _kd * 100;
  
  char buf[32];
  snprintf_P(buf, sizeof(buf), PSTR("[PIDT,%.2f,%.2f,%.2f,%.2f]"), sp, _kp, _ki, _kd);
  Serial.println(buf);
  Serial1.println(buf);
  appendSerialBuf(buf);
}

void setup() {
  pinMode(ledESP, OUTPUT);
  pinMode(audioPin, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200);
  
  // Optimize serial buffers
  Serial.setRxBufferSize(256);
  Serial1.setRxBufferSize(256);
  
  serialBuf.reserve(MAX_SERIAL_BUF);

  initAudio();
  memset(&state, 0, sizeof(state));
  
  // Initialize default values
  state.servo1 = state.servo2 = 90;
  state.motorTime = 1000;
  state.motorSel = 1;
  state.setPoint_i = 2500;
  state.kp_i = 100;
  state.ki_i = 2;
  state.kd_i = 50;
  state.currentRPM_i = 0;
  
  // Optimize WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  
  // More efficient connection wait
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 100) {
    delay(100);
    attempts++;
  }

  server.on("/", []() {
    server.send_P(200, PSTR("text/html"), INDEX_HTML);
  });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWsEvent);

  Serial.println(F("Ready"));
}

void loop() {
  server.handleClient();
  webSocket.loop();
  handleMelody();
  handleAudio();

  static uint32_t lastBroadcast = 0;
  uint32_t now = millis();
  
  if (now - lastBroadcast >= BROADCAST_INTERVAL) {
    broadcastState();
    lastBroadcast = now;
  }
  
  yield();
}