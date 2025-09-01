#include <Arduino.h>
#include <Wire.h>
#include <Arduino_PF1550.h>
#include "PF1550/PF1550_IO_Portenta_H7.h"

// Definir pines específicos para el PMIC PF1550
#define PMIC_SDA_PIN    PB_7  // SDA en PB7
#define PMIC_SCL_PIN    PB_6  // SCL en PB6

// Crear instancia de TwoWire para el bus I2C del PMIC
TwoWire pmicWire(PMIC_SDA_PIN, PMIC_SCL_PIN);

// Crear instancia personalizada del PMIC con los pines específicos
PF1550_IO_Portenta_H7 pmicIO(&pmicWire, PF1550_I2C_ADDR);
PF1550 customPMIC(pmicIO);

// Estructura para mapear registros y sus descripciones
struct RegisterInfo {
  uint8_t address;
  const char* name;
  const char* description;
};

// Mapeo de registros principales del PF1550
const RegisterInfo registers[] = {
  {0x00, "DEVICE_ID", "Device ID"},
  {0x01, "OTP_ID", "OTP ID"},
  {0x02, "PROG_ID", "Program ID"},
  {0x03, "INT_CATEGORY", "Interrupt Category"},
  {0x04, "SW_INT_STAT0", "Switch Interrupt Status 0"},
  {0x05, "SW_INT_MASK0", "Switch Interrupt Mask 0"},
  {0x06, "SW_INT_SENSE0", "Switch Interrupt Sense 0"},
  {0x07, "SW_INT_STAT1", "Switch Interrupt Status 1"},
  {0x08, "SW_INT_MASK1", "Switch Interrupt Mask 1"},
  {0x09, "SW_INT_SENSE1", "Switch Interrupt Sense 1"},
  {0x0A, "LDO_INT_STAT0", "LDO Interrupt Status 0"},
  {0x0B, "LDO_INT_MASK0", "LDO Interrupt Mask 0"},
  {0x0C, "LDO_INT_SENSE0", "LDO Interrupt Sense 0"},
  {0x0D, "TEMP_INT_STAT0", "Temperature Interrupt Status 0"},
  {0x0E, "TEMP_INT_MASK0", "Temperature Interrupt Mask 0"},
  {0x0F, "TEMP_INT_SENSE0", "Temperature Interrupt Sense 0"},
  {0x10, "MISC_INT_STAT0", "Misc Interrupt Status 0"},
  {0x11, "MISC_INT_MASK0", "Misc Interrupt Mask 0"},
  {0x12, "MISC_INT_SENSE0", "Misc Interrupt Sense 0"},
  {0x20, "CHARG_INT_STAT0", "Charger Interrupt Status 0"},
  {0x21, "CHARG_INT_MASK0", "Charger Interrupt Mask 0"},
  {0x22, "CHARG_INT_SENSE0", "Charger Interrupt Sense 0"},
  {0x23, "CHARG_INT_STAT1", "Charger Interrupt Status 1"},
  {0x24, "CHARG_INT_MASK1", "Charger Interrupt Mask 1"},
  {0x25, "CHARG_INT_SENSE1", "Charger Interrupt Sense 1"},
  {0x26, "CHARG_INT_STAT2", "Charger Interrupt Status 2"},
  {0x27, "CHARG_INT_MASK2", "Charger Interrupt Mask 2"},
  {0x28, "CHARG_INT_SENSE2", "Charger Interrupt Sense 2"},
  {0x29, "CHARG_INT_STAT3", "Charger Interrupt Status 3"},
  {0x2A, "CHARG_INT_MASK3", "Charger Interrupt Mask 3"},
  {0x2B, "CHARG_INT_SENSE3", "Charger Interrupt Sense 3"},
  {0x30, "SW1_VOLT", "SW1 Voltage Control"},
  {0x31, "SW1_STBY_VOLT", "SW1 Standby Voltage"},
  {0x32, "SW1_SLP_VOLT", "SW1 Sleep Voltage"},
  {0x33, "SW1_CTRL", "SW1 Control"},
  {0x34, "SW1_CTRL1", "SW1 Control 1"},
  {0x35, "SW2_VOLT", "SW2 Voltage Control"},
  {0x36, "SW2_STBY_VOLT", "SW2 Standby Voltage"},
  {0x37, "SW2_SLP_VOLT", "SW2 Sleep Voltage"},
  {0x38, "SW2_CTRL", "SW2 Control"},
  {0x39, "SW2_CTRL1", "SW2 Control 1"},
  {0x40, "LDO1_VOLT", "LDO1 Voltage Control"},
  {0x41, "LDO1_CTRL", "LDO1 Control"},
  {0x42, "LDO2_VOLT", "LDO2 Voltage Control"},
  {0x43, "LDO2_CTRL", "LDO2 Control"},
  {0x44, "LDO3_VOLT", "LDO3 Voltage Control"},
  {0x45, "LDO3_CTRL", "LDO3 Control"},
  {0x50, "VSNVS_CTRL", "VSNVS Control"},
  {0x60, "CHARG_CTRL", "Charger Control"},
  {0x61, "CHARG_CURR", "Charger Current"},
  {0x62, "CHARG_EOC_CNFG", "Charger End of Charge Config"},
  {0x63, "CHARG_CURR_CNFG", "Charger Current Config"},
  {0x64, "CHARG_TIMER", "Charger Timer"},
  {0x65, "CHARG_WD_CNFG", "Charger Watchdog Config"},
  {0x66, "LED_PWM", "LED PWM Control"},
  {0x67, "CHARG_SNVS_CNFG", "Charger SNVS Config"},
  {0x68, "CHARG_OP_MODE", "Charger Operation Mode"},
  {0x69, "BATT_SNVS", "Battery SNVS"},
  {0x6A, "STARTUP_CTRL", "Startup Control"},
  {0x6B, "STARTUP_CTRL2", "Startup Control 2"},
  {0x70, "PWRDN_CTRL", "Power Down Control"},
  {0x71, "WD_CONFIG", "Watchdog Config"},
  {0x72, "WD_CLEAR", "Watchdog Clear"},
  {0x73, "WD_EXPIRE", "Watchdog Expire"},
  {0x74, "WD_COUNTER", "Watchdog Counter"},
  {0x75, "PWRDN_TIME", "Power Down Time"},
  {0x80, "COIN_CTRL", "Coin Cell Control"},
  {0x81, "PWRON_CTRL", "Power On Control"},
  {0x88, "WD_SNVS_CONFIG", "Watchdog SNVS Config"},
  {0x89, "WD_SNVS_CLEAR", "Watchdog SNVS Clear"},
  {0x8A, "WD_SNVS_EXPIRE", "Watchdog SNVS Expire"},
  {0x8B, "WD_SNVS_COUNT", "Watchdog SNVS Count"},
  {0x90, "TEMP_SENS_CNFG", "Temperature Sensor Config"},
  {0x91, "TEMP_AMB", "Ambient Temperature"},
  {0x92, "TEMP_BBQ", "BBQ Temperature"},
  {0x93, "VMAIN_SNS", "VMAIN Sense"},
  {0x94, "VSNVS_SNS", "VSNVS Sense"},
  {0x95, "VDD_SNS", "VDD Sense"},
  {0x96, "VCOIN_SNS", "VCOIN Sense"},
  {0x97, "VBAT_SNS", "VBAT Sense"}
};

