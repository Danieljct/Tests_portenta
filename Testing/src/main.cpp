#include <Arduino.h>
#include <Arduino_PF1550.h>
#include <Wire.h>

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

// Diagnóstico de batería
void diagnosticoBateria(uint8_t vbat) {
  Serial.print("Diagnóstico de batería: ");
  if (vbat == 0x00) {
    Serial.println("Sin batería detectada (valor 0x00)");
    Serial.println("Posibles causas:");
    Serial.println("- Batería no conectada correctamente");
    Serial.println("- Batería descargada o dañada");
    Serial.println("- Configuración del PMIC no permite detección");
    Serial.println("- Pull-down en línea de BAT_SNS");
    Serial.println("- Revisa la polaridad y voltaje de la batería");
  } else if (vbat == 0xFF) {
    Serial.println("Registro de batería inválido (valor 0xFF)");
    Serial.println("Posibles causas:");
    Serial.println("- Error de comunicación I2C");
    Serial.println("- PMIC no responde correctamente");
    Serial.println("- Verifica conexiones y dirección I2C");
  } else {
    Serial.print("Valor bruto leído: 0x");
    Serial.print(vbat, HEX);
    Serial.print(" (");
    Serial.print(vbat * 0.025, 3);
    Serial.println(" V)");
    if (vbat * 0.025 < 2.5) {
      Serial.println("Advertencia: Voltaje bajo, batería posiblemente descargada.");
    }
  }
}

// Lectura y despliegue de registros críticos usando PMIC
void readAndDisplayCriticalRegisters() {
  Serial.println("\n=== LECTURA DE REGISTROS CRÍTICOS PF1550 ===\n");
  for (int i = 0; i < NUM_CRITICAL_REGISTERS; i++) {
    uint8_t value = PMIC.readPMICreg(criticalRegisters[i]);
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

// Monitoreo en tiempo real usando PMIC
void monitorCriticalRegisters() {
  Serial.println("=== MONITOREO EN TIEMPO REAL ===");
  uint8_t sw1_volt = PMIC.readPMICreg(Register::PMIC_SW1_VOLT);
  uint8_t sw2_volt = PMIC.readPMICreg(Register::PMIC_SW2_VOLT);
  uint8_t sw1_ctrl = PMIC.readPMICreg(Register::PMIC_SW1_CTRL);
  uint8_t sw2_ctrl = PMIC.readPMICreg(Register::PMIC_SW2_CTRL);
  uint8_t vbat     = PMIC.readPMICreg(Register::CHARGER_BATT_SNS);
  uint8_t charg_stat = PMIC.readPMICreg(Register::CHARGER_CHG_SNS);

  Serial.print("SW1: "); Serial.print((0.4 + (sw1_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw1_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  Serial.print("SW2: "); Serial.print((0.4 + (sw2_volt & 0x3F) * 0.025), 3); Serial.print("V ");
  Serial.print("(EN:"); Serial.print((sw2_ctrl & 0x01) ? "Y" : "N"); Serial.print(") | ");
  Serial.print("VBAT: "); Serial.print(vbat * 0.025, 3); Serial.print("V | ");
  Serial.print("CHG: 0x"); Serial.println(charg_stat, HEX);
  diagnosticoBateria(vbat);
}

void setup() {
  bootM4();
  Serial.begin(115200);
  while (!Serial);

  // Inicializar el bus I2C1 (PB6=SCL, PB7=SDA)
  Serial.println("Configurando bus I2C1 (Wire1) en PB6=SCL, PB7=SDA...");
  Wire1.begin();
  Wire1.setClock(100000); // 100kHz estándar para PMIC

  Serial.println("Inicializando PMIC PF1550...");
  Serial.println("Usando bus I2C1 (SCL: PB6, SDA: PB7) para comunicación con el PMIC.");
  PMIC.begin();
  
  Serial.println("PMIC PF1550 inicializado correctamente.");
  Serial.println("=====================================");
  readAndDisplayCriticalRegisters();
}

void loop() {
  static unsigned long lastFullScan = 0;
  static unsigned long lastMonitor = 0;
  unsigned long now = millis();

  // Escaneo completo cada 30 segundos
  if (now - lastFullScan >= 30000) {
    readAndDisplayCriticalRegisters();
    uint8_t vbat = PMIC.readPMICreg(Register::CHARGER_BATT_SNS);
    diagnosticoBateria(vbat);
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