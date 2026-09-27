## 1. Recorrido de Medición y Comandos

* **Flujo de Telemetría (MQ2 → Python):** El sensor MQ2 entrega una señal analógica leída por el ADC del ESP32. Cada 1450 ms se realiza la toma de muestras, y cada 18 segundos el ESP32 empaqueta el valor ADC junto al estado de la alarma en un payload JSON, publicándolo en el tópico `iot/ef4eea4151/telemetry` de Mosquitto. El script de Python, al estar suscrito a este tópico, recibe la trama y la despliega en pantalla.
* **Flujo de Control (Python → Buzzer):** Al enviar una instrucción desde el menú de Python (como `SILENCIAR`), el script publica una trama JSON en el tópico `iot/ef4eea4151/command`. El broker Mosquitto enruta el mensaje hacia el ESP32, el cual valida el comando y conmuta el estado lógico del Buzzer.

## 2. Autonomía y Lógica de Seguridad en el ESP32

Las reglas de decisión críticas (la lógica de confirmación mediante 6 lecturas consecutivas sobre 2650 ADC, la histéresis de 4 unidades y la respuesta ante comandos no válidos) permanecen estrictamente en el firmware del ESP32. Esta arquitectura garantiza *procesamiento en el borde* (Edge Computing): si el broker Mosquitto se cae o la red se interrumpe, el ESP32 continúa detectando gas y activando el Buzzer de forma física e independiente.

## 3. Rol del Cliente Python, Tópicos MQTT y Estructura JSON

* **Cliente Python:** Cumple el rol de Interfaz Hombre-Máquina (HMI) local para supervisión y envío de instrucciones sin alterar la lógica interna del microcontrolador.
* **Tópicos MQTT:** Organizan el tráfico de datos en canales independientes (`telemetry`, `command`, `status`, `alert`), evitando colisiones de datos y aislando la telemetría de las alarmas.
* **Estructura JSON:** Garantiza la interoperabilidad de datos entre lenguajes (C++ en la ESP32 y Python en la computadora), estandarizando tipos de datos, enteros, flotantes y valores booleanos.