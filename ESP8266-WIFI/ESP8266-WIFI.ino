// ------------------ ESP8266‑IFI‑Optimized.ino ------------------

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
} state;

// ======= FLAGS =======
inline bool getEspOn()       { return state.flags & 0x01; }
inline bool getUnoOn()       { return state.flags & 0x02; }
inline bool getDcOn()        { return state.flags & 0x04; }
inline bool isPlayingMelody(){ return state.flags & 0x08; }
inline bool isPlayingAudio(){ return state.flags & 0x10; }
inline void setEspOn(bool v)     { state.flags = (state.flags & ~0x01) | (v ? 0x01 : 0); }
inline void setUnoOn(bool v)     { state.flags = (state.flags & ~0x02) | (v ? 0x02 : 0); }
inline void setDcOn(bool v)      { state.flags = (state.flags & ~0x04) | (v ? 0x04 : 0); }
inline void setPlayingMelody(bool v){ state.flags = (state.flags & ~0x08) | (v ? 0x08 : 0); }
inline void setPlayingAudio(bool v) { state.flags = (state.flags & ~0x10) | (v ? 0x10 : 0); }

// ======= GLOBALS =======
String serialBuf;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

// Para JOY_L “significativo”
float lastFX = 0.0f;
float lastFY = 0.0f;
const float JOY_L_THRESHOLD = 0.05f;  // 5%

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
  if (wavFile) delete wavFile;
  wavFile = new AudioFileSourcePROGMEM(audio, sizeof(audio));
  wavPlayer->begin(wavFile, wavOut);
  setPlayingAudio(true);
  serialBuf += F("[PLAYAUDIO]");
}
void handleAudio() {
  if (!isPlayingAudio()) return;
  if (!wavPlayer->loop()) {
    wavPlayer->stop();
    setPlayingAudio(false);
  }
}

// ======= MELODY =======
const uint16_t melodyData[] PROGMEM = {
  262,300, 294,300, 330,300, 349,300, 392,300, 440,300, 494,300, 523,600
};
const uint8_t melodyLen = 8;
uint8_t  noteIndex   = 0;
uint32_t noteEndTime = 0;
void playMelodyAsync() {
  setPlayingMelody(true);
  noteIndex   = 0;
  noteEndTime = millis();
}
void handleMelody() {
  if (!isPlayingMelody()) return;
  uint32_t now = millis();
  if (now < noteEndTime) return;
  if (noteIndex >= melodyLen) {
    noTone(audioPin);
    setPlayingMelody(false);
    return;
  }
  uint16_t freq = pgm_read_word(&melodyData[noteIndex*2]);
  uint16_t dur  = pgm_read_word(&melodyData[noteIndex*2+1]);
  tone(audioPin, freq, dur);
  noteEndTime = now + dur + 50;
  noteIndex++;
}

// ======= DC CONTROL =======
uint32_t lastDCSent = 0;
const uint32_t DC_SEND_INTERVAL = 50;
uint8_t lastDCValue = 0;
void updateDCFromTrigger() {
  uint32_t now = millis();
  if (now - lastDCSent < DC_SEND_INTERVAL) return;
  uint8_t dcVal = state.trig7_i;
  if (abs(dcVal - lastDCValue) > 5) {
    char buf[16];
    float df = dcVal / 255.0f;
    snprintf_P(buf, sizeof(buf), PSTR("[DC,%.2f]"), df);
    Serial.println(buf);
    serialBuf += buf;
    Serial1.println(buf);
    setDcOn(dcVal > 0);
    state.dcValue = dcVal;
    lastDCValue   = dcVal;
    lastDCSent    = now;
  }
}

// ======= SERIAL INPUT =======
void handleSerial() {
  static String lineBuf;
  while (Serial.available()) {
    char c = Serial.read();
    if (c=='\n' || c=='\r') {
      if (lineBuf.length()>0) {
        char t = lineBuf.charAt(1);
        if (t=='A') {
          if (lineBuf.startsWith("[ANGLES,")) {
            float p,r,y; sscanf(lineBuf.c_str()+8,"%f,%f,%f",&p,&r,&y);
            state.pitch_i = p*100; state.roll_i = r*100; state.yaw_i = y*100;
          } else if (lineBuf.startsWith("[ACCEL,")) {
            float ax,ay,az; sscanf(lineBuf.c_str()+7,"%f,%f,%f",&ax,&ay,&az);
            state.accelX_i = ax*1000; state.accelY_i = ay*1000; state.accelZ_i = az*1000;
          }
        } else if (t=='T') {
          float T; sscanf(lineBuf.c_str()+6,"%f",&T);
          state.temperature_i = T*100;
        } else if (t=='U') {
          float d; sscanf(lineBuf.c_str()+4,"%f",&d);
          state.ultrasonic_i = d*100;
        } else if (t=='P') {
          float e,o; int a;
          sscanf(lineBuf.c_str()+5,"%f,%f,%d",&e,&o,&a);
          state.pidError_i  = e*100;
          state.pidOutput_i = o*100;
          state.pidAngle    = a;
        }
        lineBuf = "";
      }
    } else {
      lineBuf += c;
    }
  }
}

