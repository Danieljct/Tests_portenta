#pragma once
#include <Wire.h>

// Pines y configuración analógica
#define P_2SMPP_02  A2
#define N_2SMPP_02  A1
#define VDD         3.3
#define V_offset    -2.5
#define pendiente   0.8378 // V/kPa

// Pines y configuración I2C
#define I2C3_SCL    D12
#define I2C3_SDA    D11

#ifndef DEV_I2C_DEFINED
#define DEV_I2C_DEFINED
static TwoWire dev_i2c(I2C3_SDA, I2C3_SCL);
#endif

// Direcciones y registros I2C del sensor
const int SENSOR_I2C_ADDRESS_UNPROTECTED = 0x6C;
const int TEMP_REG_ADDR = 0x2E;   // Dirección para leer la temperatura
const int PRESS_REG_ADDR = 0x30;  // Dirección para leer la presión
const int STATUS_REG_ADDR = 0x32; // Dirección para leer el estado

inline void SM_4000_begin() {
    dev_i2c.begin();
    //analogReadResolution(16); // Descomenta si tu plataforma lo requiere
}

inline void SM_4000_readAnalog() {
    int rawVoutPos = analogRead(P_2SMPP_02);
    int rawVoutNeg = analogRead(N_2SMPP_02);

    float vOutPos = (float)rawVoutPos / 65535.0 * VDD;
    float vOutNeg = (float)rawVoutNeg / 65535.0 * VDD;
    float vOutDiff = vOutPos - vOutNeg;

    Serial.print("Voltaje Diferencial: ");
    Serial.print(vOutDiff * 1000.0, 3); // mV
    Serial.print(" mV | ");

    float Presion = (vOutDiff - V_offset)/pendiente;
    Serial.print("Presion: ");
    Serial.print(Presion, 3);
    Serial.println(" kPa  ");
}

inline float SM_4000_readI2C_pressure() {
    dev_i2c.beginTransmission(SENSOR_I2C_ADDRESS_UNPROTECTED);
    dev_i2c.write(PRESS_REG_ADDR); // Dirección de inicio (0x30)
    dev_i2c.endTransmission(false); // Mantener la conexión abierta para la lectura

    dev_i2c.requestFrom(SENSOR_I2C_ADDRESS_UNPROTECTED, 2);

    if (dev_i2c.available() == 2) {
        byte pressLo = dev_i2c.read();
        byte pressHi = dev_i2c.read();

        int rawPressure = (pressHi << 8) | pressLo;
        
        // Convertir valor raw a presión en kPa (ajustar según calibración del sensor)
        // Asumiendo rango de 0-172 kPa y 16 bits de resolución
        float pressure_kPa = ((float)rawPressure / 65535.0) * 172.0;
        
        return pressure_kPa * 10.0; // Convertir a mbar
    } else {
        return -1.0; // Error en la lectura
    }
}

inline void SM_4000_readI2C() {
    dev_i2c.beginTransmission(SENSOR_I2C_ADDRESS_UNPROTECTED);
    dev_i2c.write(TEMP_REG_ADDR); // Dirección de inicio (0x2E)
    dev_i2c.endTransmission(false); // Mantener la conexión abierta para la lectura

    dev_i2c.requestFrom(SENSOR_I2C_ADDRESS_UNPROTECTED, 6);

    if (dev_i2c.available() == 6) {
        byte tempHi = dev_i2c.read();
        byte tempLo = dev_i2c.read();
        byte pressHi = dev_i2c.read();
        byte pressLo = dev_i2c.read();
        byte statusHi = dev_i2c.read();
        byte statusLo = dev_i2c.read();

        int rawTemperature = (tempHi << 8) | tempLo;
        int rawPressure = (pressHi << 8) | pressLo;
        int rawStatus = (statusHi << 8) | statusLo;

        Serial.print("Temperatura Raw: ");
        Serial.print(rawTemperature);
        Serial.print(" | Presion Raw: ");
        Serial.print(rawPressure);
        Serial.print(" | Estado Raw: ");
        Serial.println(rawStatus);
    } else {
        Serial.println("Error: no se pudo leer la informacion del sensor.");
    }
}
