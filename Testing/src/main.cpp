#include <Arduino.h>
#include "sensor_elv.h"

// Usar el objeto Serial1 existente proporcionado por el core de Arduino
// HardwareSerial Serial1(PA_9, PA_10); // TX, RX (D13, D14)

void setup() {
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Hola desde M7 por UART!");
  bootM4();
  sensorELV_begin();
}

void loop() {
sensorELV_read();
  delay(100);
}
