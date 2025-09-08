#pragma once

#define P_2SMPP_02  A2
#define N_2SMPP_02  A1

const float V_OFFSET_MV_02 = -2.5;   // Voltaje de offset en mV
const float P_SPAN_02 = 37.0;     // Rango de presión total (P_MAX - P_MIN)
const float V_SPAN_MV_02 = 31.0;   // Voltaje de span en mV
const float SENSOR_SLOPE_02 = V_SPAN_MV_02 / P_SPAN_02; // Pendiente en mV/kPa



inline void D_2SMPP_02_read() {
    int rawVoutPos = analogRead(P_2SMPP_02);
    int rawVoutNeg = analogRead(N_2SMPP_02);

  // 2. Convertir los valores raw a voltajes
  float vOutPos = (float)rawVoutPos / 65535.0 * VDD;
  float vOutNeg = (float)rawVoutNeg / 65535.0 * VDD;
  
  // 3. Calcular la diferencia de voltaje en mV
  float vOutDiff_mv = (vOutPos - vOutNeg) * 1000.0;
  
  // 4. Convertir el voltaje diferencial a presión (kPa)
  float pressure_kPa = (vOutDiff_mv - V_OFFSET_MV_02) / SENSOR_SLOPE_02;

    Serial.print("Voltaje Diferencial 02: ");
    Serial.print(vOutDiff_mv, 3); // mV
    Serial.print(" mV | ");


    Serial.print("Presion: ");
    Serial.print(pressure_kPa, 3);
    Serial.println(" kPa  ");
}
