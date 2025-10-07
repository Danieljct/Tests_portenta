#ifndef PORTENTA_RGB_H
#define PORTENTA_RGB_H

#include <Arduino.h>

// Definición de pines RGB del Portenta H7
#define LED_RED    LEDR    // Pin del LED rojo
#define LED_GREEN  LEDG    // Pin del LED verde  
#define LED_BLUE   LEDB    // Pin del LED azul

// Estructura para colores RGB
struct RGBColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

// Clase para manejar el LED RGB del Portenta H7 (Versión reducida)
class PortentaRGB {
private:
    bool inverted;  // Los LEDs del Portenta son lógica invertida (LOW = encendido)
    
public:
    // Constructor
    PortentaRGB(bool invertedLogic = true);
    
    // Inicialización
    void begin();
    
    // Control básico
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void setColor(RGBColor color);
    void off();
    
    // Solo colores básicos
    void red();
    void green();
    void blue();
    void white();
    void yellow();
    void cyan();
    void magenta();
    void black();
    
    // Solo efecto básico
    void blink(RGBColor color, uint16_t onTime = 500, uint16_t offTime = 500, uint8_t times = 3);
    
    // Solo utilidad básica
    RGBColor createColor(uint8_t red, uint8_t green, uint8_t blue);
    
    // Solo colores básicos como constantes
    static const RGBColor COLOR_RED;
    static const RGBColor COLOR_GREEN;
    static const RGBColor COLOR_BLUE;
    static const RGBColor COLOR_WHITE;
    static const RGBColor COLOR_BLACK;
    static const RGBColor COLOR_YELLOW;
    static const RGBColor COLOR_CYAN;
    static const RGBColor COLOR_MAGENTA;
};

#endif // PORTENTA_RGB_H