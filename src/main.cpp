#include <Wire.h>
#include <LSM6DSLSensor.h>


#define I2C3_SCL    D12
#define I2C3_SDA    D11


TwoWire dev_i2c(I2C3_SDA, I2C3_SCL);
LSM6DSLSensor AccGyr(&dev_i2c, LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW);

// Variables para timing a 6.66 kHz
unsigned long previousMicros = 0;
const unsigned long interval = 150; // 1/6660 Hz = ~150 microseconds


void setup() {
  Serial.begin(2000000); // Baudios altos para transmisión rápida
  while (!Serial);

  // Inicializamos el bus en los pines alternativos
  dev_i2c.begin();
  
  // Inicializamos el sensor en este bus
  if (AccGyr.begin() != LSM6DSL_STATUS_OK) {
    Serial.println("Error: No se encontró LSM6DSL en I2C3!");
    while (1);
  }
  
  // Habilitamos acelerómetro y giroscopio
  AccGyr.Enable_X();
  AccGyr.Enable_G();
  
  // Configuramos ODR (Output Data Rate) alto para ambos sensores
  // Para 6.66 kHz necesitamos la frecuencia más alta disponible
  AccGyr.Set_X_ODR(6660.0); // Acelerómetro a 6.66 kHz
  AccGyr.Set_G_ODR(6660.0); // Giroscopio a 6.66 kHz
  
  // Configuramos full scale
  AccGyr.Set_X_FS(4.0); // ±4g para acelerómetro
  AccGyr.Set_G_FS(2000.0); // ±2000 dps para giroscopio

  Serial.println("LSM6DSL configurado para 6.66 kHz");
  Serial.println("Formato: Tiempo(us),AccX,AccY,AccZ,GyroX,GyroY,GyroZ");
}

void loop() {
  unsigned long currentMicros = micros();
  
  // Verificamos si ha pasado el tiempo necesario para 6.66 kHz
  if (currentMicros - previousMicros >= interval) {
    previousMicros = currentMicros;
    
    // Datos del acelerómetro
    int32_t accelerometer[3];
    AccGyr.Get_X_Axes(accelerometer);
    
    // Datos del giroscopio
    int32_t gyroscope[3];
    AccGyr.Get_G_Axes(gyroscope);
    
    // Imprimimos datos en formato CSV con timestamp
    Serial.print(accelerometer[0]);
    Serial.print(",");
    Serial.print(accelerometer[1]);
    Serial.print(",");
    Serial.print(accelerometer[2]);
    Serial.print(",");
    Serial.print(gyroscope[0]);
    Serial.print(",");
    Serial.print(gyroscope[1]);
    Serial.print(",");
    Serial.println(gyroscope[2]);
  }
}