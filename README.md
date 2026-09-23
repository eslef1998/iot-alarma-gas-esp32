# Proyecto IoT - Sistema de Monitoreo de Gas y Alerta Temprana

ID de Proyecto: IOT-EF4EEA4151  
Plataforma: ESP32 + Wokwi + PlatformIO  
Protocolo de Red: MQTT sobre Broker Público (HiveMQ)  

---

## 1. Descripción del Sistema

Este proyecto implementa un prototipo IoT de monitoreo y alerta de niveles de gas en laboratorio. El sistema utiliza un microcontrolador ESP32 para adquirir datos analógicos de un sensor MQ2, procesar los valores mediante una máquina de estados finitos no bloqueante y gestionar alertas locales mediante una pantalla LCD 1602 I2C y un buzzer piezoeléctrico.

El firmware integra conectividad remota a la nube mediante el protocolo MQTT, emitiendo telemetría estructurada en formato JSON y permitiendo el control a distancia del modo de operación a través de comandos entrantes.

---

## 2. Arquitectura de Hardware y Conexiones

| Componente | Pin del Módulo | Pin ESP32 | Protocolo / Función |
| :--- | :--- | :--- | :--- |
| Sensor MQ2 | AO (Analog Out) | GPIO34 | Entrada Analógica (ADC1_CH6) |
| Buzzer | Positivo (+) | GPIO18 | Salida Digital / Tono PWM |
| LCD 1602 I2C | SDA | GPIO21 | Datos I2C |
| LCD 1602 I2C | SCL | GPIO22 | Reloj I2C |
| LCD 1602 / MQ2 | VCC / GND | 3V3 / VIN / GND | Alimentación y Referencia |

---

## 3. Especificaciones Técnicas y Lógica de Control

* Muestreo de Gas: Período de 1450 ms implementado con millis() sin bloqueos.
* Umbral de Alerta: Lectura mayor o igual a 2650 unidades ADC.
* Ventana de Confirmación: Se requieren 6 lecturas consecutivas sobre el umbral para validar la activación del estado de ALARMA.
* Histéresis de Desactivación: Margen de 4 unidades ADC. El sistema retorna a estado NORMAL cuando la lectura desciende por debajo de 2646 unidades ADC.
* Cadencia de Telemetría: Envío automático de datos cada 18000 ms (18 segundos).
* Reconexión MQTT: Intento de reestablecimiento de conexión cada 9000 ms (9 segundos) de forma no bloqueante.

---

## 4. Arquitectura de Red y Broker MQTT

* Broker Público: broker.hivemq.com (Puerto 1883)
* Tópico de Telemetría: iot/IOT-EF4EEA4151/telemetry
* Tópico de Comandos: iot/IOT-EF4EEA4151/command
* Tópico de Estado: iot/IOT-EF4EEA4151/status

### Formato del Paquete JSON (Telemetría)

```json
{
  "device_id": "IOT-EF4EEA4151",
  "variable": "nivel analogico de gas",
  "value": 2850,
  "unit": "unidades ADC",
  "mode": "AUTO",
  "alarm": true,
  "sequence": 12
}
```

### Comandos de Control Remoto

* AUTO: Establece el control automático del sistema basado en los umbrales del sensor.
* ARMAR: Fuerza la activación de la alarma sonora de forma manual.
* SILENCIAR: Desactiva el buzzer manteniendo el muestreo del sensor y la emisión de telemetría.

---

## 5. Estructura de Archivos del Proyecto

```text
proyecto_IOT-EF4EEA4151/
├── include/
│   └── secrets.example.h       # Configuración de red y credenciales MQTT
├── src/
│   └── main.cpp                # Código fuente principal en C++
├── docs/
│   ├── interpretacion_actividad_2.md
│   ├── pruebas_actividad_2.md
│   └── arquitectura/
│       └── tabla_conexiones.md
├── diagram.json                # Configuración del circuito en Wokwi
├── wokwi.toml                  # Ruta del firmware compilado para la simulación
├── platformio.ini              # Configuración de entorno y dependencias
└── README.md                   # Documentación técnica del repositorio
```

---

## 6. Instrucciones de Compilación y Ejecución

1. Clonar el repositorio y abrir la carpeta en Visual Studio Code.
2. Compilar el proyecto y descargar las librerías mediante la terminal:
   ```bash
   pio run
   ```
3. Ejecutar la simulación en Wokwi presionando F1 y seleccionando "Wokwi: Start Simulator".