const int NUM_REGISTERS = sizeof(registers) / sizeof(RegisterInfo);


// Función para interpretar el registro DEVICE_ID
void interpretDeviceId(uint8_t value) {
  Serial.print("  Device ID: 0x"); Serial.print(value, HEX);
  if (value == 0x7C) Serial.println(" (PF1550 - Correcto)");
  else if (value == 0x10) Serial.println(" (PF1550 - ID alternativo)");
  else Serial.println(" (Unknown device)");
}

// Función para interpretar registros de voltage
void interpretVoltageRegister(const char* name, uint8_t value) {
  Serial.print("  "); Serial.print(name); Serial.print(": ");
  // Decodificar según el tipo de regulador
  if (strstr(name, "SW1") || strstr(name, "SW2")) {
    float voltage = 0.4 + (value & 0x3F) * 0.025; // SW: 0.4V base, 25mV steps
    Serial.print(voltage, 3); Serial.println(" V");
  } else if (strstr(name, "LDO")) {
    if ((value & 0x0F) == 0) Serial.println("0.75 V");
    else {
      float voltage = 1.5 + ((value & 0x0F) - 1) * 0.1; // LDO: 1.5V base, 100mV steps
      Serial.print(voltage, 1); Serial.println(" V");
    }
  }
}

// Función para interpretar registros de control
void interpretControlRegister(const char* name, uint8_t value) {
  Serial.print("  "); Serial.print(name); Serial.print(": 0x"); Serial.print(value, HEX);
  Serial.print(" [");
  if (value & 0x01) Serial.print("EN ");
  if (value & 0x02) Serial.print("STBY_EN ");
  if (value & 0x04) Serial.print("SLP_EN ");
  if (value & 0x08) Serial.print("LPWR ");
  if (value & 0x10) Serial.print("FPWM ");
  if (value & 0x20) Serial.print("PHASE ");
  if (value & 0x40) Serial.print("ILIM ");
  if (value & 0x80) Serial.print("WDIS ");
  Serial.println("]");
}

