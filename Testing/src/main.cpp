#include <Arduino.h>
#include <arduinoFFT.h>
#include "window_analysis.h"
#include "ABPLLN.h"
#include "sensor_elv.h"
#include "SM_4000.h"
#include "CCDANN600MDSA3.h"

#define WAIT_TIME 500  // 500us para 2kHz
#define FFT_SIZE 4096  
#define SAMPLES_PER_SECOND 2000

// Enum para los sensores
enum SensorState {
  SENSOR_ABPLLN = 0,
  SENSOR_ELV = 1,
  SENSOR_SM4291 = 2,
  SENSOR_CCDANN600 = 3
};

// Variables globales
SensorState currentSensor = SENSOR_ABPLLN;
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
#define BTN PC_7

// Buffer para FFT
float buffer[FFT_SIZE];
uint16_t bufferIndex = 0;
bool bufferFull = false;

// Variables para FFT
float vReal[FFT_SIZE];
float vImag[FFT_SIZE];
ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, FFT_SIZE, SAMPLES_PER_SECOND);

// Función para leer el sensor actual
float readCurrentSensor() {
  float pressure = 0.0;
  
  switch (currentSensor) {
    case SENSOR_ABPLLN:
      if (readPressureSensor()) {
        pressure = currentPressure;
      }
      break;
      
    case SENSOR_ELV:
      {
        int pressureRaw = sensorELV_read(false, false);
        if (!isnan(pressureRaw)) {
          pressure = pressure_raw_to_pressure_mbar(pressureRaw) * 1000;
        }
      }
      break;
      
    case SENSOR_SM4291:
      {
        float pressureMbar = SM_4000_readI2C_pressure();
        if (pressureMbar != -1.0) {
          pressure = pressureMbar;
        }
      }
      break;
      
    case SENSOR_CCDANN600:
      pressure = CCDANN600MDSA3_read();
      break;
  }
  
  return pressure;
}

// Función para agregar muestra al buffer
void addSample(float sample) {
  buffer[bufferIndex] = sample;
  bufferIndex++;
  
  if (bufferIndex >= FFT_SIZE) {
    bufferIndex = 0;
    bufferFull = true;
  }
}

// Función para calcular y imprimir FFT
void calculateAndPrintFFT() {
  if (!bufferFull) {
    return;
  }
  
  // Copiar datos al array de FFT
  for (int i = 0; i < FFT_SIZE; i++) {
    vReal[i] = buffer[i];
    vImag[i] = 0.0;
  }
  
  // Calcular FFT
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();
  
  // Imprimir solo los bins útiles (FFT_SIZE/2)
  for (int i = 0; i < FFT_SIZE/2; i++) {
    Serial.println(vReal[i], 6);
  }
}

// Función para verificar el botón
void checkButton() {
  int reading = digitalRead(BTN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;

      if (currentButtonState == HIGH) { // Botón presionado
        // Cambiar al siguiente sensor
        currentSensor = (SensorState)((currentSensor + 1) % 4);
        
        // Limpiar buffer
        bufferIndex = 0;
        bufferFull = false;
        memset(buffer, 0, sizeof(buffer));
        
        // Imprimir nombre del sensor actual
        Serial.print("SENSOR:");
        switch (currentSensor) {
          case SENSOR_ABPLLN:
            Serial.println("ABPLLN");
            break;
          case SENSOR_ELV:
            Serial.println("ELV");
            break;
          case SENSOR_SM4291:
            Serial.println("SM4291");
            break;
          case SENSOR_CCDANN600:
            Serial.println("CCDANN600");
            break;
        }
      }
    }
  }
  
  lastButtonState = reading;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  bootM4();
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(BTN, INPUT_PULLDOWN);
  setLed(LED_OFF);
  
  // Inicializar sensores
  initPressureSensor(); // ABPLLN
  sensorELV_begin();    // ELV
  SM_4000_begin();      // SM4291
  CCDANN600MDSA3_begin(); // CCDANN600
  sensorELV_scan();
  
  // Imprimir sensor inicial
  Serial.println("SENSOR:ABPLLN");
  
  // Limpiar buffer
  memset(buffer, 0, sizeof(buffer));
  bufferIndex = 0;
  bufferFull = false;
}

void loop() {
  static unsigned long lastReading = 0;
  unsigned long now = micros();
  
  // Verificar botón
  checkButton();
  
  // Leer sensor cada 500us (2kHz)
  if (now - lastReading >= WAIT_TIME) {
    float pressure = readCurrentSensor();
    addSample(pressure);
    lastReading = now;
  }
  
  // Calcular e imprimir FFT cuando el buffer esté lleno
  if (bufferFull) {
    calculateAndPrintFFT();
    // Resetear buffer para siguiente captura
    bufferIndex = 0;
    bufferFull = false;
  }
}