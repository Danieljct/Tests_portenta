#include <Arduino.h>
#include "../../Testing/src/shared.h"

__attribute__((section(".ccmram"))) volatile SharedBuffer msgBuffer;

void sendToM7(const char* msg) {
    int i = 0;
    while (msg[i] != '\0') {
        int next = (msgBuffer.head + 1) % BUF_SIZE;
        if (next == msgBuffer.tail) break; // buffer lleno
        msgBuffer.data[msgBuffer.head] = msg[i];
        msgBuffer.head = next;
        i++;
    }
}

void setup() {
}

bool ledState = false;

void loop() {

    ledState = !ledState;
    digitalWrite(LEDB, ledState ? HIGH : LOW);

    sendToM7("Hola desde M4\n");
    delay(500);

}
