#include <Wire.h>
#include "sensor_elv.h"
#include <HoneywellTruStabilitySPI.h>
#include "analog_sm.h"

#define SLAVE_SELECT_PIN PA_8
TruStabilityPressureSensor sensor( SLAVE_SELECT_PIN, -15.0, 15.0 );


#define P_2SMPP_02  A2
#define N_2SMPP_02  A1
#define VDD         3.3
#define V_offset    -2.5
#define pendiente   0.8378 // V/kPa

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
  analog_SM_read();
  //sensorELV_read();

  int rawVoutPos = analogRead(P_2SMPP_02);
  int rawVoutNeg = analogRead(N_2SMPP_02);
  
  // Convertir los valores raw a voltajes
  float vOutPos = (float)rawVoutPos / 65535.0 * VDD;
  float vOutNeg = (float)rawVoutNeg / 65535.0 * VDD;
  
  // Calcular la diferencia de voltaje
  float vOutDiff = vOutPos - vOutNeg;
  
  // Imprimir el voltaje diferencial
  Serial.print("Voltaje Diferencial: ");
  Serial.print(vOutDiff * 1000.0, 3); // Convertir a mV
  Serial.print(" mV | ");
  
  // NOTA: Para convertir a presion, usa la formula del datasheet con vOutDiff.
  float Presion = (vOutDiff - V_offset)/pendiente;
  Serial.print("Presion: ");
  Serial.print(Presion, 3);
  Serial.println(" kPa  ");

}
