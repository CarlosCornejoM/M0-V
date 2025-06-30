// ------------------ ESP8266-IFI.ino ------------------
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
#include "audio.h"  // WAV sample in PROGMEM

// ======= CONFIG =======
const char* ssid     = "Mov";
const char* password = "12345678";
const int audioPin   = 3;
const int ledESP     = 14;

// ======= STATE =======
struct State {
  bool espOn = false;
  bool unoOn = false;
  bool dcOn  = false;
  uint8_t servo1 = 90, servo2 = 90;
  int16_t motorVel = 0;
  uint16_t motorTime = 1000;
  uint8_t motorSel = 1;
  uint8_t dcValue = 0;  // Nuevo: valor DC de 0-1
  float joyLX = 0, joyLY = 0, joyRX = 0, joyRY = 0;
  float trig6 = 0, trig7 = 0;
  bool btn[16] = {0};
} state;
String serialBuf;

// ======= SERVERS =======
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
MDNSResponder mdns;

// ======= ASYNC MELODY =======
const uint16_t melodyFreqs[] = {262,294,330,349,392,440,494,523};
const uint16_t melodyDur[]   = {300,300,300,300,300,300,300,600};
const uint8_t  melodyLen     = sizeof(melodyFreqs)/sizeof(melodyFreqs[0]);
bool playingMelody = false;
uint8_t noteIndex = 0;
unsigned long noteEndTime = 0;

// ======= DC CONTROL VARIABLES =======
unsigned long lastDCSent = 0;
const unsigned long DC_SEND_INTERVAL = 50; // Enviar cada 50ms para evitar spam
uint8_t lastDCValue = 0; // Para evitar envios redundantes

void playMelodyAsync() {
  playingMelody = true;
  noteIndex = 0;
  noteEndTime = millis();
}

void handleMelody() {
  if (!playingMelody) return;
  unsigned long now = millis();
  if (now < noteEndTime) return;
  if (noteIndex >= melodyLen) {
    noTone(audioPin);
    playingMelody = false;
    return;
  }
  tone(audioPin, melodyFreqs[noteIndex], melodyDur[noteIndex]);
  noteEndTime = now + melodyDur[noteIndex] + 50;
  noteIndex++;
}

// ======= ASYNC WAV PLAYBACK =======
AudioGeneratorWAV *wavPlayer;
AudioFileSourcePROGMEM *wavFile;
AudioOutputI2SNoDAC *wavOut;
bool playingAudio = false;

void initAudio() {
  audioLogger = &Serial;
  wavOut    = new AudioOutputI2SNoDAC();
  wavPlayer = new AudioGeneratorWAV();
}

void playAudio() {
  if (playingAudio) return;
  if (wavFile) delete wavFile;
  wavFile = new AudioFileSourcePROGMEM(audio, sizeof(audio));
  wavPlayer->begin(wavFile, wavOut);
  playingAudio = true;
  Serial.println("[PLAYAUDIO]"); serialBuf += "[PLAYAUDIO]";
}

void handleAudio() {
  if (!playingAudio) return;
  if (wavPlayer->isRunning()) {
    if (!wavPlayer->loop()) {
      wavPlayer->stop();
      playingAudio = false;
    }
  }
}

// ======= DC CONTROL FROM TRIGGER =======
void updateDCFromTrigger() {
  unsigned long now = millis();
  if (now - lastDCSent < DC_SEND_INTERVAL) return;
  
  // Convertir trigger R2 (0.0-1.0)
  float dcValue = constrain(state.trig7, 0.0f, 1.0f);
  const float DC_DELTA = 0.02f;
  static float lastDCValueF = 0.0f;
  if (fabs(dcValue - lastDCValueF) > DC_DELTA) {
    char buf[16];
    // Formato con dos decimales: [DC,0.75]
    snprintf(buf, sizeof(buf), "[DC,%.2f]", dcValue);

    Serial.println(buf);
    serialBuf += String(buf);
    Serial1.println(buf);

    // Actualiza estado y umbral
    state.dcOn      = (dcValue > 0.0f);
    state.dcValue   = dcValue;      // cambia dcValue en struct a float
    lastDCValueF    = dcValue;
    lastDCSent      = now;
  }
}

// ======= SERIAL1 HANDLER =======
void handleSerial1() {
  while (Serial1.available()) {
    serialBuf += (char)Serial1.read();
  }
}

