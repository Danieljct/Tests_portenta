#include <Wire.h>
#include "sensor_elv.h"
#include <HoneywellTruStabilitySPI.h>
#include "analog_sm.h"
#include "D_2SMPP_02.h"

#define SLAVE_SELECT_PIN PA_8
TruStabilityPressureSensor sensor( SLAVE_SELECT_PIN, -15.0, 15.0 );



// --- Constantes del Sensor 2SMPP-03 ---

const float V_OFFSET_MV_03 = -2.5;   // Voltaje de offset en mV
const float P_SPAN_03 = 100.0;     // Rango de presión total (P_MAX - P_MIN)
const float V_SPAN_MV_03 = 42.0;   // Voltaje de span en mV
const float SENSOR_SLOPE_03 = V_SPAN_MV_03 / P_SPAN_03; // Pendiente en mV/kPa

// --- Pines de Conexión del Portenta ---
const int VOUT_POS_PIN = A2; // Pin para Vout(+)
const int VOUT_NEG_PIN = A1; // Pin para Vout(-)


void setup() {
    Serial.begin(115200); // start Serial communication
    SPI.begin(); // start SPI communication
    sensor.begin(); // run sensor initialization
    sensorELV_begin();
    while (!Serial) {
        // Esperar a que la comunicación serial esté lista
    }
    //sensorELV_scan();

    // Configurar la resolución de los pines analógicos a 16 bits
    analogReadResolution(16); // 16 bits para Portenta H7 u otros compatibles
}

void loop() {
  // the sensor returns 0 when new data is ready
  /*
  digitalWrite(SLAVE_SELECT_PIN, LOW);
  delay(1);
  int status = sensor.readSensor();
  delay(1);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
  if( status == 0 ) {
    Serial.print( "temp [C]: " );
    Serial.print( sensor.temperature() );
    Serial.print( "\t pressure [psi]: " );
    Serial.println( sensor.pressure() );
  }
  else{
    Serial.println("Status: " + String(status));
    Serial.print("Buffer: ");
    for (int i = 0; i < 4; i++) {
      Serial.print("0x");
      Serial.print(sensor.readBuff()[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
*/
  delay( 100 ); // Slow down sampling to 10 Hz. This is just a test.
  //analog_SM_read();
  //sensorELV_read();
//  D_2SMPP_02_read();
    // 1. Leer los valores raw del ADC
  int rawVoutPos = analogRead(VOUT_POS_PIN);
  int rawVoutNeg = analogRead(VOUT_NEG_PIN);
  
  // 2. Convertir los valores raw a voltajes
  float vOutPos = (float)rawVoutPos / 65535.0 * VDD;
  float vOutNeg = (float)rawVoutNeg / 65535.0 * VDD;
  
  // 3. Calcular la diferencia de voltaje en mV
  float vOutDiff_mv = (vOutPos - vOutNeg) * 1000.0;
  
  // 4. Convertir el voltaje diferencial a presión (kPa)
  float pressure_kPa = (vOutDiff_mv - V_OFFSET_MV_03) / SENSOR_SLOPE_03;

  // --- Imprimir los resultados ---
  Serial.print("Voltaje Diferencial 03: ");
  Serial.print(vOutDiff_mv, 2);
  Serial.print(" mV | Presion: ");
  Serial.print(pressure_kPa, 2);
  Serial.println(" kPa");
  
  delay(100);
}