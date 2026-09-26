# Proyecto IoT - Sistema de Monitoreo de Gas y Alerta Temprana

ID de Proyecto: IOT-EF4EEA4151  
Plataforma: ESP32 + Wokwi + PlatformIO  
Protocolo de Red: MQTT sobre Broker Público (HiveMQ)  

---

## 1. Descripción del Sistema

Este proyecto implementa un prototipo IoT de monitoreo y alerta de niveles de gas en laboratorio. El sistema utiliza un ESP32 para leer una señal analógica simulada, evaluar reglas por modo de operación y gestionar alertas locales mediante una pantalla LCD 1602 I2C y un buzzer piezoeléctrico.

El firmware integra conectividad remota a la nube mediante el protocolo MQTT, emitiendo telemetría estructurada en formato JSON y permitiendo el control a distancia del modo de operación a través de comandos entrantes.

---

## 2. Arquitectura de Hardware y Conexiones

| Componente | Pin del Módulo | Pin ESP32 | Protocolo / Función |
| :--- | :--- | :--- | :--- |
| Sensor MQ2 / potenciómetro Wokwi | AO / SIG | GPIO36 (VP) | Entrada Analógica (ADC1_CH0) |
| Buzzer | Positivo (+) | GPIO25 | Salida digital de alarma |
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
* Reconexión: el intento se programa cada 9000 ms con `millis()`. La llamada `mqttClient.connect()` es síncrona y puede pausar temporalmente el muestreo mientras espera respuesta.

---

## 4. Arquitectura de Red y Broker MQTT

* Broker Público: broker.hivemq.com (Puerto 1883)
* Tópico de Telemetría: `iot/ef4eea4151/telemetry`
* Tópico de Comandos: `iot/ef4eea4151/command`
* Tópico de Estado y errores: `iot/ef4eea4151/status`

El ESP32 usa MQTT sobre TCP en el puerto 1883. Para las pruebas manuales, HiveMQ Web Client se conecta por WebSockets (puerto 8000 o 8884); desde ese cliente hay que suscribirse a telemetría y estado, y publicar los comandos en el tópico correspondiente.

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
.
├── .gitignore
├── .vscode/
│   └── extensions.json
├── diagram.json
├── docs/
│   ├── arquitectura/
│   │   └── tabla_conexiones.md
│   ├── interpretacion_actividad_2.md
│   ├── interpretacion_asignacion.md
│   └── pruebas_actividad_2.md
├── include/
│   └── secrets.example.h
├── insumos_generador/
│   └── proyecto_iot-ef4eea4151.pdf
├── src/
│   └── main.cpp
├── platformio.ini
├── README.md
└── wokwi.toml
```

---

## 6. Instrucciones de Compilación y Ejecución

1. Clonar el repositorio y abrir la carpeta raíz en Visual Studio Code.
  ```bash
   git clone https://github.com/eslef1998/iot-alarma-gas-esp32.git
   ```
2. Compilar el proyecto y descargar las librerías desde la carpeta raíz:
   ```bash
   python -m platformio run
   ```
3. Ejecutar la simulación en Wokwi seleccionando "Wokwi: Start Simulator".
   ```bash
   
   ```