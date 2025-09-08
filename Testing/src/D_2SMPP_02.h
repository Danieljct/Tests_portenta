#pragma once

#define P_2SMPP_02  A2
#define N_2SMPP_02  A1
#define VDD         3.3
#define V_offset    -2.5
#define pendiente   0.8378 // V/kPa

inline void D_2SMPP_02_read() {
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