// Función para interpretar estado del cargador
void interpretChargerStatus(uint8_t value) {
  Serial.print("  Charger Status: 0x"); Serial.print(value, HEX);
  Serial.print(" [");
  if (value & 0x01) Serial.print("BATT_DET ");
  if (value & 0x02) Serial.print("CHG_OK ");
  if (value & 0x04) Serial.print("BAT_OK ");
  if (value & 0x08) Serial.print("CHG_TMR ");
  if (value & 0x10) Serial.print("DONE ");
  if (value & 0x20) Serial.print("SUSP ");
  if (value & 0x40) Serial.print("VBUS_OK ");
  if (value & 0x80) Serial.print("THERM_OK ");
  Serial.println("]");
}

// Función para interpretar temperatura
void interpretTemperature(const char* sensor, uint8_t value) {
  Serial.print("  "); Serial.print(sensor); Serial.print(": ");
  if (value == 0xFF) {
    Serial.println("Invalid/Not Available");
  } else {
    int temp = (int)value - 40; // Offset de -40°C
    Serial.print(temp); Serial.println(" °C");
  }
}

// Función para interpretar voltajes de sensores
void interpretSensorVoltage(const char* sensor, uint8_t value) {
  Serial.print("  "); Serial.print(sensor); Serial.print(": ");
  float voltage = value * 0.025; // 25mV por LSB típico
  Serial.print(voltage, 3); Serial.println(" V");
}
void readAndDisplayAllRegistersDefault();
void readAndDisplayAllRegisters();
// Variable global para indicar qué PMIC está en uso
bool usingCustomPMIC = false;
bool usingDirectI2C = false;

// Función para leer registros directamente via I2C
uint8_t readPMICRegisterDirect(uint8_t regAddr) {
  pmicWire.beginTransmission(PF1550_I2C_DEFAULT_ADDR);
  pmicWire.write(regAddr);
  if (pmicWire.endTransmission(false) != 0) {
    return 0xFF; // Error
  }
  
  pmicWire.requestFrom(PF1550_I2C_DEFAULT_ADDR, (uint8_t)1);
  if (pmicWire.available()) {
    return pmicWire.read();
  }
  return 0xFF; // Error
}

// Función para lecturas directas I2C cuando los drivers fallan
void performDirectI2CReads() {
  Serial.println("=== LECTURAS DIRECTAS I2C DEL PF1550 ===");
  Serial.println("Usando comunicación I2C de bajo nivel");
  Serial.println("=========================================");
  
  usingDirectI2C = true;
  
  // Leer algunos registros críticos directamente
  uint8_t criticalRegs[] = {0x00, 0x30, 0x35, 0x40, 0x42, 0x44, 0x33, 0x38, 0x91, 0x92, 0x93, 0x97, 0x20};
  const char* criticalNames[] = {"DEVICE_ID", "SW1_VOLT", "SW2_VOLT", "LDO1_VOLT", "LDO2_VOLT", "LDO3_VOLT", 
                                "SW1_CTRL", "SW2_CTRL", "TEMP_AMB", "TEMP_BBQ", "VMAIN_SNS", "VBAT_SNS", "CHARG_STAT"};
  
  for (int i = 0; i < 13; i++) {
    uint8_t value = readPMICRegisterDirect(criticalRegs[i]);
    
    Serial.print("[0x");
    if (criticalRegs[i] < 16) Serial.print("0");
    Serial.print(criticalRegs[i], HEX);
    Serial.print("] ");
    Serial.print(criticalNames[i]);
    Serial.print(": 0x");
    if (value < 16) Serial.print("0");
    Serial.print(value, HEX);
    Serial.print(" (");
    Serial.print(value);
    Serial.println(")");
    
    // Interpretación básica
    if (criticalRegs[i] == 0x00) {
      Serial.print("  Device ID: ");
      if (value == 0x7C) Serial.println("PF1550 ✓");
      else if (value == 0x10) Serial.println("PF1550 (ID alternativo) ✓");
      else if (value == 0xFF) Serial.println("Error de lectura");
      else Serial.println("ID desconocido");
    }
    else if (criticalRegs[i] == 0x91 || criticalRegs[i] == 0x92) { // Temperaturas
      Serial.print("  Temperatura: ");
      if (value == 0x00) {
        Serial.println("Sensor no habilitado o sin batería");
      } else if (value == 0xFF) {
        Serial.println("Error de lectura");
      } else {
        int temp = (int)value - 40;
        Serial.print(temp); Serial.println("°C");
      }
    }
    else if (criticalRegs[i] == 0x97) { // VBAT
      Serial.print("  VBAT: ");
      if (value == 0x00) {
        Serial.println("Sin batería detectada");
      } else {
        float voltage = value * 0.025;
        Serial.print(voltage, 3); Serial.println(" V");
      }
    }
    
    delay(10);
  }
  
  Serial.println("=========================================");
}


