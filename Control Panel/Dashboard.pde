import processing.serial.*;
import controlP5.*;
import java.util.regex.*;
import java.util.Arrays;

Serial port;
ControlP5 cp5;

// Variables de datos
float pitch, roll, yaw, temp;
int ax, ay, az, gx, gy, gz;

// Historial de datos
final int MAX_HISTORY = 200;
ArrayList<Integer> histAx = new ArrayList<>();
ArrayList<Integer> histAy = new ArrayList<>();
ArrayList<Integer> histAz = new ArrayList<>();

// Historial de consola serial
ArrayList<String> serialLog = new ArrayList<String>();
final int MAX_LOG_LINES = 10; // Número máximo de líneas a mostrar

// Controles
int servo1Angle = 90, servo2Angle = 90;
int motorSelect = 1, motorVelocity = 0, motorTime = 1000;

// UI
PFont fontH1, fontH2, fontText;
PImage imgPlaceholder;
boolean calibrating = false;
int calStart, calDur = 2000;

// Control de envío de comandos
int lastMotorSend = 0;

// Nuevos colores (distintos a los de aceleración)
color pitchColor = color(255, 128, 0);   // Naranja
color rollColor  = color(128, 0, 255);   // Morado
color yawColor   = color(0, 255, 255);   // Cian

void setup() {
  size(1280, 720);
  smooth(8);
  frameRate(144);
  
  // Fuentes
  fontH1 = createFont("Arial Bold", 28);
  fontH2 = createFont("Arial", 20);
  fontText = createFont("Arial", 16);
  textFont(fontText);
  
  // Imagen placeholder
  imgPlaceholder = createImage(480, 320, RGB);
  imgPlaceholder.loadPixels();
  for (int i = 0; i < imgPlaceholder.pixels.length; i++) {
    imgPlaceholder.pixels[i] = color(50);
  }
  imgPlaceholder.updatePixels();
  
  // UI framework
  cp5 = new ControlP5(this);
  
  // --- Comunicación serial con selector ---
  println(Serial.list());
  String[] ports = Serial.list();

  // Creamos el dropdown de puertos
  cp5.addDropdownList("serialPort")
     .setLabel("Puerto Serial")
     .setPosition(60, 80)
     .setSize(200, ports.length * 20)
     .setItemHeight(20)
     .setBarHeight(20)
     .setFont(fontText)
     .addItems(Arrays.asList(ports));

  // Abrimos el primer puerto por defecto
  if (ports.length > 0) {
    openSerialPort(ports[0]);
  }
  
  // Botones
  cp5.addButton("calibrar")
     .setLabel("CALIBRAR")
     .setPosition(60, 650)
     .setSize(120, 40)
     .setFont(fontText);
  
  // Controles de servos
  cp5.addSlider("servo1Angle")
     .setLabel("Servo 1")
     .setRange(0, 180)
     .setValue(90)
     .setPosition(width - 380, height - 220)
     .setSize(200, 20)
     .setFont(fontText);
  
  cp5.addSlider("servo2Angle")
     .setLabel("Servo 2")
     .setRange(0, 180)
     .setValue(90)
     .setPosition(width - 380, height - 180)
     .setSize(200, 20)
     .setFont(fontText);
  
  // Controles de motores
  cp5.addDropdownList("motorSelect")
   .setLabel("Motor")
   .addItem("Motor 1", 1)
   .addItem("Motor 2", 2)
   .addItem("Ambos", 3)
   .setPosition(width - 380, height - 140)
   .setSize(100, 100)
   .setFont(fontText)
   .setValue(0);
  
  cp5.addSlider("motorVelocity")
     .setLabel("Potencia")
     .setRange(-255, 255)
     .setValue(0)
     .setPosition(width - 380, height - 110)
     .setSize(200, 20)
     .setFont(fontText);
  
  cp5.addSlider("motorTime")
     .setLabel("Tiempo (ms)")
     .setRange(0, 5000)
     .setValue(1000)
     .setPosition(width - 380, height - 80)
     .setSize(200, 20)
     .setFont(fontText);
}

void draw() {
  background(18);
  drawHeader();
  drawImageSection();
  drawTelemetryPanel();
  drawGraphSection();
  drawServosMotorsPanel();
  drawCalibrationBar();
  drawSerialConsole();
}

