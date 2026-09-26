# Interpretación de la Actividad 2 - IOT-EF4EEA4151

## 1. Objetivo del sistema
Este proyecto implementa una alarma de gas basada en ESP32 para detectar niveles anómalos de concentración mediante una lectura analógica del sensor MQ2. La lógica local es crítica, porque el dispositivo debe seguir monitoreando y reaccionando ante una condición peligrosa aunque no exista conectividad con el broker MQTT.

El sistema emplea:
- Potenciómetro de Wokwi como señal analógica del MQ2 en GPIO36 (VP / ADC1_CH0).
- Buzzer en GPIO25 para alerta audible.
- LCD 1602 I2C a 0x27 para visualización local.
- Wi-Fi para conectarse al broker MQTT público.
- Publicación de telemetría JSON cada 18 segundos.
- Recepción de comandos vía MQTT para cambiar el modo operativo.

## 2. Lógica de detección
La lectura analógica se programa cada 1450 ms con `millis()` y no usa `delay()`. La llamada de conexión MQTT es síncrona, por lo que puede pausar temporalmente el programa durante un intento de reconexión.

Se define:
- Umbral de alerta = 2650 ADC
- Histéresis = 4 ADC
- Umbral de desactivación = 2646 ADC

La alarma no se activa al primer valor alto. El sistema requiere seis lecturas consecutivas iguales o mayores a 2650 ADC para confirmar una condición de riesgo. Cualquier lectura menor a 2650 reinicia el contador antes de activar la alarma. Esto reduce falsas alarmas provocadas por variaciones puntuales o ruido eléctrico.

Una vez activa, la alarma se mantiene encendida entre 2646 y 2649 ADC. Solo se apaga cuando la lectura cae por debajo de 2646 ADC.

## 3. Estados del sistema
El sistema opera con estos estados:
- `ESTADO_NORMAL`: sin alarma detectada.
- `ESTADO_EVALUANDO`: se está confirmando la alarma con mediciones altas consecutivas.
- `ESTADO_ALARMA`: la condición de riesgo ha sido validada y el buzzer se activa.

## 4. Modos operativos
Los modos disponibles son:
- `AUTO`: alarma activa cuando el sensor alcanza o supera el umbral y se confirma.
- `ARMAR`: fuerza la sirena para pruebas o operación manual.
- `SILENCIAR`: desactiva el buzzer sin perder la lógica interna del sistema.

## 5. Gestión de riesgos y dificultades
Se aplican estas medidas de robustez:
1. Histéresis: evita oscilaciones repetidas alrededor del umbral.
2. Confirmación por 6 lecturas: elimina disparos por ruido breve.
3. El muestreo local continúa cuando Wi-Fi o MQTT no están disponibles.

Hay dos dificultades importantes: el valor analógico del sensor puede variar y requiere calibración; además, el broker público no ofrece privacidad para este prototipo y la conexión MQTT del ESP32 usa el puerto 1883 sin cifrado. La reconexión se intenta cada 9 segundos, pero `mqttClient.connect()` es síncrona y puede pausar temporalmente el muestreo.

La interfaz LCD muestra el nivel actual, el modo y el estado del sistema, permitiendo diagnóstico local sin depender de la nube.

## 6. Comunicación MQTT
El dispositivo publica su telemetría en:
- `iot/ef4eea4151/telemetry`

y escucha comandos en:
- `iot/ef4eea4151/command`

Los comandos aceptados son:
- `AUTO`
- `ARMAR`
- `SILENCIAR`

El estado de conexión y los errores de comandos se publican en:
- `iot/ef4eea4151/status`

La telemetría JSON incluye `device_id`, `variable`, `value`, `unit`, `mode`, `alarm` y `sequence`. El ESP32 usa MQTT TCP en el puerto 1883; el cliente web de HiveMQ usa WebSockets y debe suscribirse manualmente a los tópicos de telemetría y estado.

## 7. Resultado funcional esperado
Con el sensor en un valor por encima del umbral durante varias muestras consecutivas, el sistema:
- cambia a estado de evaluación,
- activa la alarma cuando se cumplen las 6 confirmaciones,
- enciende el buzzer,
- publica JSON por MQTT,
- y refleja el cambio en el LCD y en la salida serial.

Este enfoque mantiene la alarma local útil aunque falle la red. Sin conexión, no se reciben comandos remotos ni se publica telemetría; además, el prototipo es académico y no reemplaza un detector industrial certificado.