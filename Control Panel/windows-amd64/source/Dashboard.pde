import processing.serial.*;
import controlP5.*;
import java.util.regex.*;
import java.util.Arrays;

Serial port;
ControlP5 cp5;

ArrayList<String> irCodes = new ArrayList<>();
final int MAX_IR_CODES = 5;    // cuántos últimos verás en pantalla

// Datos IMU
float pitch, roll, yaw, temp;
int ax, ay, az, gx, gy, gz;
float ax_g, ay_g, az_g;

// Historial
final int MAX_HISTORY = 200;
ArrayList<Integer> histAx = new ArrayList<>();
ArrayList<Integer> histAy = new ArrayList<>();
ArrayList<Integer> histAz = new ArrayList<>();
ArrayList<Float>   histAxG = new ArrayList<>();
ArrayList<Float>   histAyG = new ArrayList<>();
ArrayList<Float>   histAzG = new ArrayList<>();

// UI
boolean showX = true, showY = false, showZ = false;
boolean useRaw = true;

// Fuentes y colores
PFont fontH1, fontH2, fontText;
color pitchColor, rollColor, yawColor;

// Calibración inicial
boolean calibrating = true;
int calStart, calDur = 9000;

// Servos/Motores
int servo1Angle = 90, servo2Angle = 90;
int motorSelect = 0, motorVelocity = 0, motorTime = 1000;
int lastMotorSend = 0;

void setup() {
  size(1280,720,P3D);
  surface.setLocation(100, 100);  // mueve la ventana a x=100, y=100 en pantalla
  smooth(8);
  frameRate(144);

  // Colores
  pitchColor = color(255,128,0);
  rollColor  = color(128,0,255);
  yawColor   = color(0,255,255);

  // Fuentes
  fontH1   = createFont("Arial Bold",28);
  fontH2   = createFont("Arial",20);
  fontText = createFont("Arial",14);

  cp5 = new ControlP5(this);

  // Dropdown COM al lado del título
  String[] ports = Serial.list();
  cp5.addDropdownList("serialPort")
     .setLabel("COM")
     .setPosition(300, 31)
     .setSize(160, ports.length*20)
     .setItemHeight(20)
     .setBarHeight(20)
     .setFont(fontText)
     .addItems(Arrays.asList(ports))
     .setValue(0);
  if(ports.length>0) openSerialPort(ports[0]);

  // Toggles X/Y/Z/Raw bajados para no solapar Gyro
  int togY = 80 + 240 + 20;
  cp5.addToggle("showX").setLabel("X")
     .setPosition(40, togY).setSize(30,15).setValue(true);
  cp5.addToggle("showY").setLabel("Y")
     .setPosition(80, togY).setSize(30,15);
  cp5.addToggle("showZ").setLabel("Z")
     .setPosition(120, togY).setSize(30,15);
  cp5.addToggle("useRaw").setLabel("Raw")
     .setPosition(160, togY).setSize(45,15).setValue(true);

  // Botón CALIBRAR al lado derecho de Telemetría
  float telX = 40 + (width/2 - 80);
  cp5.addButton("calibrar")
     .setLabel("CALIBRAR")
     .setPosition(telX+10, 80)
     .setSize(80,30)
     .setFont(fontText);

  // Sliders Servos y Motores
  setupServosMotors();

  // Iniciar conteo de calibración
  calStart = millis();
}

void draw() {
  background(18);

  // Durante la calibración, solo muestro la barra
  if (calibrating) {
    drawCalibrationBarFull();
    return;
  }

  // Una vez calibrado, muestro toda la UI
  drawHeader();
  drawTelemetryPanel();
  drawIRPanel();
  drawCubePanel();
  drawGraphSection();
  drawServosMotorsPanel();
}

void drawHeader() {
  textFont(fontH1);
  fill(220);
  textAlign(LEFT, CENTER);
  text("M0-V Dashboard", 40, 40);
}

