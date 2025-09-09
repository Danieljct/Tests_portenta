#include <Wire.h>
//#include "sensor_elv.h"
#include "SM_4000.h"
#include "D_2SMPP_02.h"

void setup() {
    Serial.begin(115200); // start Serial communication
    SPI.begin(); // start SPI communication
    SM_4000_begin();
    while (!Serial) {
        // Esperar a que la comunicación serial esté lista
    }
    //sensorELV_scan();

    // Configurar la resolución de los pines analógicos a 16 bits
    //analogReadResolution(16); // 16 bits para Portenta H7 u otros compatibles
}

void loop() {
    delay(100);
    SM_4000_readAnalog();
    SM_4000_readI2C();
    //D_2SMPP_02_read();
}