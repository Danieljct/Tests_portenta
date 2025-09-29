#include <Arduino.h>
#include "sensor_elv.h"
#include "window_analysis.h"

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Presion, curtosis");
  bootM4();
  sensorELV_begin();

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  setLed(LED_OFF);
}

void loop() {
  int pressure = sensorELV_read();
  addSampleToWindow(pressure);

  Serial.print(pressure_raw_to_pressure_mbar(pressure), 4); Serial.print(", ");
  
  processWindowAnalysis();

  delay(10);
}
