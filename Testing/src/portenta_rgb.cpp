#include "portenta_rgb.h"

// Solo colores básicos necesarios
const RGBColor PortentaRGB::COLOR_RED = {255, 0, 0};
const RGBColor PortentaRGB::COLOR_GREEN = {0, 255, 0};
const RGBColor PortentaRGB::COLOR_BLUE = {0, 0, 255};
const RGBColor PortentaRGB::COLOR_WHITE = {255, 255, 255};
const RGBColor PortentaRGB::COLOR_BLACK = {0, 0, 0};
const RGBColor PortentaRGB::COLOR_YELLOW = {255, 255, 0};
const RGBColor PortentaRGB::COLOR_CYAN = {0, 255, 255};
const RGBColor PortentaRGB::COLOR_MAGENTA = {255, 0, 255};

// Constructor
PortentaRGB::PortentaRGB(bool invertedLogic) {
    inverted = invertedLogic;
}

// Inicialización
void PortentaRGB::begin() {
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    off(); // Empezar con LEDs apagados
}

// Control básico
void PortentaRGB::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    bool redOn = (red > 127);
    bool greenOn = (green > 127);
    bool blueOn = (blue > 127);
    
    if (inverted) {
        // Lógica invertida para Portenta H7 (LOW = encendido)
        digitalWrite(LED_RED, redOn ? LOW : HIGH);
        digitalWrite(LED_GREEN, greenOn ? LOW : HIGH);
        digitalWrite(LED_BLUE, blueOn ? LOW : HIGH);
    } else {
        digitalWrite(LED_RED, redOn ? HIGH : LOW);
        digitalWrite(LED_GREEN, greenOn ? HIGH : LOW);
        digitalWrite(LED_BLUE, blueOn ? HIGH : LOW);
    }
}

void PortentaRGB::setColor(RGBColor color) {
    setColor(color.red, color.green, color.blue);
}

void PortentaRGB::off() {
    setColor(0, 0, 0);
}

// Solo colores básicos
void PortentaRGB::red() { setColor(COLOR_RED); }
void PortentaRGB::green() { setColor(COLOR_GREEN); }
void PortentaRGB::blue() { setColor(COLOR_BLUE); }
void PortentaRGB::white() { setColor(COLOR_WHITE); }
void PortentaRGB::yellow() { setColor(COLOR_YELLOW); }
void PortentaRGB::cyan() { setColor(COLOR_CYAN); }
void PortentaRGB::magenta() { setColor(COLOR_MAGENTA); }
void PortentaRGB::black() { setColor(COLOR_BLACK); }

// Solo efecto de parpadeo básico
void PortentaRGB::blink(RGBColor color, uint16_t onTime, uint16_t offTime, uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        setColor(color);
        delay(onTime);
        off();
        if (i < times - 1) {
            delay(offTime);
        }
    }
}

// Utilidad básica
RGBColor PortentaRGB::createColor(uint8_t red, uint8_t green, uint8_t blue) {
    RGBColor color = {red, green, blue};
    return color;
}