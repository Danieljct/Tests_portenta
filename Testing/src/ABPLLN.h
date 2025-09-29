#ifndef ABPLLN_H
#define ABPLLN_H

#include <Arduino.h>

// Configuración del sensor de presión I2C
#define PRESSURE_SENSOR_ADDR 0x08
#define PRESSURE_RANGE_MBAR 600.0f
#define PRESSURE_MIN_PERCENT 10.0f
#define PRESSURE_MAX_PERCENT 90.0f
#define PRESSURE_RESOLUTION_BITS 14
#define PRESSURE_MAX_COUNTS ((1 << PRESSURE_RESOLUTION_BITS) - 1)

// Variables globales
extern float currentPressure;
extern uint16_t rawPressureData;
extern bool sensorConnected;

// Declaraciones de funciones
void initPressureSensor();
uint16_t readRawPressure();
float convertToPressure(uint16_t rawValue);
bool readPressureSensor();
void displayPressureInfo();
void scanI2CDevices();

#endif