void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Inicializar el bus I2C personalizado para el PMIC
  pmicWire.begin();
  Serial.println("Bus I2C personalizado inicializado (SDA: PB7, SCL: PB6)");
  
  // Configurar velocidad I2C (el PMIC puede requerir velocidad específica)
  pmicWire.setClock(100000); // 100kHz estándar
  Serial.println("Velocidad I2C configurada a 100kHz");
  
  // Verificar la dirección que vamos a usar
  Serial.print("Dirección PMIC configurada: 0x");
  if (PF1550_I2C_DEFAULT_ADDR < 16) Serial.print("0");
  Serial.println(PF1550_I2C_DEFAULT_ADDR, HEX);
  
  // Hacer un escaneo I2C para detectar dispositivos
  Serial.println("Escaneando bus I2C...");
  bool deviceFound = false;
  bool pmicFound = false;
  
  for (uint8_t addr = 1; addr < 127; addr++) {
    pmicWire.beginTransmission(addr);
    if (pmicWire.endTransmission() == 0) {
      Serial.print("Dispositivo encontrado en dirección: 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      
      if (addr == PF1550_I2C_DEFAULT_ADDR) {
        Serial.println(" <- ¡PMIC PF1550 detectado!");
        pmicFound = true;
      } else {
        Serial.println("");
      }
      deviceFound = true;
    }
  }
  
  if (!deviceFound) {
    Serial.println("No se encontraron dispositivos I2C. Verificar conexiones.");
    Serial.println("Posibles problemas:");
    Serial.println("- Conexiones de PB6 (SCL) y PB7 (SDA)");
    Serial.println("- Alimentación del PMIC");
    Serial.println("- Pull-ups en líneas I2C");
    Serial.println("Intentando con el PMIC global predeterminado...");
    
    // Intentar con el PMIC global como fallback
    if (!PMIC.begin()) {
      Serial.println("Error: Tampoco se pudo inicializar el PMIC predeterminado");
      while (1);
    } else {
      Serial.println("PMIC predeterminado inicializado correctamente");
      Serial.println("Usando PMIC global en lugar del personalizado");
      Serial.println("=====================================");
      usingCustomPMIC = false;
      readAndDisplayAllRegistersDefault();
      return;
    }
  } else if (!pmicFound) {
    Serial.print("Se encontraron dispositivos I2C, pero no el PMIC en 0x");
    if (PF1550_I2C_DEFAULT_ADDR < 16) Serial.print("0");
    Serial.println(PF1550_I2C_DEFAULT_ADDR, HEX);
    Serial.println("Intentando inicializar el PMIC personalizado de todas formas...");
  } else {
    Serial.println("¡PMIC PF1550 detectado correctamente en la dirección esperada!");
  }

  // Dado que sabemos que el PMIC responde correctamente via I2C,
  // saltar directamente a lecturas directas para evitar problemas con los drivers
  Serial.println("PMIC detectado correctamente. Saltando a lecturas directas I2C...");
  Serial.println("Esto evita problemas con los drivers de la biblioteca.");
  
  usingCustomPMIC = false;
  performDirectI2CReads();
  return;

  // Inicializar el PMIC con los pines específicos
  Serial.println("Intentando inicializar PMIC personalizado...");
  
  // Probar lectura directa antes de la inicialización completa
  Serial.println("Probando lectura directa del registro DEVICE_ID...");
  pmicWire.beginTransmission(PF1550_I2C_DEFAULT_ADDR);
  pmicWire.write(0x00); // Registro DEVICE_ID
  uint8_t error = pmicWire.endTransmission(false);
  
  if (error == 0) {
    pmicWire.requestFrom(PF1550_I2C_DEFAULT_ADDR, (uint8_t)1);
    if (pmicWire.available()) {
      uint8_t deviceId = pmicWire.read();
      Serial.print("Device ID leído directamente: 0x");
      if (deviceId < 16) Serial.print("0");
      Serial.print(deviceId, HEX);
      if (deviceId == 0x7C) {
        Serial.println(" <- ¡Correcto! Es un PF1550");
      } else {
        Serial.println(" <- ID inesperado");
      }
    } else {
      Serial.println("No se recibieron datos en la lectura directa");
    }
  } else {
    Serial.print("Error en transmisión I2C: ");
    Serial.println(error);
  }
  
  // Intentar inicialización del PMIC personalizado
  if (!customPMIC.begin()) {
    Serial.println("Error: No se pudo inicializar el PMIC PF1550 personalizado");
    
    // Probar con diferentes configuraciones de I2C
    Serial.println("Probando con velocidad I2C más baja...");
    pmicWire.setClock(50000); // 50kHz
    
    // Dar un poco de tiempo para que se estabilice
    delay(100);
    Serial.println("Intentando inicialización con 50kHz...");
    
    bool lowSpeedSuccess = false;
    // Intentar varias veces con timeout
    for (int attempt = 0; attempt < 3; attempt++) {
      Serial.print("Intento "); Serial.print(attempt + 1); Serial.println("/3...");
      
      unsigned long startTime = millis();
      bool initResult = customPMIC.begin();
      unsigned long endTime = millis();
      
      Serial.print("Tiempo de inicialización: "); 
      Serial.print(endTime - startTime); 
      Serial.println(" ms");
      
      if (initResult) {
        lowSpeedSuccess = true;
        break;
      } else {
        Serial.println("Fallo en este intento");
        delay(200); // Pausa entre intentos
      }
      
      // Si tarda más de 5 segundos, abortar
      if ((endTime - startTime) > 5000) {
        Serial.println("Timeout en inicialización, abortando...");
        break;
      }
    }
    
    if (!lowSpeedSuccess) {
      Serial.println("Error: Tampoco funcionó con 50kHz después de 3 intentos");
      Serial.println("Saltando al PMIC global predeterminado...");
      
      // Fallback al PMIC global
      if (!PMIC.begin()) {
        Serial.println("Error: Tampoco se pudo inicializar el PMIC predeterminado");
        Serial.println("Procediendo con lecturas directas I2C...");
        
        // Si todo falla, hacer lecturas directas
        usingCustomPMIC = false;
        performDirectI2CReads();
        return;
      } else {
        Serial.println("PMIC predeterminado inicializado correctamente");
        Serial.println("Usando PMIC global en lugar del personalizado");
        Serial.println("=====================================");
        usingCustomPMIC = false;
        readAndDisplayAllRegistersDefault();
        return;
      }
    } else {
      Serial.println("¡Éxito con velocidad reducida!");
    }
  }

  Serial.println("¡PMIC PF1550 inicializado correctamente en pines específicos!");
  Serial.println("SDA: PB7, SCL: PB6, Dirección: 0x08");
  Serial.println("=====================================");
  usingCustomPMIC = true;
  
  // Leer y mostrar todos los registros
  readAndDisplayAllRegisters();
}

