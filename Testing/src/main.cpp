#include <SPI.h>

// Definición de pines para la comunicación SPI
// Puedes usar cualquier pin digital para el Slave Select (SS)
const int SS_PIN = PA_8;

// Constantes para el cálculo de presión (del datasheet del sensor SSC)
// Estas constantes son específicas para el modelo SSCDANN600MDSA3-ND
const float P_MAX = 600.0;     // Presión máxima en mbar
const float P_MIN = -600.0;    // Presión mínima en mbar
const int OUTPUT_MAX = 14745;  // 90% del rango de 14 bits (2^14 * 0.90)
const int OUTPUT_MIN = 1638;   // 10% del rango de 14 bits (2^14 * 0.10)

void setup() {
  // Iniciar la comunicación serial a 115200 baudios
  Serial.begin(115200);
  while (!Serial) {
    // Esperar a que la comunicación serial esté lista
  }
  Serial.println("Inicializando sensor de presion...");
  
  // Configurar el pin SS como salida
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // Asegurarse de que el sensor no esté seleccionado al inicio

  // Inicializar la interfaz SPI
  // El modo SPI_MODE1 es crucial, ya que el sensor de Honeywell cambia los datos
  // en el flanco de bajada del reloj (CPHA=1) y el microcontrolador los lee
  // en el flanco de subida.
  SPI.begin();
}

void loop() {
  unsigned int rawData;
  float pressure_mbar;
  int status;

  // Asegura que SS esté alto antes de iniciar la transacción
  digitalWrite(SS_PIN, HIGH);

  // Prueba con SPI_MODE1 y frecuencia baja
  SPI.beginTransaction(SPISettings(20000, MSBFIRST, SPI_MODE1));
  digitalWrite(SS_PIN, LOW);

  delay(10); // Retardo corto para estabilidad

  byte byte1 = SPI.transfer(0x00);
  byte byte2 = SPI.transfer(0x00);
  delay(10); // Retardo corto para estabilidad
  digitalWrite(SS_PIN, HIGH);
  SPI.endTransaction();

  Serial.print("Byte1: 0x"); Serial.print(byte1, HEX);
  Serial.print(" | Byte2: 0x"); Serial.println(byte2, HEX);

  rawData = (byte1 << 8) | byte2;
  status = (rawData >> 14) & 0b11;

  Serial.print("Estado del sensor: ");
  if (status == 0) {
    Serial.print("OK");
  } else if (status == 1) {
    Serial.print("En modo comando");
  } else if (status == 2) {
    Serial.print("Datos obsoletos");
  } else if (status == 3) {
    Serial.print("Error de diagnostico");
  }
  Serial.print(" | Datos raw (hex): 0x");
  Serial.print(rawData, HEX);

  unsigned int pressureData = rawData & 0x3FFF;
  pressure_mbar = ((float)pressureData - OUTPUT_MIN) * (P_MAX - P_MIN) / (OUTPUT_MAX - OUTPUT_MIN) + P_MIN;

  Serial.print(" | Presion: ");
  Serial.print(pressure_mbar, 3);
  Serial.println(" mbar");

  delay(100);
}