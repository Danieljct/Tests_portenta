/*
  Sensor SM4291 con indicador LED RGB
  SENSOR DE SUCCIÓN: 0 a -500 mbar
  USANDO digitalWrite() para compatibilidad con Portenta H7
  
  Códigos de LED:
  - Azul claro: Succión baja (0 a -50 mbar)
  - Verde: Succión media (-50 a -200 mbar) 
  - Cyan: Succión alta (-200 a -500 mbar)
  - Rojo: Error persistente en la lectura
  - Naranja: Errores ocasionales  
  - Amarillo: Valor fuera de rango de succión
  - Azul: Inicializando
*/

#include <Arduino.h>
#include "SM_4000.h"
#include "portenta_rgb.h"

// Estas definiciones deben estar antes del include
#define _TIMERINTERRUPT_LOGLEVEL_     0
#include "Portenta_H7_TimerInterrupt.h"

// Variables para la interrupción
volatile bool readSensor = false;

// Init timer TIM12
Portenta_H7_Timer ITimer(TIM12);

// Crear instancia del LED RGB
PortentaRGB rgb;

// Variables para análisis del sensor
float lastSuction = 0.0;
unsigned long lastReadTime = 0;
int consecutiveErrors = 0;
int readingCount = 0;

// Rangos de succión para indicadores (valores negativos)
const float SUCTION_MIN_NORMAL = 0.0;      // mbar (sin succión)
const float SUCTION_MAX_NORMAL = -500.0;   // mbar (succión máxima)
const float SUCTION_LOW_MIN = 0.0;         // mbar (succión baja/sin succión)
const float SUCTION_LOW_MAX = -50.0;       // mbar
const float SUCTION_MEDIUM_MIN = -50.0;    // mbar (succión media)
const float SUCTION_MEDIUM_MAX = -200.0;   // mbar
const float SUCTION_HIGH_MIN = -200.0;     // mbar (succión alta)
const float SUCTION_HIGH_MAX = -500.0;     // mbar (succión máxima)

// Función de callback de la interrupción del timer
void TimerHandler() {
  readSensor = true;
}

void updateLEDStatus(float suction) {
  if (suction == -1.0) {
    // Error en la lectura
    consecutiveErrors++;
    if (consecutiveErrors > 5) {
      rgb.red();  // Rojo para errores persistentes
    } else {
      // Parpadeo naranja para errores ocasionales (simulado con digital)
      static bool blinkState = false;
      blinkState = !blinkState;
      if (blinkState) {
        rgb.yellow(); // Amarillo como "naranja" digital
      } else {
        rgb.off();
      }
    }
  } else {
    consecutiveErrors = 0; // Reset contador de errores
    
    // Verificar rangos de succión
    if (suction > SUCTION_MIN_NORMAL || suction < SUCTION_MAX_NORMAL) {
      // Valor fuera del rango de succión normal (0 a -500 mbar)
      rgb.yellow();
    } else if (suction >= SUCTION_LOW_MIN && suction > SUCTION_LOW_MAX) {
      // Succión baja o sin succión (0 a -50 mbar)
      rgb.blue();  // Azul para succión baja (azul claro no disponible en digital)
    } else if (suction <= SUCTION_LOW_MAX && suction > SUCTION_MEDIUM_MAX) {
      // Succión media (-50 a -200 mbar)
      rgb.green();      // Verde para succión normal/media
    } else if (suction <= SUCTION_MEDIUM_MAX && suction >= SUCTION_HIGH_MAX) {
      // Succión alta (-200 a -500 mbar)
      rgb.cyan();       // Cyan para succión alta (óptima)
    } else {
      // Valor por defecto
      rgb.magenta();    // Magenta para casos no definidos
    }
  }
}

void showStartupSequence() {
  Serial.println("Iniciando secuencia de startup...");
  
  // Secuencia de colores de inicio (simplificada para digital)
  rgb.blue();
  delay(500);
  rgb.magenta();
  delay(500);
  rgb.green();
  delay(500);
  
  // Parpadeo para indicar listo
  rgb.blink(rgb.COLOR_WHITE, 200, 200, 3);
  
  Serial.println("Sistema listo!");
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  // Inicializar LED RGB
  rgb.begin();
  rgb.blue(); // Azul durante inicialización
  
  bootM4();
  
  // Inicializar solo sensor SM4291
  SM_4000_begin();
  
  Serial.println("=== SENSOR SM4291 SUCCIÓN con LED RGB - 2kHz ===");
  Serial.println("USANDO digitalWrite() - Compatible con Portenta H7");
  
  // Configurar timer para 2kHz (500us)
  if (ITimer.attachInterruptInterval(500, TimerHandler)) {
    Serial.println("Timer configurado correctamente a 2kHz (500us)");
    rgb.green(); // Verde si timer OK
    delay(1000);
  } else {
    Serial.println("Error: No se pudo configurar el timer");
    rgb.red(); // Rojo si error en timer
    delay(2000);
  }
  
  // Mostrar secuencia de inicio
  showStartupSequence();
  
}

void loop() {
  // Verificar si el timer disparó la interrupción
  if (readSensor) {
    readSensor = false; // Limpiar flag
    
    float suctionMbar = SM_4000_readI2C_pressure();
    readingCount++;
    
    // Actualizar LED según el valor leído
    updateLEDStatus(suctionMbar);
    
    if (suctionMbar != -1.0) {
      Serial.print(suctionMbar, 6);
      
      // Agregar información de estado cada 1000 lecturas
      if (readingCount % 1000 == 0) {
        Serial.print(" [Lecturas: ");
        Serial.print(readingCount);
        Serial.print(", Errores: ");
        Serial.print(consecutiveErrors);
        
        // Mostrar nivel de succión
        if (suctionMbar >= -50.0) {
          Serial.print(", Nivel: BAJO");
        } else if (suctionMbar >= -200.0) {
          Serial.print(", Nivel: MEDIO");
        } else if (suctionMbar >= -500.0) {
          Serial.print(", Nivel: ALTO");
        } else {
          Serial.print(", Nivel: EXTREMO");
        }
        Serial.print("]");
      }
      Serial.println();
      
      lastSuction = suctionMbar;
      lastReadTime = millis();
    } else {
      Serial.println("ERROR");
    }
    
  }
}