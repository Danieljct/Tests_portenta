#include <Arduino.h>
#include "window_analysis.h"
#include "ABPLLN.h"
#include "sensor_elv.h"
#include "SM_4000.h"
#include "CCDANN600MDSA3.h"

// Enum para los estados de sensores
enum SensorState {
  SENSOR_ABPLLN = 0,
  SENSOR_ELV = 1,
  SENSOR_SM4291 = 2,
  SENSOR_SM4000_ANALOG = 3,
  SENSOR_CCDANN600 = 4
};

// Variables para control del botón y toggle
SensorState currentSensor = SENSOR_ABPLLN;
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
#define BTN PC_7

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("=== SISTEMA DE LECTURA DE PRESIÓN TRIPLE ===");
  
  bootM4();

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(BTN, INPUT_PULLDOWN);
  setLed(LED_OFF);
  
  // Inicializar los tres sensores
  initPressureSensor(); // ABPLLN
  sensorELV_begin();    // ELV
  SM_4000_begin();      // SM4291 (usando SM_4000)
  CCDANN600MDSA3_begin();
  sensorELV_scan();    // Escanear dispositivos I2C3

  Serial.println("Presiona el botón para cambiar entre sensores");
  Serial.println("Estado inicial: ABPLLN");
}

void checkButton() {
  int reading = digitalRead(BTN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      if (currentButtonState == LOW) { // Botón presionado
        currentSensor = (SensorState)((currentSensor + 1) % 5);
        Serial.println("=== CAMBIO DE SENSOR ===");
        switch (currentSensor) {
          case SENSOR_ABPLLN:
            Serial.println("Modo: ABPLLN");
            break;
          case SENSOR_ELV:
            Serial.println("Modo: ELV");
            break;
          case SENSOR_SM4291:
            Serial.println("Modo: SM4291");
            break;
          case SENSOR_CCDANN600:
            Serial.println("Modo: CCDANN600MDSA3");
            break;
          case SENSOR_SM4000_ANALOG:
            Serial.println("Modo: SM4000 Analógico");
            break;
        }
        Serial.println("========================");
      }
    }
  }
  
  lastButtonState = reading;
}

void loop() {



  static unsigned long lastReading = 0;
  
  unsigned long now = millis();
  
  // Verificar botón
  checkButton();
  
  // Lectura cada 20ms
  if (now - lastReading >= 20) {
    switch (currentSensor) {
      case SENSOR_ABPLLN:
        // Leer sensor ABPLLN
        if (readPressureSensor()) {
          Serial.print("ABPLLN: ");
          Serial.print(currentPressure, 2);
          Serial.println(" mbar");
        }
        break;
        
      case SENSOR_ELV:
        // Leer sensor ELV
        {
          int pressureRaw = sensorELV_read(false, false);
          if (!isnan(pressureRaw)) {
            float pressureMbar = pressure_raw_to_pressure_mbar(pressureRaw) * 1000;
            Serial.print("ELV: ");
            Serial.print(pressureMbar, 2);
            Serial.println(" mbar");
          }
        }
        break;
        
      case SENSOR_SM4291:
        // Leer sensor SM4291 usando SM_4000
        {
          float pressureMbar = SM_4000_readI2C_pressure();
          if (pressureMbar != -1.0) {
            Serial.print("SM4291: ");
            Serial.print(pressureMbar, 2);
            Serial.println(" mbar");
          }
        }
        break;
        
      case SENSOR_CCDANN600:
        // Leer sensor CCDANN600MDSA3
        {
          float pressureMbar = CCDANN600MDSA3_read();
          Serial.print("CCDANN600: ");
          Serial.print(pressureMbar, 2);
          Serial.println(" mbar");
        }
        break;
        
      case SENSOR_SM4000_ANALOG:
        // Leer sensor SM4000 analógico
        {
          float pressureMbar = SM_4000_readAnalog();
          Serial.print("SM4000 Analógico: ");
          Serial.print(pressureMbar, 2);
          Serial.println(" mbar");
        }
        break;
    }
    lastReading = now;
  }
  
  delay(10);
}
