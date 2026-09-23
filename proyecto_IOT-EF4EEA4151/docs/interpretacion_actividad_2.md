# Interpretación de la Actividad 2 - IOT-EF4EEA4151

## 1. Objetivo del sistema
Este proyecto implementa una alarma de gas basada en ESP32 para detectar niveles anómalos de concentración mediante una lectura analógica del sensor MQ2. La lógica local es crítica, porque el dispositivo debe seguir monitoreando y reaccionando ante una condición peligrosa aunque no exista conectividad con el broker MQTT.

El sistema emplea:
- Sensor MQ2 conectado al ADC1 del ESP32 en GPIO34.
- Buzzer en GPIO18 para alerta audible.
- LCD 1602 I2C a 0x27 para visualización local.
- Wi-Fi para conectarse al broker MQTT público.
- Publicación de telemetría JSON cada 18 segundos.
- Recepción de comandos vía MQTT para cambiar el modo operativo.

## 2. Lógica de detección
La lectura analógica del sensor se toma cada 1450 ms, de forma no bloqueante con `millis()`. Esto evita detener la ejecución del programa mientras se gestionan tareas de red y la interfaz local.

Se define:
- Umbral de alerta = 2650 ADC
- Histéresis = 4 ADC
- Umbral de desactivación = 2646 ADC

La alarma no se activa al primer valor alto. El sistema requiere seis lecturas consecutivas por encima del umbral para confirmar una condición real de riesgo. Esto reduce falsas alarmas provocadas por variaciones puntuales o ruido eléctrico.

Cuando la lectura cae por debajo de 2646 ADC, el sistema vuelve a `ESTADO_NORMAL` y apaga la alarma.

## 3. Estados del sistema
El sistema opera con estos estados:
- `ESTADO_NORMAL`: sin alarma detectada.
- `ESTADO_EVALUANDO`: se está confirmando la alarma con mediciones altas consecutivas.
- `ESTADO_ALARMA`: la condición de riesgo ha sido validada y el buzzer se activa.

## 4. Modos operativos
Los modos disponibles son:
- `AUTO`: alarma activa solo cuando el sensor supera el umbral y se confirma.
- `ARMAR`: fuerza la sirena para pruebas o operación manual.
- `SILENCIAR`: desactiva el buzzer sin perder la lógica interna del sistema.

## 5. Gestión de riesgos y robustez
Se aplican tres medidas de seguridad:
1. Histéresis: evita oscilaciones repetidas alrededor del umbral.
2. Confirmación por 6 lecturas: elimina disparos por ruido breve.
3. Reconexión no bloqueante: si falla Wi-Fi o MQTT, el sistema no se detiene; el muestreo y la lógica local continúan funcionando.

Además, la interfaz LCD muestra el nivel actual, el modo y el estado del sistema, permitiendo diagnóstico local sin depender de la nube.

## 6. Comunicación MQTT
El dispositivo publica su telemetría en:
- `iot/IOT-EF4EEA4151/telemetry`

y escucha comandos en:
- `iot/IOT-EF4EEA4151/command`

Los comandos aceptados son:
- `AUTO`
- `ARMAR`
- `SILENCIAR`

La respuesta de confirmación se envía a:
- `iot/IOT-EF4EEA4151/status`

## 7. Resultado funcional esperado
Con el sensor en un valor por encima del umbral durante varias muestras consecutivas, el sistema:
- cambia a estado de evaluación,
- activa la alarma cuando se cumplen las 6 confirmaciones,
- enciende el buzzer,
- publica JSON por MQTT,
- y refleja el cambio en el LCD y en la salida serial.

Este enfoque mantiene el sistema útil para laboratorio, simulación y validación de rendimiento sin depender de una red sólida para su operación básica.