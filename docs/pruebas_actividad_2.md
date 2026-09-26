# Pruebas de Referencia - Actividad 2

## 1. Matriz de pruebas

| Caso | Condición | Valor / evento | Resultado esperado |
| :--- | :--- | :--- | :--- |
| A | Normal | 200 ADC | Sistema en estado normal, buzzer apagado |
| B | Umbral alcanzado | 2650 ADC | Se inicia estado de evaluación (conteo 1/6) |
| C | Confirmación de alarma | 6 lecturas consecutivas >= 2650 ADC | Buzzer y estado de alarma activos |
| D | Racha interrumpida antes de alarma | Tras varias lecturas altas, leer 2649 ADC | Contador vuelve a 0; alarma continúa segura |
| E | Retención por histéresis | Con alarma activa, leer entre 2646 y 2649 ADC | La alarma permanece activa |
| F | Retorno seguro | Con alarma activa, leer 2645 ADC | Alarma se desactiva y contador vuelve a 0 |
| G | LCD al iniciar | Encender la simulación | Fila 0 muestra `ADC: 0 AUTO`; fila 1, `ALARM: SEGURA` |
| H | Lectura no válida | Valor fuera del rango 0-4095 | Se ignora la muestra y se mantiene el valor anterior |
| I | Pérdida de red | Wi-Fi o MQTT caídos | La lógica local continúa; MQTT reintenta cada 9 s y `connect()` puede bloquear temporalmente |
| J | Comando MQTT | `AUTO`, `ARMAR`, `SILENCIAR` en `iot/ef4eea4151/command` | Cambia el modo; validar el estado en LCD y telemetría/status |
| K | Comando inválido | `INVALIDO` | `iot/ef4eea4151/status` recibe `COMANDO_DESCONOCIDO` |

## 2. Ejemplo de telemetría emitida

```json
{
  "device_id": "IOT-EF4EEA4151",
  "variable": "nivel analogico de gas",
  "value": 2450,
  "unit": "unidades ADC",
  "mode": "AUTO",
  "alarm": false,
  "sequence": 1
}
```

## 3. Observaciones de validación
- La lectura se toma cada 1450 ms.
- La publicación de MQTT se realiza cada 18 segundos.
- La alarma usa 6 muestras consecutivas iguales o mayores a 2650 ADC.
- Cualquier lectura menor a 2650 interrumpe la racha antes de activar la alarma; con la alarma activa, esta se conserva entre 2646 y 2649 ADC.
- Para pruebas manuales, suscribirse en HiveMQ Web Client a `iot/ef4eea4151/telemetry` y `iot/ef4eea4151/status`. El cliente web usa WebSockets; el ESP32 usa MQTT TCP en el puerto 1883.