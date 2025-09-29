#include "ABPLLN.h"
#include <Wire.h>

// Variables globales para el sensor
float currentPressure = 0.0f;
uint16_t rawPressureData = 0;
bool sensorConnected = false;

// Función para inicializar el sensor de presión
void initPressureSensor() {
  Serial.println("Inicializando sensor de presión I2C...");
  
  Wire.begin();
  Wire.setClock(100000); // 100kHz para mayor estabilidad
  
  // Verificar si el sensor responde
  Wire.beginTransmission(PRESSURE_SENSOR_ADDR);
  uint8_t error = Wire.endTransmission();
  
  if (error == 0) {
    sensorConnected = true;
    Serial.println("✓ Sensor de presión detectado en 0x08");
  } else {
    sensorConnected = false;
    Serial.print("✗ Error conectando al sensor: ");
    Serial.println(error);
  }
}

// Función para leer datos raw del sensor (14 bits)
uint16_t readRawPressure() {
  if (!sensorConnected) return 0;
  
  Wire.requestFrom(PRESSURE_SENSOR_ADDR, 2); // Solicitar 2 bytes
  
  if (Wire.available() >= 2) {
    // Leer los 2 bytes (MSB primero para 14 bits)
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    
    // Combinar en 14 bits (los 2 bits superiores del MSB pueden ser status)
    uint16_t rawValue = ((uint16_t)msb << 8) | lsb;
    
    // Extraer solo los 14 bits de datos (mask para eliminar bits de status si los hay)
    rawValue = rawValue & 0x3FFF; // Mantener solo 14 bits
    
    return rawValue;
  }
  
  return 0;
}

// Función para convertir valor raw a presión en mbar
float convertToPressure(uint16_t rawValue) {
  // Calcular el rango útil (10% a 90% del rango total)
  float usefulRange = PRESSURE_MAX_PERCENT - PRESSURE_MIN_PERCENT; // 80%
  float countsPerPercent = PRESSURE_MAX_COUNTS / 100.0f;
  
  // Calcular los counts correspondientes al 10% y 90%
  uint16_t minCounts = (uint16_t)(PRESSURE_MIN_PERCENT * countsPerPercent);
  uint16_t maxCounts = (uint16_t)(PRESSURE_MAX_PERCENT * countsPerPercent);
  
  // Verificar si el valor está en el rango válido
  if (rawValue < minCounts) {
    return 0.0f; // Por debajo del rango mínimo
  }
  if (rawValue > maxCounts) {
    return PRESSURE_RANGE_MBAR; // Por encima del rango máximo
  }
  
  // Conversión lineal del rango útil (10-90%) a 0-600 mbar
  float normalizedValue = (float)(rawValue - minCounts) / (float)(maxCounts - minCounts);
  float pressure = normalizedValue * PRESSURE_RANGE_MBAR;
  
  return pressure;
}

// Función para leer y procesar la presión
bool readPressureSensor() {
  rawPressureData = readRawPressure();
  
  if (rawPressureData == 0 && !sensorConnected) {
    return false;
  }
  
  currentPressure = convertToPressure(rawPressureData);
  return true;
}

// Función para mostrar información detallada del sensor
void displayPressureInfo() {
  Serial.println("=== INFORMACIÓN DEL SENSOR DE PRESIÓN ===");
  Serial.print("Estado: ");
  Serial.println(sensorConnected ? "Conectado" : "Desconectado");
  
  if (sensorConnected) {
    Serial.print("Valor RAW: ");
    Serial.print(rawPressureData);
    Serial.print(" (0x");
    Serial.print(rawPressureData, HEX);
    Serial.println(")");
    
    Serial.print("Presión: ");
    Serial.print(currentPressure, 2);
    Serial.println(" mbar");
    
    // Mostrar porcentaje del rango
    float percentage = (currentPressure / PRESSURE_RANGE_MBAR) * 100.0f;
    Serial.print("Porcentaje del rango: ");
    Serial.print(percentage, 1);
    Serial.println("%");
    
    // Información técnica
    Serial.print("Resolución: ");
    Serial.print(PRESSURE_RESOLUTION_BITS);
    Serial.println(" bits");
    Serial.print("Rango: 0-");
    Serial.print(PRESSURE_RANGE_MBAR, 0);
    Serial.println(" mbar");
    Serial.print("Rango útil: ");
    Serial.print(PRESSURE_MIN_PERCENT, 0);
    Serial.print("%-");
    Serial.print(PRESSURE_MAX_PERCENT, 0);
    Serial.println("%");
  }
  Serial.println("=========================================");
}

// Función para escanear dispositivos I2C
void scanI2CDevices() {
  Serial.println("Escaneando dispositivos I2C...");
  
  int deviceCount = 0;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Dispositivo encontrado en 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      if (address == PRESSURE_SENSOR_ADDR) {
        Serial.print(" (Sensor de presión)");
      }
      Serial.println();
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No se encontraron dispositivos I2C");
  } else {
    Serial.print("Total de dispositivos encontrados: ");
    Serial.println(deviceCount);
  }
  Serial.println();
}
