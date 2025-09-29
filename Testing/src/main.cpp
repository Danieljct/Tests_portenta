#include <Arduino.h>
#include "window_analysis.h"
#include "ABPLLN.h"

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("=== SISTEMA DE LECTURA DE PRESIÓN ===");
  
  bootM4();

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  setLed(LED_OFF);
  
  // Inicializar sensor de presión
  initPressureSensor();
}

void loop() {
  static unsigned long lastReading = 0;
  
  unsigned long now = millis();
  
  // Lectura cada 20ms
  if (now - lastReading >= 20) {
    if (readPressureSensor()) {
      Serial.print(currentPressure, 2);
      Serial.println(" mbar");
    }
    lastReading = now;
  }
  
  delay(10);
}