// ======= BROADCAST STATE =======
void broadcastState() {
  handleSerial1();
  StaticJsonDocument<300> doc;
  doc["espOn"]     = state.espOn;
  doc["unoOn"]     = state.unoOn;
  doc["dcOn"]      = state.dcOn;
  doc["servo1"]    = state.servo1;
  doc["servo2"]    = state.servo2;
  doc["motorVel"]  = state.motorVel;
  doc["motorTime"] = state.motorTime;
  doc["motorSel"]  = state.motorSel;
  doc["dcValue"]   = state.dcValue;  // Nuevo: valor DC actual
  doc["joyLX"]     = state.joyLX;
  doc["joyLY"]     = state.joyLY;
  doc["joyRX"]     = state.joyRX;
  doc["joyRY"]     = state.joyRY;
  doc["trig6"]     = state.trig6;
  doc["trig7"]     = state.trig7;
  doc["serial"]    = serialBuf;

  // Serializar JSON y enviarlo
  String out;
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);

  // Luego enviar el texto puro de la consola serial
  if (serialBuf.length() > 0) {
    webSocket.broadcastTXT(serialBuf);
  }

  // Limpiar buffer para la siguiente iteración
  serialBuf = "";
}

// ======= WEBSOCKET CALLBACK =======
void onWsEvent(uint8_t num, WStype_t type, uint8_t* pl, size_t len) {
  if (type != WStype_TEXT) return;
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, pl, len)) return;
  String cmd = doc["cmd"].as<String>();

  if (cmd == "ping") {
    webSocket.sendTXT(num, "{\"cmd\":\"pong\"}");
    return;
  }
  if (cmd == "gamepad") {
    state.joyLX = doc["lx"];
    state.joyLY = doc["ly"];
    state.joyRX = doc["rx"];
    state.joyRY = doc["ry"];
    state.trig6 = constrain(doc["tl"].as<float>(), 0, 1);  // L2 trigger
    state.trig7 = constrain(doc["tr"].as<float>(), 0, 1);  // R2 trigger
    
    // Actualizar DC automáticamente desde el trigger R2
    updateDCFromTrigger();
  }

  // COMANDOS DE CONTROL
  if (cmd == "espOn") {
    Serial.println("[ESPON]"); serialBuf += "[ESPON]";
    Serial1.println("ESP ON");
    digitalWrite(ledESP, HIGH);
    state.espOn = true;
  }
  else if (cmd == "espOff") {
    Serial.println("[ESPOFF]"); serialBuf += "[ESPOFF]";
    Serial1.println("ESP OFF");
    digitalWrite(ledESP, LOW);
    state.espOn = false;
  }
  else if (cmd == "unoOn") {
    Serial.println("[UNOON]"); serialBuf += "[UNOON]";
    Serial1.println("UNO ON");
    state.unoOn = true;
  }
  else if (cmd == "unoOff") {
    Serial.println("[UNOOFF]"); serialBuf += "[UNOOFF]";
    Serial1.println("UNO OFF");
    state.unoOn = false;
  }
  else if (cmd == "dcOn") {
    Serial.println("[DCON]"); serialBuf += "[DCON]";
    Serial1.println("DC ON");
    state.dcOn = true;
  }
  else if (cmd == "dcOff") {
    Serial.println("[DCOFF]"); serialBuf += "[DCOFF]";
    Serial1.println("DC OFF");
    state.dcOn = false;
  }
  
  // AUDIO
  else if (cmd == "playMelody") {
    Serial.println("[MELODY]"); serialBuf += "[MELODY]";
    Serial1.println("PLAY MELODY");
    playMelodyAsync();
  }
  else if (cmd == "playAudio") {
    Serial.println("[AUDIO]"); serialBuf += "[AUDIO]";
    Serial1.println("PLAY AUDIO");
    playAudio();
  }
  // SERVOS
  else if (cmd == "setServos") {
    uint8_t a1 = doc["a1"], a2 = doc["a2"];
    char buf[32]; snprintf(buf, sizeof(buf), "[S,%u,%u]", a1, a2);
    Serial.println(buf); serialBuf += buf;
    Serial1.println(buf);
    state.servo1 = a1;
    state.servo2 = a2;
  }
  // MOTOR
  else if (cmd == "setMotor") {
    int vel  = doc["vel"], time = doc["time"], sel = doc["sel"];
    char buf[32]; snprintf(buf, sizeof(buf), "[M,%d,%d,%d]", vel, time, sel);
    Serial.println(buf); serialBuf += buf;
    Serial1.println(buf);
    state.motorVel  = vel;
    state.motorTime = time;
    state.motorSel  = sel;
  }

  broadcastState();
}

void setup() {
  pinMode(ledESP, OUTPUT);

  // Parpadeo de arranque
  for (int i = 0; i < 6; i++) {
    digitalWrite(ledESP, i % 2);
    delay(100);
  }

  pinMode(audioPin, OUTPUT);

  Serial.begin(115200);
  Serial1.begin(115200);

  initAudio();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  mdns.begin("esp8266", WiFi.localIP());

  server.on("/", []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });
  server.begin();
  Serial.println("HTTP server started");

  webSocket.begin();
  webSocket.onEvent(onWsEvent);
  Serial.println("WebSocket server started");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  handleMelody();
  handleAudio();
}