# Pruebas de la Actividad 3 - IOT-EF4EEA4151

## 1. Validación de Condiciones Analógicas
* **Condición A:** Lectura en 2450 ADC. Sistema estable, actuador apagado.
* **Condición B:** Lectura en 3150 ADC. El sistema cuenta 6 lecturas y activa la alarma, reflejando `"alarm": true` en JSON.
* **Dato no válido:** Lectura fuera de rango rechazada y notificada como alerta.

## 2. Resiliencia de Red e Interrupción
* Se desconectó la red durante 20 segundos.
* El control local operó sin bloqueos gracias a `millis()`.
* Broker emitió LWT `OFFLINE`.
* Reconexión exitosa a los 9 segundos.

## 3. Validación de Comandos
* Comandos `AUTO`, `ARMAR` y `SILENCIAR` cambian el estado y se reflejan en LCD y telemetría.
* `COMANDO_DESCONOCIDO` fue rechazado y publicado en el tópico `alert`, manteniendo el sistema seguro.