void drawTelemetryPanel() {
  float x=40, y=80, w=width/2-80, h=240, p=16;
  noStroke(); fill(40,200);
  rect(x,y,w,h,8);

  textFont(fontH2); fill(200);
  text("Telemetría", x+p, y+p);

  textFont(fontText);
  float ty=y+p+32;
  fill(235); text("Pitch:", x+p,ty);    fill(pitchColor); text(nf(pitch,1,2)+"°", x+p+60, ty);
  fill(235); text("Roll:",  x+p,ty+24); fill(rollColor);  text(nf(roll,1,2)+"°", x+p+60, ty+24);
  fill(235); text("Yaw:",   x+p,ty+48); fill(yawColor);   text(nf(yaw,1,2)+"°", x+p+60, ty+48);
  fill(235); text("Temp: "+nf(temp,1,2)+" °C", x+p, ty+72);

  fill(235); text("Acel (raw):", x+p, ty+100);
  fill(#FF0000); text("X:"+ax, x+p+100,ty+100);
  fill(#00FF00); text("Y:"+ay, x+p+160,ty+100);
  fill(#0000FF); text("Z:"+az, x+p+220,ty+100);

  fill(235); text("Acel (g):", x+p, ty+124);
  fill(#FF0000); text("X:"+nf(ax_g,1,3)+"g", x+p+100,ty+124);
  fill(#00FF00); text("Y:"+nf(ay_g,1,3)+"g", x+p+160,ty+124);
  fill(#0000FF); text("Z:"+nf(az_g,1,3)+"g", x+p+220,ty+124);

  fill(235);
  text("Gyro → X:"+gx+"  Y:"+gy+"  Z:"+gz, x+p, ty+156);
}

void drawIRPanel() {

  // Fondo del panel
  noStroke();
  fill(40, 200);
  rect(480, 30, width/2 - 80, 20 + MAX_IR_CODES * 16, 8);

  // Título
  textFont(fontH2);
  fill(200);
  textAlign(LEFT, CENTER);
  text("IR Received (hex)", 480+10, 30+12);

  // Lista de códigos
  textFont(fontText);
  fill(180);
  for (int i = 0; i < irCodes.size(); i++) {
    text(irCodes.get(i), 480+10, 30 + 30 + i*16);
  }
}

float sensorScale = 50;  // 50 px = 1 mm
void drawCubePanel() {
  // Área superior derecha para el paralelepípedo 3D
  float x = width/2 + 20;
  float y = 80;
  float w = 300;
  float h = 200;

  // Dimensiones reales del MPU‑6050 en mm
  float sx_mm = 4.0;
  float sy_mm = 4.0;
  float sz_mm = 0.9;
  // Convertir a píxeles
  float sx = sx_mm * sensorScale;
  float sy = sy_mm * sensorScale;
  float sz = sz_mm * sensorScale;

  pushMatrix();
    // Coloca el origen en el centro del panel
    translate(x + w/2, y + h/2, 0);

    // Orientación según tus ángulos (en radianes)
    rotateX(radians(pitch));
    rotateY(radians(roll));
    rotateZ(radians(yaw));

    // Dibuja un paralelepípedo (wireframe)
    noFill();
    stroke(200);
    strokeWeight(2);
    box(sx, sy, sz);
  popMatrix();
}


void drawGraphSection() {
  float x=40, y=380, w=width/2-80, h=180;
  noStroke(); fill(40,200);
  rect(x,y,w,h,8);

  textFont(fontH2); fill(200);
  textAlign(CENTER);
  text("Gráfico de Aceleración", x+w/2, y-10);

  stroke(120);
  line(x, y+h/2, x+w, y+h/2);
  line(x, y, x, y+h);

  noStroke(); fill(200); textFont(fontText);
  textAlign(CENTER);
  text("Tiempo", x+w/2, y+h+25);

  pushMatrix();
    translate(x-35, y+h/2);
    rotate(-HALF_PI);
    String unit = useRaw ? "LSB" : "g";
    textAlign(CENTER);
    text("Valor ("+unit+")", 0, 0);
  popMatrix();

  float m = useRaw ? 16384 : 1.0;
  if(showX) drawCurve(useRaw?histAx:histAxG, #FF0000, x,y,w,h,m);
  if(showY) drawCurve(useRaw?histAy:histAyG, #00FF00, x,y,w,h,m);
  if(showZ) drawCurve(useRaw?histAz:histAzG, #0000FF, x,y,w,h,m);
}

void drawServosMotorsPanel(){
  float x=width/2+10;
  float y=height/2 - 50;
  float w=width/2-40;
  float h=height - y - 20;
  noStroke(); fill(40,200);
  rect(x,y,w,h,8);

  textFont(fontH2); fill(200);
  textAlign(LEFT, CENTER);
  text("Control de Servos y Motores", x+20, y+30);
}

void drawCalibrationBarFull(){
  // fondo semitransparente
  fill(0, 180);
  rect(0, 0, width, height);
  // barra centrada
  float bw = width * 0.6f;
  float bh = 30;
  float bx = (width - bw)/2;
  float by = (height - bh)/2;
  float t = constrain((millis()-calStart)/(float)calDur, 0, 1);
  noStroke(); fill(60);
  rect(bx, by, bw, bh, 4);
  fill(0,150,250);
  rect(bx, by, bw*t, bh, 4);
  fill(230); textFont(fontText);
  textAlign(CENTER, CENTER);
  text("Calibrando giroscopio... " + int(t*100) + "%", width/2, by + bh/2);
  if (t >= 1) calibrating = false;
}

void drawCurve(ArrayList<? extends Number> data, color c,
               float x,float y,float w,float h,float maxV) {
  if(data.size()<2) return;
  stroke(c); noFill(); beginShape();
  for(int i=0; i<data.size(); i++){
    float v = data.get(i).floatValue();
    float xp = map(i,0,data.size()-1, x, x+w);
    float yp = map(v, -maxV, maxV, y+h, y);
    vertex(xp,yp);
  }
  endShape();
}

void setupServosMotors(){
  int baseX = width/2 + 20;
  int y1 = height/2 + 40;
  cp5.addSlider("servo1Angle")
     .setLabel("Servo 1")
     .setRange(0,180).setValue(90)
     .setPosition(baseX, y1)
     .setSize(250,15)
     .setFont(fontText);

  cp5.addSlider("servo2Angle")
     .setLabel("Servo 2")
     .setRange(0,180).setValue(90)
     .setPosition(baseX, y1+40)
     .setSize(250,15)
     .setFont(fontText);

  cp5.addDropdownList("motorSelect")
     .setLabel("Motor")
     .addItem("Motor 1", 0)
     .addItem("Motor 2", 1)
     .addItem("Ambos",   2)
     .setPosition(baseX, y1+80)
     .setSize(160,60)
     .setItemHeight(18)
     .setBarHeight(18)
     .setFont(fontText);

  cp5.addSlider("motorVelocity")
     .setLabel("Potencia")
     .setRange(-255,255).setValue(0)
     .setPosition(baseX, y1+150)
     .setSize(250,15)
     .setFont(fontText);

  cp5.addSlider("motorTime")
     .setLabel("Tiempo (ms)")
     .setRange(0,5000).setValue(1000)
     .setPosition(baseX, y1+190)
     .setSize(250,15)
     .setFont(fontText);
}

// ------------------------------------------
// Callbacks Serial y ControlP5
// ------------------------------------------

void serialEvent(Serial p){
  String line = p.readStringUntil('\n');
  if(line == null) return;
  line = line.trim();

  // Si viene un código IR, lo guardamos aparte
  if (line.startsWith("IR raw: 0x")) {
    irCodes.add(line.substring(11));
    if (irCodes.size() > MAX_IR_CODES) {
      irCodes.remove(0);
    }
  }
  
  ArrayList<String> nums = new ArrayList<>();
  Matcher m = Pattern.compile("[-]?\\d+\\.?\\d*").matcher(line);
  while(m.find()) nums.add(m.group());
  if(line.startsWith("Pitch:")){
    pitch=Float.parseFloat(nums.get(0));
    roll =Float.parseFloat(nums.get(1));
    yaw  =Float.parseFloat(nums.get(2));
  } else if(line.startsWith("Accel")){
    ax_g=Float.parseFloat(nums.get(0));
    ay_g=Float.parseFloat(nums.get(1));
    az_g=Float.parseFloat(nums.get(2));
    ax=round(ax_g*16384); histAx.add(ax); if(histAx.size()>MAX_HISTORY) histAx.remove(0);
    ay=round(ay_g*16384); histAy.add(ay); if(histAy.size()>MAX_HISTORY) histAy.remove(0);
    az=round(az_g*16384); histAz.add(az); if(histAz.size()>MAX_HISTORY) histAz.remove(0);
    histAxG.add(ax_g); if(histAxG.size()>MAX_HISTORY) histAxG.remove(0);
    histAyG.add(ay_g); if(histAyG.size()>MAX_HISTORY) histAyG.remove(0);
    histAzG.add(az_g); if(histAzG.size()>MAX_HISTORY) histAzG.remove(0);
  } else if(line.startsWith("Gyro")){
    gx=int(round(Float.parseFloat(nums.get(0))));
    gy=int(round(Float.parseFloat(nums.get(1))));
    gz=int(round(Float.parseFloat(nums.get(2))));
  } else if(line.startsWith("Temp")){
    temp=Float.parseFloat(nums.get(0));
  }
}

void calibrar(int v){
  if(port!=null){
    port.write('C');
    calibrating = true;
    calStart = millis();
  }
}

void servo1Angle(float v){
  servo1Angle=int(v);
  if(port!=null) port.write("S,"+servo1Angle+","+servo2Angle+"\n");
}

void servo2Angle(float v){
  servo2Angle=int(v);
  if(port!=null) port.write("S,"+servo1Angle+","+servo2Angle+"\n");
}

void motorSelect(int v){
  motorSelect=v;
  sendMotorCommand();
}

void motorVelocity(float v){
  motorVelocity=int(v);
  sendMotorCommand();
}

void motorTime(float v){
  motorTime=int(v);
}

void sendMotorCommand(){
  if(port!=null && millis()-lastMotorSend>50){
    port.write("M,"+motorVelocity+","+motorTime+","+motorSelect+"\n");
    lastMotorSend=millis();
  }
}

void openSerialPort(String name){
  if(port!=null) port.stop();
  port=new Serial(this,name,115200);
  port.clear();
  port.bufferUntil('\n');
}

void serialPort(int idx){
  String[] ps=Serial.list();
  if(idx>=0 && idx<ps.length){
    openSerialPort(ps[idx]);
  }
}

void showX(boolean v){ showX=v; }
void showY(boolean v){ showY=v; }
void showZ(boolean v){ showZ=v; }
void useRaw(boolean v){ useRaw=v; }
