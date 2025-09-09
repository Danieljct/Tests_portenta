#pragma once

const float SM_P_MIN = -500.0; // Presión mínima en mbar
const float SM_P_MAX = 500.0;  // Presión máxima en mbar
const float VDD = 3.3;         // Voltaje de alimentación del sensor (3V o 5V)
const int SM_HGE_pin = PA_0C;  // Pin analógico para leer el sensor

inline void analog_SM_read() {
    int rawADC = analogRead(SM_HGE_pin);
    float voltage = (float)rawADC / 65535.0 * VDD;
    float pressure = (((voltage / VDD) - 0.1) * (SM_P_MAX - SM_P_MIN)) / 0.8 + SM_P_MIN;

    Serial.print("Voltaje: ");
    Serial.print(voltage, 3);
    Serial.print("V | Presion: ");
    Serial.print(pressure, 2);
    Serial.println(" mbar");
}
