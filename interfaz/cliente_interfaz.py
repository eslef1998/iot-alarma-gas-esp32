import paho.mqtt.client as mqtt
import json
import time

BROKER = "localhost"
PORT = 1883
TOPIC_TELEMETRY = "iot/ef4eea4151/telemetry"
TOPIC_COMMAND   = "iot/ef4eea4151/command"
TOPIC_STATUS    = "iot/ef4eea4151/status"
TOPIC_ALERT     = "iot/ef4eea4151/alert"

def on_connect(client, userdata, flags, reason_code, properties):
    print(f"\n[ESTADO] Conectado al Broker Mosquitto (Código: {reason_code})")
    client.subscribe([(TOPIC_TELEMETRY, 0), (TOPIC_STATUS, 0), (TOPIC_ALERT, 0)])

def on_message(client, userdata, msg):
    try:
        if msg.topic == TOPIC_TELEMETRY:
            data = json.loads(msg.payload.decode('utf-8'))
            print(f"\n[TELEMETRÍA] Secuencia: {data.get('sequence')} | ADC: {data.get('value')} | Modo: {data.get('mode')} | Alarma: {data.get('alarm')}")
        else:
            print(f"\n[{msg.topic.upper()}] {msg.payload.decode('utf-8')}")
    except Exception as e:
        pass

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="Python_Interface")
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT, 60)
client.loop_start()

try:
    while True:
        print("\n1. AUTO | 2. ARMAR | 3. SILENCIAR | 4. COMANDO_DESCONOCIDO | 5. Salir")
        op = input("Selecciona: ")
        if op == '1': client.publish(TOPIC_COMMAND, "AUTO")
        elif op == '2': client.publish(TOPIC_COMMAND, "ARMAR")
        elif op == '3': client.publish(TOPIC_COMMAND, "SILENCIAR")
        elif op == '4': client.publish(TOPIC_COMMAND, "COMANDO_DESCONOCIDO")
        elif op == '5': break
        time.sleep(0.5)
finally:
    client.loop_stop()
    client.disconnect()