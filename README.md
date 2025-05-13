
## Descripción del proyecto

- **Objetivo**: Diseñar y construir un robot de péndulo invertido que se equilibre por sí mismo mediante control PID.  
- **Inspiración**: Diseño basado en el robot M0-V de *WALL·E*.  
- **Componentes principales**:  
  - Arduino UNO (o compatible) + MPU6050 (acelerómetro/giroscopio) + receptor IR  
  - 2 servos (pines 9 y 10)  
  - 2 motores NEMA controlados por puente H (pines ENA/ENB, IN1–IN4)  
  - Dashboard en Processing para telemetría y control por serie  

## Hardware

1. **Microcontrolador**: Arduino UNO o equivalente  
2. **Sensores**:  
   - MPU6050  
   - Receptor IR (pin 2)  
   - HC-SR04 ultrasonico (pendiente)  
3. **Actuadores**:  
   - 2× Servos SG90  
   - 2× Motores NEMA en puente H  
4. **Interfaz**:  
   - Cable USB  
   - PC con Processing  
5. **Estructura mecánica**:  
   - Modelos CAD en `hardware/CAD/`  
   - Impresión 3D (pendiente)

## Software

### Arduino (`arduino/inverted_pendulum.ino`)

- Inicializa y calibra MPU6050  
- Comandos seriales:  
  - `S,a1,a2\n` → ajusta servos  
  - `M,vel,tiempo,motor\n` → controla motores  
- Lectura de IR y muestra código bruto  
- Envía telemetría periódica:
- Funciones de avance y freno seguro

### Processing (`processing/DashboardM0V/DashboardM0V.pde`)

- Conexión a puerto serie (115200 bps)  
- Panel de telemetría: ángulos, aceleraciones, giroscopio  
- Gráfico histórico de aceleraciones  
- Control CP5 para servos y motores  
- Consola serial integrada  
- Placeholder para futura visualización 3D

# ROADMAP

## Corto plazo (1–2 semanas)

- Determinar y mapear **códigos IR** para control remoto  
- Mejorar y depurar el código Arduino y Processing  
- Ajustar precisión de calibración y filtrado de sensores  
- En la dashboard:  
  - Cubo 3D que refleje en tiempo real **pitch/roll/yaw**  
  - Integrar sensor ultrasónico HC-SR04 y mostrar la línea de distancia al suelo dentro del cubo 3D  
  - Añadir LEDs de estado (calibración, alerta de balance)

## Mediano plazo (1–2 meses)

- Completar diseño físico del robot en CAD  
- Impresión 3D de piezas estructurales  
- Simulación y análisis en ANSYS (momentos de inercia, masa)  
- Ajustar geometría y centro de masa para mejorar estabilidad  
- Montaje completo de todos los componentes en el chasis impreso

## Largo plazo (3–6 meses)

- Implementar controlador **PID** completo en Arduino  
- Lograr equilibrio automático del robot en posición invertida  
- Control remoto por IR usando los códigos definidos  
- Diseñar e implementar PCB para sustituir protoboard  
- Añadir reconocimiento de obstáculos con ultrasonido y/o sensores adicionales  

