# Alarma de gas IoT con ESP32

Prototipo académico con ESP32, sensor MQ2 (potenciómetro en Wokwi), LCD 1602 I2C, buzzer y mensajería MQTT. El firmware conserva la detección local aunque no haya conexión al broker.

## Requisitos

- Visual Studio Code con PlatformIO y la extensión Wokwi, o PlatformIO Core instalado.
- Python 3.9 o posterior.
- Mosquitto instalado en el equipo para las pruebas locales.

## Compilar y simular

1. Clona el repositorio y abre su carpeta en Visual Studio Code.
2. Compila el firmware desde la raíz:

   ```powershell
   python -m platformio run
   ```

   PlatformIO descarga las bibliotecas declaradas en `platformio.ini`. El firmware generado queda en `.pio/`, que Git ignora.
3. Inicia Wokwi con **Wokwi: Start Simulator**. `wokwi.toml` apunta al firmware compilado y `diagram.json` contiene el circuito.

## Ejecutar MQTT local

El firmware de ejemplo usa `host.wokwi.internal:1883` para que la simulación Wokwi alcance el broker del equipo. La interfaz Python usa `localhost:1883`. Inicia Mosquitto en una terminal desde la raíz del repositorio:

```powershell
mosquitto -c mosquitto/configuracion_sin_secretos.conf -v
```

En otra terminal, instala la dependencia del cliente y ejecuta la interfaz:

```powershell
python -m pip install -r requirements.txt
python interfaz/cliente_interfaz.py
```

La configuración de Mosquitto permite conexiones anónimas para pruebas locales. No expongas este broker a Internet ni reutilices esta configuración en producción.

## Comandos y tópicos

La interfaz publica `AUTO`, `ARMAR` o `SILENCIAR` en `iot/ef4eea4151/command`. El dispositivo publica telemetría JSON en `iot/ef4eea4151/telemetry`, estado en `iot/ef4eea4151/status` y errores en `iot/ef4eea4151/alert`.

El modo `AUTO` activa la alarma tras 6 lecturas consecutivas de al menos 2650 ADC y la desactiva por debajo de 2646 ADC. El muestreo ocurre cada 1450 ms, la telemetría cada 18 s y la reconexión MQTT se intenta cada 9 s.

## Conexiones

| Componente | ESP32 | Función |
| --- | --- | --- |
| Sensor MQ2 / potenciómetro | GPIO36 (VP) | Entrada analógica |
| Buzzer | GPIO25 | Salida de alarma |
| LCD 1602 I2C | GPIO21 SDA, GPIO22 SCL | Pantalla |

El diagrama detallado está en `diagram.json` y [docs/arquitectura/tabla_conexiones.md](docs/arquitectura/tabla_conexiones.md). Los informes y pruebas de las actividades están en `docs/`.

## Archivos locales y Git

`.gitignore` excluye compilados de PlatformIO, cachés/entornos de Python, configuraciones generadas de VS Code y archivos locales de credenciales. El firmware usa `include/secrets.h` si existe y, en caso contrario, `include/secrets.example.h`. Para credenciales privadas, copia el ejemplo a `include/secrets.h` y edita solo la copia local; no subas `secrets.h`, `.env` ni contraseñas reales.