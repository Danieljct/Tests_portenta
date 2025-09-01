#include <Arduino.h>

const int potPin = A0;       // Potenciómetro
const int pwmPin = D6;       // Salida PWM
const int measurePin = A1;   // Pin para medir PWM

void setup() {
  Serial.begin(2000000); // Velocidad alta
  analogWriteResolution(12);  // PWM a 12 bits
  analogReadResolution(12);   // ADC a 12 bits
  pinMode(pwmPin, OUTPUT);
}

void loop() {
  // Leer potenciómetro
  uint16_t potValue = analogRead(potPin); // 0..4095

  // Ajustar PWM según potenciómetro
  analogWrite(pwmPin, potValue);

  // Medir la propia PWM en A1
  uint16_t pwmMeasure = analogRead(measurePin);

  // Imprimir la lectura
  Serial.println(pwmMeasure);
}
