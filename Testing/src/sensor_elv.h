#pragma once
#include <Wire.h>

// Dirección I2C y pines del sensor
#define SENSOR_I2C_ADDR 0x28
#define I2C3_SCL    D12
#define I2C3_SDA    D11

// Rango de salida del sensor (ajusta según tu modelo)
const int OUTPUT_MIN = 1638;  // 10% del rango de 14 bits (2^14 * 0.10)
const int OUTPUT_MAX = 14745; // 90% del rango de 14 bits (2^14 * 0.90)

// Rango de presión del sensor (ajusta según tu modelo ELVH-015D)
const float P_MIN = -1.0;    // Presión mínima en bares
const float P_MAX = 1.0;     // Presión máxima en bares

// Rango de temperatura del sensor
const float T_MIN = -50.0;   // Temperatura mínima en grados C
const float T_MAX = 150.0;   // Temperatura máxima en grados C

// Buffer para almacenar los 4 bytes leídos del sensor
static byte sensorData[4];
static TwoWire dev_i2c(I2C3_SDA, I2C3_SCL);

inline void sensorELV_begin() {
    dev_i2c.begin();
}

inline void sensorELV_scan() {
    Serial.println("Escaneando dispositivos I2C3...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        dev_i2c.beginTransmission(addr);
        if (dev_i2c.endTransmission() == 0) {
            Serial.print("Dispositivo encontrado en 0x");
            Serial.println(addr, HEX);
        }
        delay(2);
    }
    Serial.println("Escaneo I2C3 finalizado.");
}

inline float sensorELV_read(bool crudo = false) {
    dev_i2c.requestFrom(SENSOR_I2C_ADDR, 4);
    if (dev_i2c.available() == 4) {
        for (int i = 0; i < 4; i++) {
            sensorData[i] = dev_i2c.read();
        }
        if (crudo) {
            Serial.print("I2C3 bytes: ");
            for (int i = 0; i < 4; i++) {
                Serial.print("0x");
                Serial.print(sensorData[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        } else {
            int pressure_raw;
            int temperature_raw;
            int status;
            status = (sensorData[0] >> 6) & 0b11;
            pressure_raw = ((sensorData[0] & 0x3F) << 8) | sensorData[1];
            temperature_raw = ((sensorData[2] << 3) | (sensorData[3] >> 5));
            float pressure_mbar = ((float)pressure_raw - OUTPUT_MIN) * (P_MAX - P_MIN) / (OUTPUT_MAX - OUTPUT_MIN) + P_MIN;
            float temperature_c = ((float)temperature_raw / 2047.0) * (T_MAX - T_MIN) + T_MIN;
            Serial.print("Estado: ");
            Serial.print(status);
            Serial.print(" | Presion Raw: ");
            Serial.print(pressure_raw);
            Serial.print(" | Presion: ");
            Serial.print(pressure_mbar, 2);
            Serial.print(" bares | Temp Raw: ");
            Serial.print(temperature_raw);
            Serial.print(" | Temperatura: ");
            Serial.print(temperature_c, 2);
            Serial.println(" C");
            return pressure_mbar;
        }
    } else {
        Serial.println("No se recibieron 4 bytes del sensor I2C.");
        return NAN;
    }

}
