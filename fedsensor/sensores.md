# Sensores de Presión - Configuración y Especificaciones

## SSCDANN600MDSA3-ND

**Estado:** Funcionando ✅ (EL PIN 0 SE VE CON EL PUNTO DE ARRIBA)

**Protocolo:** SPI (Estándar de Honeywell)

### Pinout:
1. GND
2. Vsupply
3. MISO
4. SCLK
5. SS

temp: -20 a 85 °C<br>
diferencial: ±600 mbar<br>
resolucion: 12 bits<br> (comunicacion de 14 bits)
error band: ±2%<br>
long term stability (1000 hrs): ±0.25%<br>
response time: 0.46 ms<br>
fs: aproximadamente 2khz (pagina 2 datasheet)

**Hoja de datos:** [SSC Series Datasheet](ssc_series_DS.pdf)  
*La función de transferencia está en la página 11.*  
**Para leerlo:** [Honeywell SPI communication](sps-siot-spi-comms-digital-ouptu-pressure-sensors-tn-008202-3-en-ciid-45843.pdf) (SPI de 2 bits, solo presión)


---

## ELVH-015D-HAND-C-P2A4

**Estado:** Funcionando ✅

**Protocolo:** I2C (dirección 0x28)

### Pinout:
1. GND
2. VS 
3. SDA (con pull-up de 1kΩ)
4. SCL (con pull-up de 1kΩ)

temp: 0 a 50 °C<br>
diferencial: -103 a 103 kPa (±15 psi) <br>
resolucion: 14 bits reales  <br> 
error band: ±0.5% FSS <br>
response time: 0.46 ms <br>
fs: aproximadamente 2173 Hz (1 / 0.00046 s). igual que el ssc

**Hoja de datos:** [ELV Series Datasheet](ELV_Series.pdf)

---

## SM4291-HGE-S-500-000

**Estado:** Funcionando ✅

**Protocolo:** Analógico + I2C (dirección 0x6D)

**Referencia de pinout:** Pin 1 es el de arriba a la izquierda con la entrada de presión hacia abajo.

### Pinout:
1. NC 
2. SCL (con pull-up de 4.7kΩ)
3. SDA (con pull-up de 4.7kΩ)
4-6. NC
7. GND (está en el pin 5 del breakout)
8. NC
9. Analog Out
10. Power

temp: -20 a 85°C<br>
diferencial: rango de  25 psi  <br>
resolucion: 16 bits I2C, 10-90% Vdc an <br> 
error band: digital: ±1.5% F.S analogo: ±1% F.S<br>
fs: 2000 hz 

**Hojas de datos:** 
- [SM4291 Datasheet PDF](SM4291-HGE-S-500-000.PDF)
- [Product Datasheet](product-4291-HGE-S-500-000.datasheet.pdf)
- [SM4000/SM1000 Pressure Sensor Datasheet](SM4000_SM1000-pressure-sensor-AMSYS-datasheet.pdf)
---

## 2SMPP-02

**Estado:** Funciona con voltajes muy pequeños ⚠️

**Protocolo:** Salida analógica

**Referencia de pinout:** Pin 1 arriba a la izquierda.

### Pinout:
1. Vout +
2. NC (En el breakout está como GND y el 3 como NC)
3. GND
4. Vout -
5. N-sub
6. Icc

**Características técnicas:**

- **Temperatura de operación:** 0 a 50 °C  
- **Rango de presión:** 0 a 37 kPa  
- **Voltaje de offset:** –2.5 ± 4 mV  
- **Voltaje de span:** 31.0 ± 3.1 mV  
- **Influencia de temperatura (span):** ±1.0 % FS  
- **Influencia de temperatura (offset):** ±3.0 % FS  
- **No linealidad:** 0.8 % FS máx. (0 a 37 kPa)  
- **Histéresis de presión:** 0.5 % FS (0 a 37 kPa)  
- **Influencia de temperatura en el span:**  
    - 0.5 ± 1.0 % FS (0 °C)  
    - 0.3 ± 1.0 % FS (50 °C)  
- **Influencia de temperatura en el offset:**  
    - 0 ± 3.0 % FS (0 a 25 °C)  
    - 0 ± 3.0 % FS (25 a 50 °C)  


**Hoja de datos:** [2SMPP-02 Datasheet](en-2smpp-02.pdf)

---

## 2SMPP-03

**Estado:** Funciona como el anterior ⚠️

**Protocolo:** Salida analógica

**Referencia de pinout:** Pin 1 arriba a la izquierda.

### Pinout:
1. Vout +
2. NC (En el breakout está como GND y el 3 como NC)
3. GND
4. Vout -
5. N-sub
6. Icc

**Notas:** Funciona como el 2SMPP-02. El tema es de hardware - al final es solo un voltaje analógico.

**Características técnicas:**

- **Temperatura de operación:** 0 a 85 °C  
- **Rango de presión:** -50kPa a 50kPa  
**Características técnicas:**

- **Temperatura de operación:** 0 a 85 °C  
- **Rango de presión:** -50 kPa a 50 kPa  
- **Voltaje de offset:** –2.5 ± 4 mV  
- **Voltaje de span:** ±42.0 mV (típico, positivo: 42.0 mV, negativo: –43.0 mV)  
- **Resistencia del puente:** 20.0 kΩ típica (18.0–22.0 kΩ)  
- **Corriente de alimentación:** 100 μA típica  
- **No linealidad:** ±0.8 % FS máx. (positivo: 0.8 % FS, negativo: 0.7 % FS)  
- **Histéresis de presión:** ±0.2 % FS  
- **Influencia de temperatura en el span:**  
    - 0 °C: ±1.0 % FS (positivo), ±0.8 % FS (negativo)  
    - 50 °C: ±1.0 % FS (positivo), ±0.1 % FS (negativo)  
    - 85 °C: ±1.0 % FS (positivo), ±1.2 % FS (negativo)  
- **Influencia de temperatura en el offset:**  
    - 0 °C: ±3.0 % FS  
    - 50 °C: ±3.0 % FS  
    - 85 °C: ±5.0 % FS  







**Hoja de datos:** [2SMPP-03 Datasheet](en-2smpp-03.pdf)

---

## Documentación Adicional

### Otros Datasheets Disponibles:
- [PF1550 Datasheet](PF1550.pdf)



*Última actualización: 23 de septiembre de 2025*