// Función principal para leer y mostrar todos los registros
void readAndDisplayAllRegisters() {
  Serial.println("\n=== LECTURA COMPLETA DE REGISTROS PF1550 (PMIC PERSONALIZADO) ===\n");
  
  for (int i = 0; i < NUM_REGISTERS; i++) {
    uint8_t value = customPMIC.readPMICreg((Register)registers[i].address);
    
    Serial.print("[0x"); 
    if (registers[i].address < 0x10) Serial.print("0");
    Serial.print(registers[i].address, HEX);
    Serial.print("] ");
    Serial.print(registers[i].name);
    Serial.print(" - ");
    Serial.print(registers[i].description);
    Serial.print(": 0x");
    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);
    Serial.print(" (");
    Serial.print(value);
    Serial.println(")");
    
    // Interpretación específica según el registro
    if (registers[i].address == 0x00) {
      interpretDeviceId(value);
    }
    else if (strstr(registers[i].name, "VOLT")) {
      interpretVoltageRegister(registers[i].name, value);
    }
    else if (strstr(registers[i].name, "CTRL")) {
      interpretControlRegister(registers[i].name, value);
    }
    else if (strstr(registers[i].name, "CHARG") && strstr(registers[i].name, "STAT")) {
      interpretChargerStatus(value);
    }
    else if (strstr(registers[i].name, "TEMP")) {
      interpretTemperature(registers[i].name, value);
    }
    else if (strstr(registers[i].name, "_SNS")) {
      interpretSensorVoltage(registers[i].name, value);
    }
    
    Serial.println();
    delay(10); // Pequeña pausa entre lecturas
  }
}

