#include <Arduino.h>
#include "sensor_elv.h"

#define BUTTON_PIN  D4
#define BUFFER_SIZE 512

volatile bool buttonPressed = false;

void handleButtonInterrupt() {
  buttonPressed = true;
}

float pressureBuffer[BUFFER_SIZE];
size_t bufferIndex = 0;
bool recording = false;
bool sending = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Hola desde M7 por UART!");
  bootM4();
  sensorELV_begin();

  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, RISING);
}

void loop() {
  // Manejo del botón (debounce simple)
  static unsigned long lastDebounce = 0;
  if (buttonPressed && (millis() - lastDebounce > 200)) {
    buttonPressed = false;
    lastDebounce = millis();

    if (!recording && !sending) {
      // Primer toque: comenzar a grabar
      recording = true;
      bufferIndex = 0;
      Serial.println("Grabando datos de presión...");
    } else if (recording) {
      // Segundo toque: dejar de grabar y comenzar a enviar
      recording = false;
      sending = true;
      Serial.print("Enviando buffer de ");
      Serial.print(bufferIndex);
      Serial.println(" muestras...");
    } else if (sending) {
      // Tercer toque: volver a grabar (borrar buffer anterior)
      sending = false;
      recording = true;
      bufferIndex = 0;
      Serial.println("Reiniciando grabación de datos de presión...");
    }
  }

  if (recording) {
    if (bufferIndex < BUFFER_SIZE) {
      float pressure = sensorELV_read();
      pressureBuffer[bufferIndex++] = pressure;
    }
    delay(10); // Ajusta la frecuencia de muestreo si lo deseas
  } else if (sending && bufferIndex > 0) {
    // Enviar el buffer por Serial de manera continua
    for (size_t i = 0; i < bufferIndex; i++) {
      Serial.print("Presion[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(pressureBuffer[i], 4);
      delay(5); // Pequeño retardo para no saturar el puerto serie
    }
    Serial.println("---- Fin de buffer, repitiendo ----");
    delay(500); // Espera antes de volver a enviar el buffer
  } else {
    // Si no está grabando ni enviando, solo lee el sensor normalmente (opcional)
    delay(100);
  }
}
