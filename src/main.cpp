#include <Arduino.h>

// ===== Configuración de pines =====
const int potPin = A0;      // Pin del potenciómetro
const int pwmPin = D6;      // Pin de salida PWM
const int adcPwmPin = A1;   // Pin para leer la propia PWM

// ===== Configuración de resolución =====
const int adcResolution = 12;  // Resolución ADC: 12 bits → 0-4095
const int pwmResolution = 8;   // PWM: 8 bits → 0-255

void setup() {
    Serial.begin(115200);

    // Configurar salida PWM
    pinMode(pwmPin, OUTPUT);
    analogWriteResolution(pwmResolution);

    // Configurar entradas analógicas
    analogReadResolution(adcResolution);

    Serial.println("=== PWM controlada por potenciómetro + lectura de señal ===");
}

void loop() {
    // Leer potenciómetro
    int potValue = analogRead(potPin);

    // Mapear potenciómetro → PWM
    int pwmValue = map(potValue, 0, 4095, 0, 255);
    analogWrite(pwmPin, pwmValue);

    // Medir la propia señal PWM
    int measuredPWM = analogRead(adcPwmPin);

    // Mostrar sólo la lectura de la señal
    Serial.println(measuredPWM);

    delay(2);  // Retardo pequeño para refrescar rápido (~500 muestras/s)
}
