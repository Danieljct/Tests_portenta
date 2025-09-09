#pragma once
#include <Arduino.h>
#include <SPI.h>

// Cambia el pin CS según tu conexión
#ifndef CCDANN600MDSA3_CS_PIN
#define CCDANN600MDSA3_CS_PIN PIN_SPI_SS
#endif

// Constantes del sensor (ajusta si tu modelo es diferente)
constexpr float CCDANN600MDSA3_P_MIN = -600.0; // mbar
constexpr float CCDANN600MDSA3_P_MAX = 600.0;  // mbar
constexpr int CCDANN600MDSA3_OUTPUT_MIN = 1638;   // 10% de 2^12
constexpr int CCDANN600MDSA3_OUTPUT_MAX = 14745;  // 90% de 2^12

inline void CCDANN600MDSA3_begin() {
    pinMode(CCDANN600MDSA3_CS_PIN, OUTPUT);
    digitalWrite(CCDANN600MDSA3_CS_PIN, HIGH);
    SPI.begin();
}

inline void CCDANN600MDSA3_read() {
    SPI.beginTransaction(SPISettings(750000, MSBFIRST, SPI_MODE3));
    digitalWrite(CCDANN600MDSA3_CS_PIN, LOW);
    delayMicroseconds(2); // tCSS típico

    uint8_t b0 = SPI.transfer(0x00);
    uint8_t b1 = SPI.transfer(0x00);

    delayMicroseconds(1); // tCSH corto
    digitalWrite(CCDANN600MDSA3_CS_PIN, HIGH);
    SPI.endTransaction();

    uint16_t w = (uint16_t(b0) << 8) | b1;
    uint8_t status = (w >> 14) & 0x03; // 2 bits de estado
    uint16_t raw = (w >> 2) & 0x0FFF;  // 12 bits de presión (bits 13..2)

    // Conversión a presión física (mbar)
    float pressure = ((float)raw - CCDANN600MDSA3_OUTPUT_MIN) * (CCDANN600MDSA3_P_MAX - CCDANN600MDSA3_P_MIN) /
                     (CCDANN600MDSA3_OUTPUT_MAX - CCDANN600MDSA3_OUTPUT_MIN) + CCDANN600MDSA3_P_MIN;

    Serial.print("Status: ");
    switch (status) {
        case 0: Serial.print("OK"); break;
        case 1: Serial.print("Comando"); break;
        case 2: Serial.print("Obsoleto"); break;
        case 3: Serial.print("Error"); break;
    }
    Serial.print(" | Raw: ");
    Serial.print(raw);
    Serial.print(" | Presion: ");
    Serial.print(pressure, 2);
    Serial.println(" mbar");
}