void drawHeader() {
  textFont(fontH1);
  fill(220);
  textAlign(LEFT, CENTER);
  text("M0-V Dashboard", 40, 40);
}

void drawImageSection() {
  float ix = width - imgPlaceholder.width - 40;
  float iy = 80;
  noStroke();
  fill(60);
  rect(ix - 10, iy - 10, imgPlaceholder.width + 20, imgPlaceholder.height + 20, 8);
  image(imgPlaceholder, ix, iy);
}

void drawTelemetryPanel() {
  float x = 40, y = 80, w = 520, h = 240, pad = 16;
  noStroke();
  fill(40, 200);
  rect(x, y, w, h, 8);

  textFont(fontH2);
  fill(200);
  text("Telemetría", x + pad, y + pad);

  textFont(fontText);
  float ty = y + pad + 32;
  fill(235); text("Pitch: ", x+pad, ty);    fill(pitchColor); text(nf(pitch,1,2)+"°", x+pad+60, ty);
  fill(235); text("Roll: ", x+pad, ty+24);  fill(rollColor);  text(nf(roll,1,2)+"°", x+pad+60, ty+24);
  fill(235); text("Yaw: ", x+pad, ty+48);   fill(yawColor);   text(nf(yaw,1,2)+"°", x+pad+60, ty+48);
  fill(235); text("Temp: " + nf(temp,1,2)+" °C", x+pad, ty+72);

  fill(235); text("Aceleración:", x+pad, ty+100);
  fill(#FF0000); text("X:"+ax, x+pad+100, ty+100);
  fill(#00FF00); text("Y:"+ay, x+pad+200, ty+100);
  fill(#0000FF); text("Z:"+az, x+pad+300, ty+100);

  fill(235); text("Gyro → X:"+gx+" Y:"+gy+" Z:"+gz, x+pad, ty+124);
}

void drawGraphSection() {
  float x = 40, y = 340, w = 600, h = 180;
  noStroke();
  fill(40, 200);
  rect(x, y, w, h, 8);
  drawAccelerationGraph(x+20, y+20, w-40, h-40);
  float barY = y + h + 20;
  drawAngleBar(x+20, barY,   w-40, 20, pitch, "Pitch", pitchColor);
  drawAngleBar(x+20, barY+40,w-40, 20, roll,  "Roll",  rollColor);
  drawAngleBar(x+20, barY+80,w-40, 20, yaw,   "Yaw",   yawColor);
}

void drawAccelerationGraph(float x, float y, float w, float h) {
  stroke(120);
  line(x, y+h, x+w, y+h);
  line(x, y,   x,   y+h);

  fill(200); textSize(10);
  text("-17000", x - 40, y + h);
  text("0",      x - 20, y + h/2);
  text("17000",  x - 40, y);

  drawDataCurve(histAx, #FF0000, x, y, w, h);
  drawDataCurve(histAy, #00FF00, x, y, w, h);
  drawDataCurve(histAz, #0000FF, x, y, w, h);
}

void drawDataCurve(ArrayList<Integer> data, color c, float x, float y, float w, float h) {
  if (data.size() < 2) return;
  stroke(c);
  noFill();
  beginShape();
  for (int i = 0; i < data.size(); i++) {
    float xp = map(i, 0, data.size()-1, x, x+w);
    float yp = map(data.get(i), -17000, 17000, y+h, y);
    vertex(xp, yp);
  }
  endShape();
}

void drawAngleBar(float x, float y, float w, float h, float val, String label, color c) {
  float filled = map(val, -180, 180, 0, w);
  noStroke();
  fill(60); rect(x, y, w, h, 4);
  fill(c); rect(x, y, filled, h, 4);
  fill(235); textAlign(LEFT, CENTER);
  text(label + ": " + nf(val,1,2) + "°", x + w + 15, y + h/2);
}

void drawServosMotorsPanel() {
  float x = width - 400, y = height - 280, w = 360, h = 200;
  noStroke(); fill(40, 200); rect(x, y, w, h, 8);
  textFont(fontH2); fill(200);
  text("Control de Servos y Motores", x+20, y+20);
  textFont(fontText); fill(235);
  text("Servo 1: " + servo1Angle + "°", x+20, y+50);
  text("Servo 2: " + servo2Angle + "°", x+20, y+80);
  text("Motor: V=" + motorVelocity + " M=" + motorSelect, x+20, y+110);
  text("Duración: " + motorTime + "ms", x+20, y+140);
}

void drawCalibrationBar() {
  if (!calibrating) return;
  float x = width/2 - 200, y = height/2 - 20, w = 400, h = 30;
  float t = constrain((millis() - calStart) / (float)calDur, 0, 1);
  noStroke(); fill(50); rect(x, y, w, h, 4);
  fill(0, 150, 250); rect(x, y, w * t, h, 4);
  fill(230); textFont(fontText); textAlign(CENTER, CENTER);
  text("Calibrando... " + int(t*100) + "%", x + w/2, y + h/2);
  if (t >= 1) calibrating = false;
}

void drawSerialConsole() {
  float x = 480, y = height - 128, w = 500, h = 128;
  noStroke(); fill(30, 200); rect(x, y, w, h, 8);
  textFont(fontH2); fill(200); text("Consola Serial", x + 10, y + 25);
  textFont(fontText); fill(180);
  for (int i = 0; i < serialLog.size(); i++) {
    text(serialLog.get(i), x + 10, y + 40 + i * 16);
  }
}

void serialEvent(Serial p) {
  String s = p.readStringUntil('\n');
  if (s == null) return;
  String line = s.trim();
  ArrayList<String> nums = new ArrayList<>();
  Matcher m = Pattern.compile("[-]?\\d+\\.?\\d*").matcher(line);
  while (m.find()) nums.add(m.group());

  if (line.startsWith("Angle")) {
    pitch = Float.parseFloat(nums.get(0));
    roll  = Float.parseFloat(nums.get(1));
    yaw   = Float.parseFloat(nums.get(2));
  } else if (line.startsWith("Accelerometer")) {
    ax = Integer.parseInt(nums.get(0));
    ay = Integer.parseInt(nums.get(1));
    az = Integer.parseInt(nums.get(2));
    histAx.add(ax); histAy.add(ay); histAz.add(az);
    if (histAx.size() > MAX_HISTORY) {
      histAx.remove(0); histAy.remove(0); histAz.remove(0);
    }
  } else if (line.startsWith("Gyroscope")) {
    gx = Integer.parseInt(nums.get(0));
    gy = Integer.parseInt(nums.get(1));
    gz = Integer.parseInt(nums.get(2));
  } else if (line.startsWith("Temperature")) {
    temp = Float.parseFloat(nums.get(0));
  }

  serialLog.add(line);
  if (serialLog.size() > MAX_LOG_LINES) serialLog.remove(0);
}

void calibrar(int v) {
  if (port != null) {
    port.write('C');
    calibrating = true;
    calStart = millis();
  }
}

// --- Métodos de callback para control instantáneo ---
void servo1Angle(float v) {
  servo1Angle = int(v);
  if (port != null) port.write("S,"+servo1Angle+","+servo2Angle+"\n");
}

void servo2Angle(float v) {
  servo2Angle = int(v);
  if (port != null) port.write("S,"+servo1Angle+","+servo2Angle+"\n");
}

void motorSelect(int v) {
  motorSelect = v;
  sendMotorCommand();
}

void motorVelocity(float v) {
  motorVelocity = int(v);
  sendMotorCommand();
}

void motorTime(float v) {
  motorTime = int(v);
  // Opcional: descomenta si quieres que el tiempo también active el motor
  // sendMotorCommand();
}

void sendMotorCommand() {
  if (port != null) {
    // Limita la frecuencia de envío para no saturar el puerto serial
    if (millis() - lastMotorSend > 50) { // Envía máximo cada 50ms
      port.write("M," + motorVelocity + "," + motorTime + "," + motorSelect + "\n");
      println("Enviado motor: V=" + motorVelocity + " T=" + motorTime + " M=" + motorSelect);
      lastMotorSend = millis();
    }
  }
}

void openSerialPort(String portName) {
  if (port != null) {
    port.stop();
    port = null;
  }
  port = new Serial(this, portName, 115200);
  port.clear();
  port.bufferUntil('\n');
}

void serialPort(int idx) {
  String[] ports = Serial.list(); 
  if (idx >= 0 && idx < ports.length) {
    openSerialPort(ports[idx]);
    serialLog.clear();
  }
}
