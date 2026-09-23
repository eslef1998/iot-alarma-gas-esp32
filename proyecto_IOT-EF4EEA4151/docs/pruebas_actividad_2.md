# Pruebas de Referencia - Actividad 2

## 1. Matriz de pruebas

| Caso | Condición | Valor / evento | Resultado esperado |
| :--- | :--- | :--- | :--- |
| A | Normal | 200 ADC | Sistema en estado normal, buzzer apagado |
| B | Umbral alcanzado | 2650 ADC | Se inicia estado de evaluación |
| C | Confirmación de alarma | 6 lecturas consecutivas > 2650 | Estado alarma activo |
| D | Histéresis | 2645 ADC | Alarma se desactiva al caer por debajo de 2646 ADC |
| E | Lectura no válida | Valor fuera del rango 0-4095 | Se ignora la muestra y se mantiene el valor anterior |
| F | Pérdida de red | Wi-Fi o MQTT caídos | El sistema sigue funcionando localmente y reintenta conexión cada 9 s |
| G | Comando MQTT | `AUTO`, `ARMAR`, `SILENCIAR` | Modo operativo cambia y se responde en el topic de status |
| H | Comando inválido | `XYZ` | Se responde con error y no cambia el estado del sistema |

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
- La alarma usa un criterio de confirmación de 6 muestras consecutivas, evitando falsos positivos.
- Las reconexiones Wi-Fi y MQTT se gestionan sin bloquear el flujo principal del programa.