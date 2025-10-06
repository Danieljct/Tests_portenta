#include <Arduino.h>
#include <mbed.h>
#include <rtos.h>
#include "window_analysis.h"
#include "ABPLLN.h"
#include "sensor_elv.h"
#include "SM_4000.h"
#include "CCDANN600MDSA3.h"

#define WAIT_TIME 500

// Enum para los estados de sensores
enum SensorState {
  SENSOR_ABPLLN = 0,
  SENSOR_ELV = 1,
  SENSOR_SM4291 = 2,
  SENSOR_CCDANN600 = 3
};

// Variables para control del botón y toggle
SensorState currentSensor = SENSOR_ABPLLN;
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
#define BTN PC_7

// Estructuras para almacenar las lecturas de cada sensor
struct SensorData {
  float pressure;
  uint32_t timestamp;
  bool valid;
};

// Variables globales para los datos de cada sensor
SensorData abplln_data = {0.0, 0, false};
SensorData elv_data = {0.0, 0, false};
SensorData sm4291_data = {0.0, 0, false};
SensorData ccdann600_data = {0.0, 0, false};

// Mutexes para proteger acceso a los datos
rtos::Mutex abplln_mutex;
rtos::Mutex elv_mutex;
rtos::Mutex sm4291_mutex;
rtos::Mutex ccdann600_mutex;
rtos::Mutex serial_mutex;

// Threads para cada sensor
rtos::Thread abplln_thread;
rtos::Thread elv_thread;
rtos::Thread sm4291_thread;
rtos::Thread ccdann600_thread;

// Funciones de los threads para cada sensor

// Thread para sensor ABPLLN - 2kHz (500us entre lecturas)
void abplln_thread_function() {
  while (true) {
    SensorData data;
    data.timestamp = millis();
    
    if (readPressureSensor()) {
      data.pressure = currentPressure;
      data.valid = true;
    } else {
      data.valid = false;
    }
    
    // Actualizar datos protegido por mutex
    abplln_mutex.lock();
    abplln_data = data;
    abplln_mutex.unlock();
    
    // Esperar 500us para conseguir 2kHz
    wait_us(WAIT_TIME);
  }
}

// Thread para sensor ELV - 2kHz (500us entre lecturas)
void elv_thread_function() {
  while (true) {
    SensorData data;
    data.timestamp = millis();
    
    int pressureRaw = sensorELV_read(false, false);
    if (!isnan(pressureRaw)) {
      data.pressure = pressure_raw_to_pressure_mbar(pressureRaw) * 1000;
      data.valid = true;
    } else {
      data.valid = false;
    }
    
    // Actualizar datos protegido por mutex
    elv_mutex.lock();
    elv_data = data;
    elv_mutex.unlock();
    
    // Esperar 500us para conseguir 2kHz
    wait_us(WAIT_TIME);
  }
}

// Thread para sensor SM4291 - 2kHz (500us entre lecturas)
void sm4291_thread_function() {
  while (true) {
    SensorData data;
    data.timestamp = millis();
    
    float pressureMbar = SM_4000_readI2C_pressure();
    if (pressureMbar != -1.0) {
      data.pressure = pressureMbar;
      data.valid = true;
    } else {
      data.valid = false;
    }
    
    // Actualizar datos protegido por mutex
    sm4291_mutex.lock();
    sm4291_data = data;
    sm4291_mutex.unlock();
    
    // Esperar 500us para conseguir 2kHz
    wait_us(WAIT_TIME);
  }
}

// Thread para sensor CCDANN600 - 2kHz (500us entre lecturas)
void ccdann600_thread_function() {
  while (true) {
    SensorData data;
    data.timestamp = millis();
    
    data.pressure = CCDANN600MDSA3_read();
    data.valid = true; // Este sensor siempre devuelve un valor
    
    // Actualizar datos protegido por mutex
    ccdann600_mutex.lock();
    ccdann600_data = data;
    ccdann600_mutex.unlock();
    
    // Esperar 500us para conseguir 2kHz
    wait_us(WAIT_TIME);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("=== SISTEMA DE LECTURA DE PRESIÓN MULTIHILO ===");
  Serial.println("Usando mbedOS - 4 sensores a 2kHz cada uno");
  
  bootM4();

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(BTN, INPUT_PULLDOWN);
  setLed(LED_OFF);
  
  // Inicializar los cuatro sensores
  initPressureSensor(); // ABPLLN
  sensorELV_begin();    // ELV
  SM_4000_begin();      // SM4291 (usando SM_4000)
  CCDANN600MDSA3_begin(); // CCDANN600
  sensorELV_scan();     // Escanear dispositivos I2C3

  Serial.println("Inicializando threads de sensores...");
  
  // Iniciar los threads para cada sensor
  abplln_thread.start(abplln_thread_function);
  elv_thread.start(elv_thread_function);
  sm4291_thread.start(sm4291_thread_function);
  ccdann600_thread.start(ccdann600_thread_function);
  
  Serial.println("Todos los threads iniciados exitosamente");
  Serial.println("Presiona el botón para cambiar entre sensores en display");
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
        currentSensor = (SensorState)((currentSensor + 1) % 4);
        
        serial_mutex.lock();
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
        }
        Serial.println("========================");
        serial_mutex.unlock();
      }
    }
  }
  
  lastButtonState = reading;
}

void loop() {
  static unsigned long lastDisplay = 0;
  unsigned long now = millis();
  
  // Verificar botón
  checkButton();
  
  // Mostrar datos cada 100ms (10 Hz) para no saturar el serial
  if (now - lastDisplay >= 100) {
    SensorData data;
    bool hasData = false;
    
    // Leer los datos del sensor seleccionado de forma thread-safe
    switch (currentSensor) {
      case SENSOR_ABPLLN:
        abplln_mutex.lock();
        data = abplln_data;
        abplln_mutex.unlock();
        hasData = data.valid;
        break;
        
      case SENSOR_ELV:
        elv_mutex.lock();
        data = elv_data;
        elv_mutex.unlock();
        hasData = data.valid;
        break;
        
      case SENSOR_SM4291:
        sm4291_mutex.lock();
        data = sm4291_data;
        sm4291_mutex.unlock();
        hasData = data.valid;
        break;
        
      case SENSOR_CCDANN600:
        ccdann600_mutex.lock();
        data = ccdann600_data;
        ccdann600_mutex.unlock();
        hasData = data.valid;
        break;
    }
    
    // Mostrar datos del sensor seleccionado de forma thread-safe
    if (hasData) {
      serial_mutex.lock();
      switch (currentSensor) {
        case SENSOR_ABPLLN:
          Serial.print("ABPLLN: ");
          break;
        case SENSOR_ELV:
          Serial.print("ELV: ");
          break;
        case SENSOR_SM4291:
          Serial.print("SM4291: ");
          break;
        case SENSOR_CCDANN600:
          Serial.print("CCDANN600: ");
          break;
      }
      Serial.print(data.pressure, 2);
      Serial.print(" mbar [");
      Serial.print(data.timestamp);
      Serial.println(" ms]");
      serial_mutex.unlock();
    }
    
    lastDisplay = now;
  }
  
  delay(10);
}
