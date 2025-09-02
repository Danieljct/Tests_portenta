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

// Usar las definiciones de registros de la biblioteca
const Register criticalRegisters[] = {
  Register::PMIC_DEVICE_ID,
  Register::PMIC_SW1_VOLT,
  Register::PMIC_SW1_CTRL,
  Register::PMIC_SW2_VOLT,
  Register::PMIC_SW2_CTRL,
  Register::PMIC_LDO1_VOLT,
  Register::PMIC_LDO1_CTRL,
  Register::PMIC_LDO2_VOLT,
  Register::PMIC_LDO2_CTRL,
  Register::PMIC_LDO3_VOLT,
  Register::PMIC_LDO3_CTRL,
  Register::CHARGER_CHG_SNS,
  Register::CHARGER_BATT_SNS,
  Register::CHARGER_VBUS_SNS
};

const char* criticalRegisterNames[] = {
  "DEVICE_ID",
  "SW1_VOLT",
  "SW1_CTRL", 
  "SW2_VOLT",
  "SW2_CTRL",
  "LDO1_VOLT",
  "LDO1_CTRL",
  "LDO2_VOLT", 
  "LDO2_CTRL",
  "LDO3_VOLT",
  "LDO3_CTRL",
  "CHG_SNS",
  "BATT_SNS",
  "VBUS_SNS"
};

const int NUM_CRITICAL_REGISTERS = sizeof(criticalRegisters) / sizeof(Register);


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
  
  // Usar registros críticos definidos con la biblioteca
  uint8_t criticalRegs[] = {
    static_cast<uint8_t>(Register::PMIC_DEVICE_ID),
    static_cast<uint8_t>(Register::PMIC_SW1_VOLT),
    static_cast<uint8_t>(Register::PMIC_SW2_VOLT),
    static_cast<uint8_t>(Register::PMIC_LDO1_VOLT),
    static_cast<uint8_t>(Register::PMIC_LDO2_VOLT),
    static_cast<uint8_t>(Register::PMIC_LDO3_VOLT),
    static_cast<uint8_t>(Register::PMIC_SW1_CTRL),
    static_cast<uint8_t>(Register::PMIC_SW2_CTRL),
    static_cast<uint8_t>(Register::CHARGER_BATT_SNS),
    static_cast<uint8_t>(Register::CHARGER_CHG_SNS)
  };
  
  const char* criticalNames[] = {
    "DEVICE_ID", "SW1_VOLT", "SW2_VOLT", "LDO1_VOLT", "LDO2_VOLT", 
    "LDO3_VOLT", "SW1_CTRL", "SW2_CTRL", "BATT_SNS", "CHG_SNS"
  };
  
  for (int i = 0; i < 10; i++) {
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
    
    // Interpretación usando las funciones existentes
    if (criticalRegs[i] == static_cast<uint8_t>(Register::PMIC_DEVICE_ID)) {
      interpretDeviceId(value);
    }
    else if (strstr(criticalNames[i], "VOLT")) {
      interpretVoltageRegister(criticalNames[i], value);
    }
    else if (strstr(criticalNames[i], "CTRL")) {
      interpretControlRegister(criticalNames[i], value);
    }
    
    delay(10);
  }
  
  Serial.println("=========================================");
}


void setup() {
  bootM4();
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

// Función principal para leer registros usando la biblioteca
void readAndDisplayCriticalRegisters() {
  Serial.println("\n=== LECTURA DE REGISTROS CRÍTICOS PF1550 ===\n");
  
  for (int i = 0; i < NUM_CRITICAL_REGISTERS; i++) {
    uint8_t value;
    
    if (usingCustomPMIC) {
      value = customPMIC.readPMICreg(criticalRegisters[i]);
    } else {
      value = PMIC.readPMICreg(criticalRegisters[i]);
    }
    
    Serial.print("[0x"); 
    uint8_t regAddr = static_cast<uint8_t>(criticalRegisters[i]);
    if (regAddr < 0x10) Serial.print("0");
    Serial.print(regAddr, HEX);
    Serial.print("] ");
    Serial.print(criticalRegisterNames[i]);
    Serial.print(": 0x");
    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);
    Serial.print(" (");
    Serial.print(value);
    Serial.println(")");
    
    // Interpretación específica según el registro
    if (criticalRegisters[i] == Register::PMIC_DEVICE_ID) {
      interpretDeviceId(value);
    }
    else if (strstr(criticalRegisterNames[i], "VOLT")) {
      interpretVoltageRegister(criticalRegisterNames[i], value);
    }
    else if (strstr(criticalRegisterNames[i], "CTRL")) {
      interpretControlRegister(criticalRegisterNames[i], value);
    }
    
    Serial.println();
    delay(10);
  }
}

