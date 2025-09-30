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


// Valores de calibración del datasheet (0 a -500 mBar)
const float RAW_MIN = -26214.0; 
const float RAW_MAX = 26214.0;
const float P_MIN_MBAR = 0.0;
const float P_MAX_MBAR = -500.0;
const float RAW_SPAN = RAW_MAX - RAW_MIN; // 52428.0
const float P_SPAN_MBAR = P_MAX_MBAR - P_MIN_MBAR; // -500.0




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

        // Interpreta el valor como entero de 16 bits en complemento a 2
        int16_t rawPressure = (int16_t)((pressHi << 8) | pressLo);

        // Puedes convertir a presión física aquí si lo necesitas:
         float pressure_mbar = ((float)rawPressure - RAW_MIN) * P_SPAN_MBAR / RAW_SPAN + P_MIN_MBAR;

        return pressure_mbar; // O retorna pressure_mbar si quieres la presión física
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
