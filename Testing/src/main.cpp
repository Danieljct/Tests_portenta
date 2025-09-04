#include "shared.h"
#include <Arduino.h>

__attribute__((section(".ccmram"))) volatile SharedBuffer msgBuffer;

void readFromM4() {
    while (msgBuffer.tail != msgBuffer.head) {
        char c = msgBuffer.data[msgBuffer.tail];
        Serial.write(c);
        msgBuffer.tail = (msgBuffer.tail + 1) % BUF_SIZE;
    }
}

void setup() {
    bootM4();
    Serial.begin(115200);
    while(!Serial);
    Serial.println("M7 iniciado!");
}

void loop() {
    readFromM4(); // imprime todo lo que envió M4
    delay(50);    // pequeño retardo
}