// Función de fallback usando el PMIC global predeterminado
void readAndDisplayAllRegistersDefault() {
  Serial.println("\n=== LECTURA COMPLETA DE REGISTROS PF1550 (PMIC PREDETERMINADO) ===\n");
  
  for (int i = 0; i < NUM_CRITICAL_REGISTERS; i++) {
    uint8_t value = PMIC.readPMICreg(criticalRegisters[i]);
    
    Serial.print("[0x"); 
    if (static_cast<uint8_t>(criticalRegisters[i]) < 0x10) Serial.print("0");
    Serial.print(static_cast<uint8_t>(criticalRegisters[i]), HEX);
    Serial.print("] ");
    Serial.print(criticalRegisterNames[i]);
    Serial.print(" - ");
    Serial.print(criticalRegisterNames[i]);
    Serial.print(": 0x");
    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);
    Serial.print(" (");
    Serial.print(value);
    Serial.println(")");
    
    // Interpretación específica según el registro
    if (criticalRegisters[i] == Register::PMIC_DEVICE_ID) {
      interpretDeviceId(value);
    }
    else if (strstr(criticalRegisterNames[i], "VOLT")) {
      interpretVoltageRegister(criticalRegisterNames[i], value);
    }
    else if (strstr(criticalRegisterNames[i], "CTRL")) {
      interpretControlRegister(criticalRegisterNames[i], value);
    }
    else if (strstr(criticalRegisterNames[i], "CHARG") && strstr(criticalRegisterNames[i], "STAT")) {
      interpretChargerStatus(value);
    }
    else if (strstr(criticalRegisterNames[i], "TEMP")) {
      interpretTemperature(criticalRegisterNames[i], value);
    }
    else if (strstr(criticalRegisterNames[i], "_SNS")) {
      interpretSensorVoltage(criticalRegisterNames[i], value);
    }
    
    Serial.println();
    delay(10); // Pequeña pausa entre lecturas
  }
}

// Función para monitoreo en tiempo real usando registros de la biblioteca
void monitorCriticalRegisters() {
  Serial.println("=== MONITOREO EN TIEMPO REAL ===");
  
  uint8_t sw1_volt, sw2_volt, sw1_ctrl, sw2_ctrl, vbat, charg_stat;
  
  if (usingCustomPMIC) {
    sw1_volt = customPMIC.readPMICreg(Register::PMIC_SW1_VOLT);
    sw2_volt = customPMIC.readPMICreg(Register::PMIC_SW2_VOLT);
    sw1_ctrl = customPMIC.readPMICreg(Register::PMIC_SW1_CTRL);
    sw2_ctrl = customPMIC.readPMICreg(Register::PMIC_SW2_CTRL);
    vbat = customPMIC.readPMICreg(Register::CHARGER_BATT_SNS);
    charg_stat = customPMIC.readPMICreg(Register::CHARGER_CHG_SNS);
  } else {
    sw1_volt = PMIC.readPMICreg(Register::PMIC_SW1_VOLT);
    sw2_volt = PMIC.readPMICreg(Register::PMIC_SW2_VOLT);
    sw1_ctrl = PMIC.readPMICreg(Register::PMIC_SW1_CTRL);
    sw2_ctrl = PMIC.readPMICreg(Register::PMIC_SW2_CTRL);
    vbat = PMIC.readPMICreg(Register::CHARGER_BATT_SNS);
    charg_stat = PMIC.readPMICreg(Register::CHARGER_CHG_SNS);
  }
  
  Serial.print("SW1: "); Serial.print((0.4 + (sw1_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw1_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  
  Serial.print("SW2: "); Serial.print((0.4 + (sw2_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw2_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  
  Serial.print("VBAT: "); Serial.print(vbat * 0.025, 3); Serial.print("V | ");
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
    readAndDisplayCriticalRegisters();
    lastFullScan = now;
    Serial.println("=====================================\n");
  }
  
  // Monitoreo rápido cada 2 segundos
  if (now - lastMonitor >= 2000) {
    monitorCriticalRegisters();
    lastMonitor = now;
  }
  
  delay(100);
}


