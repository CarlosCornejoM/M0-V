# Robot tipo WALL·E con ESP8266 y Arduino

Este repositorio contiene el código fuente y documentación de un robot inspirado en WALL·E (modelo M0-V) que emplea un Arduino (Uno/Nano) para control y un ESP8266 como interfaz web para monitoreo y control remoto en tiempo real.

## Descripción general

### Arduino Uno/Nano:
- Inicializa y calibra el sensor MPU6050 con filtros digitales y pasa-bajos de 20Hz
- Controla dos motores paso a paso NEMA17 (200 pasos/rev) con drivers DRV8825
- Mueve dos servomotores SG90 (0-180°) con control suavizado
- Ejecuta un control PID para equilibrar el sistema pendular invertido con anti-windup
- Gestiona motor DC con control PWM y H-bridge
- Envía telemetría periódica por puerto serie (115200 bps)

### ESP8266:
- Se conecta a la red WiFi (configurable en ESP8266.ino)
- Sirve una interfaz web desde index_html.h
- Establece un WebSocket en el puerto 81 para comunicación bidireccional
- Emite un JSON con el estado completo cada 20 ms
- Recibe comandos desde la página web para control remoto

### Interfaz web:
- Panel de valores de sensor (pitch, roll, yaw, aceleración, temperatura)
- Consola serial con logs en tiempo real
- Gráficos de latencia WebSocket, RPM de steppers y parámetros PID
- Controles para ajustar servos, RPM, PID y reproducir audio
- Monitoreo de estado de LEDs (ESP, Arduino, DC)
- Soporte para gamepad/joystick

## Especificaciones técnicas

### Hardware requerido:
- Arduino Uno + ESP8266
- MPU6050 (giroscopio/acelerómetro)
- 2x Motor NEMA17 con drivers DRV8825
- 2x Servomotor SG90
- Motor DC con H-bridge
- Componentes electrónicos varios

### Características del sistema:
- Comunicación serie: 115200 bps
- Velocidad I2C: 100kHz (estabilidad mejorada)
- Filtro digital MPU6050: 20Hz pasa-bajos
- Control PID: 20ms de período
- Telemetría MPU: 20ms de período
- WebSocket: 20ms de actualización
- MAX_RPM configurable: 30 RPM por defecto


## Estructura de archivos

```
├─ Arduino/            # Código de control en Arduino
│  ├─ Main.ino         # Lógica de inicialización y bucle principal
│  ├─ Param.ino        # Definición de pines y parámetros globales
│  ├─ mpu_module.ino   # Lectura, filtrado y telemetría de MPU6050
│  ├─ dual_stepper_control.ino  # Control de dos motores NEMA17
│  ├─ PID_control.ino  # Algoritmo PID para estabilización invertida
│  └─ Audio.ino        # Generación de sirena y reproducción de melodías
├─ ESP8266/            # Firmware del ESP8266 y web UI
│  ├─ ESP8266.ino      # Conexión WiFi, WebSockets, parseo Serial y broadcast
│  ├─ index_html.h     # Página HTML/CSS/JS incrustada para la UI web
│  ├─ audio.h          # Recursos de audio en PROGMEM
│  └─ README.md        # Documentación de alto nivel (este archivo)
```

## Flujo de funcionamiento

### 1. Inicialización:
- Arduino arranca módulos: parámetros, steppers, motores DC, MPU, comunicación serie, PID
- ESP8266 arranca en modo station, se conecta al SSID configurado y lanza el servidor HTTP y WebSocket

### 2. Telemetría:
Arduino envía mensajes serial con prefijos:
- `[ANGLES,pitch,roll,yaw,temp]` - Ángulos y temperatura
- `[RPM1,val]`, `[RPM2,val]` - RPM de steppers
- `[PID,error,output,integral]` - Estados del PID
- ESP8266 parsea y actualiza su estado interno
- Cada 20 ms envía por WebSocket un JSON con todos los campos

