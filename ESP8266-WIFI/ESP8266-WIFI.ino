// ------------------ ESP8266-IFI.ino ------------------
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "index_html.h"

// ======= CONFIG =======
const char* ssid     = "Mov";
const char* password = "12345678";
const int audioPin   = 2;
const int ledESP     = 14;

// ======= ESTADO =======
struct State {
  bool espOn=false, unoOn=false;
  uint8_t servo1=90, servo2=90;
  int16_t motorVel=0; uint16_t motorTime=1000; uint8_t motorSel=1;
  float joyLX=0, joyLY=0, joyRX=0, joyRY=0, trig6=0, trig7=0;
  bool btn[16]={0};
} state;
String serialBuf;

// ======= SERVIDORES =======
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
MDNSResponder mdns;

// ======= MELODÍA ASÍNCRONA =======
const uint16_t melodyFreqs[] = {262,294,330,349,392,440,494,523};
const uint16_t melodyDur[]   = {300,300,300,300,300,300,300,600};
const uint8_t  melodyLen     = sizeof(melodyFreqs)/sizeof(melodyFreqs[0]);
bool playingMelody=false; uint8_t noteIndex=0; unsigned long noteEndTime=0;

void playMelodyAsync(){
  playingMelody=true; noteIndex=0; noteEndTime=millis();
}
void handleMelody(){
  if(!playingMelody) return;
  unsigned long now=millis();
  if(now<noteEndTime) return;
  if(noteIndex>=melodyLen){
    noTone(audioPin);
    playingMelody=false;
    return;
  }
  tone(audioPin, melodyFreqs[noteIndex], melodyDur[noteIndex]);
  noteEndTime = now + melodyDur[noteIndex] + 50;
  noteIndex++;
}

// ======= CAPTURA Serial1 =======
void handleSerial1(){
  while(Serial1.available()){
    serialBuf += (char)Serial1.read();
  }
}

// ======= BROADCAST ESTADO =======
void broadcastState(){
  handleSerial1();

  StaticJsonDocument<512> doc;
  doc["espOn"]=state.espOn;
  doc["unoOn"]=state.unoOn;
  doc["servo1"]=state.servo1;
  doc["servo2"]=state.servo2;
  doc["motorVel"]=state.motorVel;
  doc["motorTime"]=state.motorTime;
  doc["motorSel"]=state.motorSel;
  doc["joyLX"]=state.joyLX;
  doc["joyLY"]=state.joyLY;
  doc["joyRX"]=state.joyRX;
  doc["joyRY"]=state.joyRY;
  doc["trig6"]=state.trig6;
  doc["trig7"]=state.trig7;

  // No agregamos botones al buffer

  doc["serial"] = serialBuf;
  String out; serializeJson(doc, out);
  webSocket.broadcastTXT(out);
  serialBuf="";
}

// ======= WS CALLBACK =======
void onWsEvent(uint8_t num, WStype_t type, uint8_t* pl, size_t len){
  if(type!=WStype_TEXT) return;
  StaticJsonDocument<512> doc;
  if(deserializeJson(doc, pl, len)) return;
  String cmd = doc["cmd"].as<String>();

  if(cmd=="ping"){
    webSocket.sendTXT(num, "{\"cmd\":\"pong\"}");
    return;
  }
  if(cmd=="gamepad"){
    state.joyLX=doc["lx"];
    state.joyLY=doc["ly"];
    state.joyRX=doc["rx"];
    state.joyRY=doc["ry"];
    state.trig6=constrain(doc["t6"].as<float>(),0,1);
    state.trig7=constrain(doc["t7"].as<float>(),0,1);
    JsonArray arrIn = doc["btn"].as<JsonArray>();
    for(int i=0;i<16;i++) state.btn[i] = arrIn[i];
    broadcastState();
    return;
  }

  // COMANDOS: solo prints explícitos
  if(cmd=="espOn"){
    Serial.println("[ESPON]"); serialBuf += "[ESPON]";
    Serial1.println("ESP ON");
    digitalWrite(ledESP,HIGH); state.espOn=true;
  }
  else if(cmd=="espOff"){
    Serial.println("[ESPOFF]"); serialBuf += "[ESPOFF]";
    Serial1.println("ESP OFF");
    digitalWrite(ledESP,LOW); state.espOn=false;
  }
  else if(cmd=="unoOn"){
    Serial.println("[UNOON]"); serialBuf += "[UNOON]";
    Serial1.println("UNO ON");
    state.unoOn=true;
  }
  else if(cmd=="unoOff"){
    Serial.println("[UNOOFF]"); serialBuf += "[UNOOFF]";
    Serial1.println("UNO OFF");
    state.unoOn=false;
  }
  else if(cmd=="playMelody"){
    Serial.println("[MELODY]"); serialBuf += "[MELODY]";
    Serial1.println("PLAY MELODY");
    playMelodyAsync();
  }
  else if(cmd=="setServos"){
    uint8_t a1=doc["a1"], a2=doc["a2"];
    char buf[32]; snprintf(buf,sizeof(buf),"[SERVOS,%u,%u]",a1,a2);
    Serial.println(buf); serialBuf += buf;
    Serial1.println(buf);
    state.servo1=a1; state.servo2=a2;
  }
  else if(cmd=="setMotor"){
    int vel=doc["vel"], time=doc["time"], sel=doc["sel"];
    char buf[32]; snprintf(buf,sizeof(buf),"[MOTOR,%d,%d,%d]",vel,time,sel);
    Serial.println(buf); serialBuf += buf;
    Serial1.println(buf);
    state.motorVel=vel; state.motorTime=time; state.motorSel=sel;
  }

  broadcastState();
}

void setup(){
  pinMode(ledESP,OUTPUT); digitalWrite(ledESP,LOW);
  pinMode(audioPin,OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200);

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){ delay(250); Serial.print("."); }
  Serial.println(); Serial.print("IP: "); Serial.println(WiFi.localIP());

  mdns.begin("esp8266", WiFi.localIP());

  server.on("/", [](){ server.send_P(200, "text/html", INDEX_HTML); });
  server.begin(); Serial.println("HTTP server started");

  webSocket.begin(); webSocket.onEvent(onWsEvent);
  Serial.println("WebSocket server started");
}

void loop(){
  server.handleClient();
  webSocket.loop();
  handleMelody();
}