// ======= BROADCAST STATE =======
void broadcastState() {
  handleSerial();
  StaticJsonDocument<256> doc;
  doc[F("espOn")]     = getEspOn();
  doc[F("unoOn")]     = getUnoOn();
  doc[F("dcOn")]      = getDcOn();
  doc[F("servo1")]    = state.servo1;
  doc[F("servo2")]    = state.servo2;
  doc[F("motorVel")]  = state.motorVel;
  doc[F("motorTime")] = state.motorTime;
  doc[F("motorSel")]  = state.motorSel;
  doc[F("dcValue")]   = state.dcValue;
  doc[F("joyLX")]     = state.joyLX_i / 32767.0f;
  doc[F("joyLY")]     = state.joyLY_i / 32767.0f;
  doc[F("joyRX")]     = state.joyRX_i / 32767.0f;
  doc[F("joyRY")]     = state.joyRY_i / 32767.0f;
  doc[F("trig6")]     = state.trig6_i / 255.0f;
  doc[F("trig7")]     = state.trig7_i / 255.0f;
  doc[F("ultrasonic")] = state.ultrasonic_i/100.0f;
  doc[F("pitch")]      = state.pitch_i/100.0f;
  doc[F("roll")]       = state.roll_i/100.0f;
  doc[F("yaw")]        = state.yaw_i/100.0f;
  doc[F("accelX")]     = state.accelX_i/1000.0f;
  doc[F("accelY")]     = state.accelY_i/1000.0f;
  doc[F("accelZ")]     = state.accelZ_i/1000.0f;
  doc[F("temperature")] = state.temperature_i/100.0f;
  doc[F("pidError")]   = state.pidError_i/100.0f;
  doc[F("pidOutput")]  = state.pidOutput_i/100.0f;
  doc[F("pidAngle")]   = state.pidAngle;
  doc[F("kp")]         = state.kp_i/100.0f;
  doc[F("ki")]         = state.ki_i/100.0f;
  doc[F("kd")]         = state.kd_i/100.0f;
  if (serialBuf.length()>0) doc[F("serial")] = serialBuf;
  String out; out.reserve(512);
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);
  serialBuf = "";
}

