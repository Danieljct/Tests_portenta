#include <Arduino.h>
#include "SM_4000.h"

// Estas definiciones deben estar antes del include
#define _TIMERINTERRUPT_LOGLEVEL_     0

#include "Portenta_H7_TimerInterrupt.h"

// Variables para la interrupción
volatile bool readSensor = false;

// Init timer TIM12 (uno de los timers disponibles en Portenta H7)
Portenta_H7_Timer ITimer(TIM12);

// Función de callback de la interrupción del timer
void TimerHandler() {
  readSensor = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  bootM4();
  
  // Inicializar solo sensor SM4291
  SM_4000_begin();
  
  Serial.println("=== SENSOR SM4291 - 2kHz con Portenta_H7_Timer ===");
  
  // Configurar timer para 2kHz (500us = 500 microsegundos)
  if (ITimer.attachInterruptInterval(500, TimerHandler)) {
    Serial.println("Timer configurado correctamente a 2kHz (500us)");
  } else {
    Serial.println("Error: No se pudo configurar el timer");
  }
}

void loop() {
  // Verificar si el timer disparó la interrupción
  if (readSensor) {
    readSensor = false; // Limpiar flag
    
    float pressureMbar = SM_4000_readI2C_pressure();
    
    if (pressureMbar != -1.0) {
      Serial.println(pressureMbar, 6);  // 6 decimales de precisión
    } else {
      Serial.println("ERROR");
    }
  }
}