// Función de fallback usando el PMIC global predeterminado
void readAndDisplayAllRegistersDefault() {
  Serial.println("\n=== LECTURA COMPLETA DE REGISTROS PF1550 (PMIC PREDETERMINADO) ===\n");
  
  for (int i = 0; i < NUM_REGISTERS; i++) {
    uint8_t value = PMIC.readPMICreg((Register)registers[i].address);
    
    Serial.print("[0x"); 
    if (registers[i].address < 0x10) Serial.print("0");
    Serial.print(registers[i].address, HEX);
    Serial.print("] ");
    Serial.print(registers[i].name);
    Serial.print(" - ");
    Serial.print(registers[i].description);
    Serial.print(": 0x");
    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);
    Serial.print(" (");
    Serial.print(value);
    Serial.println(")");
    
    // Interpretación específica según el registro
    if (registers[i].address == 0x00) {
      interpretDeviceId(value);
    }
    else if (strstr(registers[i].name, "VOLT")) {
      interpretVoltageRegister(registers[i].name, value);
    }
    else if (strstr(registers[i].name, "CTRL")) {
      interpretControlRegister(registers[i].name, value);
    }
    else if (strstr(registers[i].name, "CHARG") && strstr(registers[i].name, "STAT")) {
      interpretChargerStatus(value);
    }
    else if (strstr(registers[i].name, "TEMP")) {
      interpretTemperature(registers[i].name, value);
    }
    else if (strstr(registers[i].name, "_SNS")) {
      interpretSensorVoltage(registers[i].name, value);
    }
    
    Serial.println();
    delay(10); // Pequeña pausa entre lecturas
  }
}