// ======= WEBSOCKET CALLBACK =======
void onWsEvent(uint8_t num, WStype_t type, uint8_t* pl, size_t len) {
  if (type != WStype_TEXT) return;
  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, pl, len)) return;
  const char* cmd = doc[F("cmd")];
  if (!cmd) return;

  // ping
  if (strcmp_P(cmd,PSTR("ping"))==0) {
    webSocket.sendTXT(num, "{\"cmd\":\"pong\"}");
  }
  // gamepad
  else if (strcmp_P(cmd,PSTR("gamepad"))==0) {
    // 1) ejes
    float fx = doc[F("lx")].as<float>();
    float fy = doc[F("ly")].as<float>();
    float frx = doc[F("rx")].as<float>();
    float fry = doc[F("ry")].as<float>();
    float tl  = doc[F("tl")].as<float>();
    float tr  = doc[F("tr")].as<float>();

    // 2) JOY_L sólo cambios significativos
    if (fabs(fx) < JOY_L_THRESHOLD) fx = 0.0f;
    if (fabs(fy) < JOY_L_THRESHOLD) fy = 0.0f;
    if (fabs(fx - lastFX) > JOY_L_THRESHOLD || fabs(fy - lastFY) > JOY_L_THRESHOLD) {
      char bufJL[32];
      snprintf_P(bufJL, sizeof(bufJL), PSTR("[JOY_L:%.2f,%.2f]"), fx, fy);
      Serial.println(bufJL);
      Serial1.println(bufJL);
      serialBuf += bufJL;
      lastFX = fx;
      lastFY = fy;
    }
    // actualiza estado sticks
    state.joyLX_i = fx * 32767;
    state.joyLY_i = fy * 32767;
    state.joyRX_i = frx * 32767;
    state.joyRY_i = fry * 32767;

    // 3) triggers + DC
    state.trig6_i = constrain((int)(tl*255), 0,255);
    state.trig7_i = constrain((int)(tr*255), 0,255);
    updateDCFromTrigger();
  }
  // espOn / espOff
  else if (strcmp_P(cmd,PSTR("espOn"))==0) {
    serialBuf += F("[ESPON]");
    Serial1.println(F("ESP ON"));
    digitalWrite(ledESP, HIGH);
    setEspOn(true);
  }
  else if (strcmp_P(cmd,PSTR("espOff"))==0) {
    serialBuf += F("[ESPOFF]");
    Serial1.println(F("ESP OFF"));
    digitalWrite(ledESP, LOW);
    setEspOn(false);
  }
  // unoOn / unoOff
  else if (strcmp_P(cmd,PSTR("unoOn"))==0) {
    serialBuf += F("[UNOON]");
    Serial.println("[UNOON]");
    Serial1.println(F("UNO ON"));
    setUnoOn(true);
  }
  else if (strcmp_P(cmd,PSTR("unoOff"))==0) {
    serialBuf += F("[UNOOFF]");
    Serial.println("[UNOOFF]");
    Serial1.println(F("UNO OFF"));
    setUnoOn(false);
  }
  // dcOn / dcOff (botones manuales)
  else if (strcmp_P(cmd,PSTR("dcOn"))==0) {
    serialBuf += F("[DCON]");
    Serial1.println(F("DC ON"));
    setDcOn(true);
  }
  else if (strcmp_P(cmd,PSTR("dcOff"))==0) {
    serialBuf += F("[DCOFF]");
    Serial1.println(F("DC OFF"));
    setDcOn(false);
  }
  // playMelody / playAudio
  else if (strcmp_P(cmd,PSTR("playMelody"))==0) {
    serialBuf += F("[MELODY]");
    playMelodyAsync();
  }
  else if (strcmp_P(cmd,PSTR("playAudio"))==0) {
    serialBuf += F("[AUDIO]");
    playAudio();
  }
  // setServos
  else if (strcmp_P(cmd,PSTR("setServos"))==0) {
    uint8_t a1 = doc[F("a1")], a2 = doc[F("a2")];
    char buf[20];
    snprintf_P(buf,sizeof(buf),PSTR("[S,%u,%u]"),a1,a2);
    Serial.println(buf);
    Serial1.println(buf);
    serialBuf += buf;
    state.servo1 = a1; state.servo2 = a2;
  }
  // setMotor
  else if (strcmp_P(cmd,PSTR("setMotor"))==0) {
    int vel = doc[F("vel")], tm = doc[F("time")], sel = doc[F("sel")];
    char buf[24];
    snprintf_P(buf,sizeof(buf),PSTR("[M,%d,%d,%d]"),vel,tm,sel);
    Serial.println(buf);
    Serial1.println(buf);
    serialBuf += buf;
    state.motorVel  = vel;
    state.motorTime = tm;
    state.motorSel  = sel;
  }
  // setPID
  else if (strcmp_P(cmd,PSTR("setPID"))==0) {
    float sp = doc[F("sp")], _kp = doc[F("kp")],
          _ki = doc[F("ki")], _kd = doc[F("kd")];
    state.setPoint_i = sp*100;
    state.kp_i       = _kp*100;
    state.ki_i       = _ki*100;
    state.kd_i       = _kd*100;
    char buf[32];
    snprintf_P(buf,sizeof(buf),
      PSTR("[PIDT,%.2f,%.2f,%.2f,%.2f]"),
      sp,_kp,_ki,_kd
    );
    Serial.println(buf);
    Serial1.println(buf);
    serialBuf += buf;
  }
}

void setup() {
  pinMode(ledESP, OUTPUT);
  pinMode(audioPin, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200);
  serialBuf.reserve(128);

  initAudio();
  memset(&state,0,sizeof(state));
  state.servo1 = state.servo2 = 90;
  state.motorTime = 1000;
  state.motorSel  = 1;
  state.setPoint_i=2500; state.kp_i=100; state.ki_i=2; state.kd_i=50;

  WiFi.begin(ssid,password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  while (WiFi.status()!=WL_CONNECTED) {
    delay(100);
  }

  server.on("/", [](){
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
  if (now - lastBroadcast >= 5) {
    handleSerial();
    updateDCFromTrigger();
    broadcastState();
    lastBroadcast = now;
  }
  yield();
}
