#include "window_analysis.h"

// Buffer circular para la ventana de 50 muestras
int windowBuffer[WINDOW_SIZE];
size_t windowIndex = 0;
bool windowFilled = false;

// Estados de LED
LedState ledState = LED_OFF;
unsigned long greenLedStart = 0;

float calcularCurtosis(const int* data, size_t n) {
  if (n < 4) return NAN;
  float mean = 0, m2 = 0, m4 = 0;
  for (size_t i = 0; i < n; i++) mean += data[i];
  mean /= n;
  for (size_t i = 0; i < n; i++) {
    float d = data[i] - mean;
    m2 += d * d;
    m4 += d * d * d * d;
  }
  m2 /= n;
  m4 /= n;
  if (m2 == 0) return NAN;
  float kurtosis = m4 / (m2 * m2);
  return kurtosis;
}

void setLed(LedState state) {
  // Apaga todos
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  switch (state) {
    case LED_GREEN_:
      digitalWrite(LEDG, LOW);
      break;
    case LED_YELLOW_:
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDR, LOW);
      break;
    case LED_RED_:
      digitalWrite(LEDR, LOW);
      break;
    default:
      break;
  }
}

void addSampleToWindow(int sample) {
  windowBuffer[windowIndex++] = sample;
  if (windowIndex >= WINDOW_SIZE) {
    windowIndex = 0;
    windowFilled = true;
  }
}

void processWindowAnalysis() {
  if (windowFilled) {
    float kurt = calcularCurtosis(windowBuffer, WINDOW_SIZE);
    Serial.println(kurt, 6);

    // Si la curtosis supera 12, prende verde por 1 segundo
    if (kurt > 12) {
      setLed(LED_GREEN_);
      ledState = LED_GREEN_;
      greenLedStart = millis();
    } else if (ledState != LED_GREEN_) {
      if (kurt < 4) {
        setLed(LED_RED_);
        ledState = LED_RED_;
      } else {
        setLed(LED_YELLOW_);
        ledState = LED_YELLOW_;
      }
    }
  }

  // Apaga el verde después de 1 segundo y vuelve a mostrar el color según la curtosis actual
  if (ledState == LED_GREEN_ && (millis() - greenLedStart > 1000)) {
    // Recalcula la curtosis para decidir el color actual
    float kurt = calcularCurtosis(windowBuffer, WINDOW_SIZE);
    if (kurt < 4) {
      setLed(LED_RED_);
      ledState = LED_RED_;
    } else if (kurt <= 12) {
      setLed(LED_YELLOW_);
      ledState = LED_YELLOW_;
    } else {
      // Si sigue siendo >12, vuelve a prender verde y reinicia el timer
      setLed(LED_GREEN_);
      greenLedStart = millis();
    }
  }
}