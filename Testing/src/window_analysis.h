#ifndef WINDOW_ANALYSIS_H
#define WINDOW_ANALYSIS_H

#include <Arduino.h>

#define WINDOW_SIZE 50

// Buffer circular para la ventana de 50 muestras
extern int windowBuffer[WINDOW_SIZE];
extern size_t windowIndex;
extern bool windowFilled;

// Estados de LED
enum LedState { LED_OFF, LED_GREEN_, LED_YELLOW_, LED_RED_ };
extern LedState ledState;
extern unsigned long greenLedStart;

// Función para calcular curtosis
float calcularCurtosis(const int* data, size_t n);

// Función para configurar LED
void setLed(LedState state);

// Función para agregar muestra a la ventana
void addSampleToWindow(int sample);

// Función para procesar análisis de ventana y control de LEDs
void processWindowAnalysis();

#endif