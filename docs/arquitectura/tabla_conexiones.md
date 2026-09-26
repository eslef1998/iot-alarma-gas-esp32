# Tabla de conexiones - Actividad 2

## 1. Diagrama funcional

- Sensor MQ2 (potenciómetro de simulación) -> ESP32 GPIO36 (VP / ADC1_CH0)
- Buzzer -> ESP32 GPIO25
- LCD 1602 I2C -> ESP32 GPIO21 (SDA) y GPIO22 (SCL)

## 2. Tabla de pines

| Componente | Pin / conexión | ESP32 | Descripción |
| :--- | :--- | :--- | :--- |
| MQ2 / sensor analógico | SIG | GPIO36 (VP) | Entrada analógica principal (ADC1_CH0) |
| MQ2 / sensor analógico | VCC | 3V3 | Alimentación del sensor |
| MQ2 / sensor analógico | GND | GND | Tierra del sensor |
| Buzzer | + | GPIO25 | Salida de tono para activar la alarma |
| Buzzer | - | GND | Tierra del buzzer |
| LCD 1602 I2C | SDA | GPIO21 | Línea de datos I2C |
| LCD 1602 I2C | SCL | GPIO22 | Línea de reloj I2C |
| LCD 1602 I2C | VCC | VIN | Alimentación del módulo LCD |
| LCD 1602 I2C | GND | GND | Tierra del módulo |

## 3. Observaciones de hardware
- El sensor MQ2 se modela como una entrada analógica de 0 a 4095 ADC.
- La pantalla LCD usa protocolo I2C con dirección `0x27`.
- El buzzer se activa con `tone()` en GPIO25 y se detiene con `noTone()`.
- La conexión de alimentación del LCD se hace por VIN para mantener compatibilidad con la placa ESP32 DevKit v1.

## 4. Representación de conexión

```text
Sensor MQ2 (SIG) -> GPIO36 (VP)
Sensor MQ2 (VCC) -> 3V3
Sensor MQ2 (GND) -> GND

Buzzer (+) -> GPIO25
Buzzer (-) -> GND

LCD SDA -> GPIO21
LCD SCL -> GPIO22
LCD VCC -> VIN
LCD GND -> GND
```