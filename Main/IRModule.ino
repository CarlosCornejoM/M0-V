#include <IRremote.hpp>
extern const int IR_RECEIVE_PIN;  // de Param.ino

void initIR() {
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void handleIR() {
  if (IrReceiver.decode()) {
    Serial.print("IR raw: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume();
  }
}
