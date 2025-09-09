#include <Wire.h>
#include <SPI.h>
//#include "sensor_elv.h"
#include "SM_4000.h"
#include "D_2SMPP_02.h"
#include "CCDANN600MDSA3.h"


void setup() {
    Serial.begin(115200); // start Serial communication

        while (!Serial) {
        // Esperar a que la comunicación serial esté lista
    }
    CCDANN600MDSA3_begin();

    // Configurar la resolución de los pines analógicos a 16 bits
    //analogReadResolution(16); // 16 bits para Portenta H7 u otros compatibles
}

void loop() {
    delay(100);
    CCDANN600MDSA3_read();
    delay(2000);
}