// Función para monitoreo en tiempo real de registros críticos
void monitorCriticalRegisters() {
  Serial.println("=== MONITOREO EN TIEMPO REAL (PMIC PERSONALIZADO) ===");
  
  // Voltajes principales
  uint8_t sw1_volt = customPMIC.readPMICreg((Register)0x30);
  uint8_t sw2_volt = customPMIC.readPMICreg((Register)0x35);
  uint8_t ldo1_volt = customPMIC.readPMICreg((Register)0x40);
  uint8_t ldo2_volt = customPMIC.readPMICreg((Register)0x42);
  uint8_t ldo3_volt = customPMIC.readPMICreg((Register)0x44);
  
  // Estados de control
  uint8_t sw1_ctrl = customPMIC.readPMICreg((Register)0x33);
  uint8_t sw2_ctrl = customPMIC.readPMICreg((Register)0x38);
  
  // Temperaturas
  uint8_t temp_amb = customPMIC.readPMICreg((Register)0x91);
  uint8_t temp_bbq = customPMIC.readPMICreg((Register)0x92);
  
  // Voltajes de sensores
  uint8_t vmain = customPMIC.readPMICreg((Register)0x93);
  uint8_t vbat = customPMIC.readPMICreg((Register)0x97);
  
  // Estado del cargador
  uint8_t charg_stat = customPMIC.readPMICreg((Register)0x20);
  
  Serial.print("SW1: "); Serial.print((0.4 + (sw1_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw1_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  
  Serial.print("SW2: "); Serial.print((0.4 + (sw2_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw2_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  
  Serial.print("VBAT: "); Serial.print(vbat * 0.025, 3); Serial.print("V | ");
  Serial.print("VMAIN: "); Serial.print(vmain * 0.025, 3); Serial.print("V | ");
  
  Serial.print("Temp: "); Serial.print((int)temp_amb - 40); Serial.print("°C | ");
  
  Serial.print("CHG: 0x"); Serial.println(charg_stat, HEX);
}

// Función de monitoreo usando el PMIC predeterminado
void monitorCriticalRegistersDefault() {
  Serial.println("=== MONITOREO EN TIEMPO REAL (PMIC PREDETERMINADO) ===");
  
  // Voltajes principales
  uint8_t sw1_volt = PMIC.readPMICreg((Register)0x30);
  uint8_t sw2_volt = PMIC.readPMICreg((Register)0x35);
  uint8_t ldo1_volt = PMIC.readPMICreg((Register)0x40);
  uint8_t ldo2_volt = PMIC.readPMICreg((Register)0x42);
  uint8_t ldo3_volt = PMIC.readPMICreg((Register)0x44);
  
  // Estados de control
  uint8_t sw1_ctrl = PMIC.readPMICreg((Register)0x33);
  uint8_t sw2_ctrl = PMIC.readPMICreg((Register)0x38);
  
  // Temperaturas
  uint8_t temp_amb = PMIC.readPMICreg((Register)0x91);
  uint8_t temp_bbq = PMIC.readPMICreg((Register)0x92);
  
  // Voltajes de sensores
  uint8_t vmain = PMIC.readPMICreg((Register)0x93);
  uint8_t vbat = PMIC.readPMICreg((Register)0x97);
  
  // Estado del cargador
  uint8_t charg_stat = PMIC.readPMICreg((Register)0x20);
  
  Serial.print("SW1: "); Serial.print((0.4 + (sw1_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw1_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  
  Serial.print("SW2: "); Serial.print((0.4 + (sw2_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw2_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  
  Serial.print("VBAT: "); Serial.print(vbat * 0.025, 3); Serial.print("V | ");
  Serial.print("VMAIN: "); Serial.print(vmain * 0.025, 3); Serial.print("V | ");
  
  Serial.print("Temp: "); Serial.print((int)temp_amb - 40); Serial.print("°C | ");
  
  Serial.print("CHG: 0x"); Serial.println(charg_stat, HEX);
}


void loop() {
  static unsigned long lastFullScan = 0;
  static unsigned long lastMonitor = 0;
  
  unsigned long now = millis();
  
  // Si estamos usando lecturas directas I2C, hacer monitoreo simple
  if (usingDirectI2C) {
    if (now - lastMonitor >= 5000) { // Cada 5 segundos
      Serial.println("\n=== MONITOREO DIRECTO I2C ===");
      
      uint8_t deviceId = readPMICRegisterDirect(0x00);
      uint8_t sw1_volt = readPMICRegisterDirect(0x30);
      uint8_t sw2_volt = readPMICRegisterDirect(0x35);
      uint8_t vbat = readPMICRegisterDirect(0x97);
      uint8_t temp = readPMICRegisterDirect(0x91);
      
      Serial.print("ID: 0x"); Serial.print(deviceId, HEX);
      Serial.print(" | SW1: "); Serial.print((0.4 + (sw1_volt & 0x3F) * 0.025), 3); Serial.print("V");
      Serial.print(" | SW2: "); Serial.print((0.4 + (sw2_volt & 0x3F) * 0.025), 3); Serial.print("V");
      
      if (vbat == 0x00) {
        Serial.print(" | VBAT: Sin batería");
      } else {
        Serial.print(" | VBAT: "); Serial.print(vbat * 0.025, 3); Serial.print("V");
      }
      
      if (temp == 0x00) {
        Serial.println(" | Temp: Sensor deshabilitado");
      } else {
        Serial.print(" | Temp: "); Serial.print((int)temp - 40); Serial.println("°C");
      }
      
      lastMonitor = now;
    }
    return;
  }
  
  // Comportamiento normal para PMIC inicializado
  // Escaneo completo cada 30 segundos
  if (now - lastFullScan >= 30000) {
    if (usingCustomPMIC) {
      readAndDisplayAllRegisters();
    } else {
      readAndDisplayAllRegistersDefault();
    }
    lastFullScan = now;
    Serial.println("=====================================\n");
  }
  
  // Monitoreo rápido cada 2 segundos
  if (now - lastMonitor >= 2000) {
    if (usingCustomPMIC) {
      monitorCriticalRegisters();
    } else {
      monitorCriticalRegistersDefault();
    }
    lastMonitor = now;
  }
  
  delay(100);
}