### 3. Interfaz web:
- Conexión WebSocket en `ws://<IP>:81/`
- Actualización en tiempo real de valores y gráficos
- Envío de comandos JSON:
  - `{"cmd":"setServos","a1":...,"a2":...}` - Control de servos
  - `{"cmd":"setMRPM","rpm":...}` - RPM máximo de steppers
  - `{"cmd":"setPID","sp":...,"kp":...,"ki":...,"kd":...}` - Parámetros PID
  - `{"cmd":"playMelody"}`, `{"cmd":"playAudio"}` - Reproducción audio
  - `{"cmd":"startSiren"}`, `{"cmd":"stopSiren"}` - Control sirena
  - `{"cmd":"espOn"}/{"cmd":"espOff"}` - LEDs ESP
  - `{"cmd":"unoOn"}/{"cmd":"unoOff"}` - LEDs Arduino
  - `{"cmd":"dcOn"}/{"cmd":"dcOff"}` - Motor DC
  - `{"cmd":"JOY_L","x":...,"y":...}` - Control diferencial steppers
  - `{"cmd":"gamepad",...}` - Control por gamepad

### 4. Control por gamepad:
- La página captura un joystick Gamepad API
- Stick izquierdo: comando JOY_L (control diferencial steppers)
- Stick derecho: ajuste automático de servos
- Gatillos: nivel DC
- Botones mapeados a acciones definidas por el usuario

## Configuración de pines

### Arduino:
- **MPU6050**: I2C (SDA/SCL)
- **Steppers**: DIR1=2, STEP1=3, DIR2=4, STEP2=5, EN1=A0, EN2=A1
- **H-bridge**: ENA=14, ENB=7, IN1=8, IN2=12, IN3=10, IN4=13
- **Servos**: SERVO1=6, SERVO2=9
- **Motor DC**: PWM=11
- **Ultrasonido**: TRIG=A3, ECHO=6
- **LED**: LED_BUILTIN

### ESP8266:
- **Comunicación serie**: RX/TX con Arduino
- **LED**: D4 (GPIO2)
- **WiFi**: Antena integrada

## Algoritmos implementados

### Control PID:
- Período: 20ms
- Anti-windup: Integral limitada a ±1000
- Filtro derivativo: Pasa-bajos con coeficiente 0.7
- Salida limitada a MAX_RPM

### Control de steppers:
- Suavizado exponencial: Factor 0.15
- Aceleración: 2x velocidad máxima
- Frecuencia de ejecución: 500Hz máximo
- Habilitación automática por velocidad

### Filtrado MPU6050:
- Filtro digital interno: 20Hz
- Filtro pasa-bajos software: Coeficiente 0.8
- Verificación de conexión continua
- Manejo de errores robusto

## Configuración

### 1. En ESP8266.ino, ajusta:
```cpp
const char* ssid     = "MOV";
const char* password = "12345678";
```

### 2. Verifica los pines en Param.ino y ESP8266.ino

### 3. Flashea primero el firmware de Arduino, luego el del ESP8266

### 4. Abre el navegador en la IP mostrada por el ESP en el monitor serie

## Optimizaciones implementadas

### Rendimiento:
- Filtros de software para reducir ruido
- Suavizado exponencial en steppers
- Telemetría adaptativa (solo envía si hay cambios significativos)
- Frecuencias de ejecución optimizadas

### Estabilidad:
- Velocidad I2C reducida (100kHz)
- Filtros anti-jitter en steppers
- Verificación de conexión MPU6050
- Manejo de errores en comunicación serie

### Eficiencia:
- Reducción de spam en telemetría
- Buffers optimizados
- Timers independientes para cada módulo
- Control de flujo en WebSocket

## Roadmap

- Optimización de filtros del MPU6050 y suavizado de steppers
- Mejoras en logging y manejo de errores en la web
- Implementación de PCB personalizada
- Soporte para múltiples sensores
- Integración con ROS (Robot Operating System)
- Modo autónomo con navegación

## Troubleshooting

### Problemas comunes:
- **MPU6050 se corrompe**: Verificar conexiones I2C y alimentación estable
- **Steppers con jitter**: Ajustar SMOOTH_FACTOR y verificar drivers
- **WebSocket desconecta**: Verificar potencia de señal WiFi
- **PID inestable**: Ajustar parámetros Kp, Ki, Kd gradualmente

### Logs de debug:
- Monitor serie Arduino: Estados de sensores y actuadores
- Consola web: Comunicación WebSocket y errores JavaScript
- Debug ESP8266: Información de conexión WiFi y memoria

¡Gracias por tu interés! Cualquier issue o PR es bienvenido.
