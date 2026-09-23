#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.example.h"

// ==========================================
// CONFIGURACIÓN DE PINES Y CONSTANTES
// ==========================================
const uint8_t PIN_GAS = 34;
const uint8_t PIN_BUZZER = 18;

const char CODIGO_PROYECTO[] = "IOT-EF4EEA4151";
const uint16_t UMBRAL_ALERTA = 2650;
const uint8_t MARGEN_HISTESIS = 4;
const uint16_t UMBRAL_DESACTIVACION = UMBRAL_ALERTA - MARGEN_HISTESIS;
const uint16_t INTERVALO_MUESTREO = 1450;
const uint8_t LECTURAS_REQUERIDAS = 6;
const uint16_t FRECUENCIA_TONO = 1000;

const unsigned long INTERVALO_TELEMETRIA = 18000;
const unsigned long INTERVALO_RECONEXION_MQTT = 9000;

const char TOPIC_TELEMETRY[] = "iot/IOT-EF4EEA4151/telemetry";
const char TOPIC_COMMAND[] = "iot/IOT-EF4EEA4151/command";
const char TOPIC_STATUS[] = "iot/IOT-EF4EEA4151/status";

enum EstadoSistema {
  ESTADO_NORMAL,
  ESTADO_EVALUANDO,
  ESTADO_ALARMA
};

enum ModoOperacion {
  MODO_AUTO,
  MODO_ARMAR,
  MODO_SILENCIAR
};

EstadoSistema estadoActual = ESTADO_NORMAL;
ModoOperacion modoActual = MODO_AUTO;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long ultimaLecturaMs = 0;
unsigned long ultimaTelemetriaMs = 0;
unsigned long ultimaReconexionMQTTMs = 0;

uint8_t contadorLecturasAltas = 0;
uint16_t ultimaLecturaGas = 0;
bool lecturaValida = false;
uint32_t secuenciaTelemetria = 0;

void procesarMuestraGas();
void gestionarActuadores();
void actualizarInterfazLCD();
void conectarWiFi();
void reconectarMQTT();
void callback(char* topic, byte* payload, unsigned int length);
void publicarTelemetria();
const char* obtenerTextoModo();

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("==========================================");
  Serial.print("INICIALIZANDO PROYECTO: ");
  Serial.println(CODIGO_PROYECTO);
  Serial.println("==========================================");

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_GAS, INPUT);
  gestionarActuadores();

  conectarWiFi();
  client.setServer(SECRET_MQTT_SERVER, SECRET_MQTT_PORT);
  client.setCallback(callback);
  actualizarInterfazLCD();
}

void loop() {
  unsigned long tiempoActualMs = millis();

  if (tiempoActualMs - ultimaLecturaMs >= INTERVALO_MUESTREO) {
    ultimaLecturaMs = tiempoActualMs;
    procesarMuestraGas();
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      if (tiempoActualMs - ultimaReconexionMQTTMs >= INTERVALO_RECONEXION_MQTT) {
        ultimaReconexionMQTTMs = tiempoActualMs;
        reconectarMQTT();
      }
    } else {
      client.loop();

      if (tiempoActualMs - ultimaTelemetriaMs >= INTERVALO_TELEMETRIA) {
        ultimaTelemetriaMs = tiempoActualMs;
        publicarTelemetria();
      }
    }
  }
}

void procesarMuestraGas() {
  int lecturaRaw = analogRead(PIN_GAS);

  lecturaValida = (lecturaRaw >= 0 && lecturaRaw <= 4095);
  ultimaLecturaGas = lecturaValida ? (uint16_t)lecturaRaw : 0;

  Serial.print("Variable: nivel analogico de gas | Valor: ");
  Serial.print(ultimaLecturaGas);
  Serial.print(" | Unidad: unidades ADC | Validez: ");
  Serial.println(lecturaValida ? "VALIDO" : "NO_VALIDO");

  if (!lecturaValida) {
    return;
  }

  switch (estadoActual) {
    case ESTADO_NORMAL:
      if (ultimaLecturaGas >= UMBRAL_ALERTA) {
        contadorLecturasAltas = 1;
        estadoActual = ESTADO_EVALUANDO;
      }
      break;

    case ESTADO_EVALUANDO:
      if (ultimaLecturaGas >= UMBRAL_ALERTA) {
        contadorLecturasAltas++;
        if (contadorLecturasAltas >= LECTURAS_REQUERIDAS) {
          estadoActual = ESTADO_ALARMA;
        }
      } else {
        contadorLecturasAltas = 0;
        estadoActual = ESTADO_NORMAL;
      }
      break;

    case ESTADO_ALARMA:
      if (ultimaLecturaGas < UMBRAL_DESACTIVACION) {
        contadorLecturasAltas = 0;
        estadoActual = ESTADO_NORMAL;
      }
      break;
  }

  gestionarActuadores();
  actualizarInterfazLCD();
}

void gestionarActuadores() {
  if (modoActual == MODO_SILENCIAR) {
    noTone(PIN_BUZZER);
  } else if (modoActual == MODO_ARMAR) {
    tone(PIN_BUZZER, FRECUENCIA_TONO);
  } else {
    if (estadoActual == ESTADO_ALARMA) {
      tone(PIN_BUZZER, FRECUENCIA_TONO);
    } else {
      noTone(PIN_BUZZER);
    }
  }
}

void actualizarInterfazLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Gas:");
  if (ultimaLecturaGas < 1000) {
    lcd.print(" ");
  }
  lcd.print(ultimaLecturaGas);
  lcd.print(" M:");
  lcd.print(obtenerTextoModo());

  lcd.setCursor(0, 1);
  if (estadoActual == ESTADO_ALARMA) {
    lcd.print("ESTADO: ALARMA  ");
  } else if (estadoActual == ESTADO_EVALUANDO) {
    lcd.print("CONFIRM:");
    lcd.print(contadorLecturasAltas);
    lcd.print("/6    ");
  } else {
    lcd.print("ESTADO: NORMAL  ");
  }
}

void conectarWiFi() {
  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(SECRET_SSID);
  WiFi.begin(SECRET_SSID, SECRET_PASS);
}

void reconectarMQTT() {
  Serial.print("Intentando conexion MQTT...");
  if (client.connect(SECRET_MQTT_CLIENT_ID)) {
    Serial.println("Conectado!");
    client.subscribe(TOPIC_COMMAND);
  } else {
    Serial.print("Fallo, rc=");
    Serial.println(client.state());
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";
  for (unsigned int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  mensaje.trim();

  Serial.print("Comando recibido: ");
  Serial.println(mensaje);

  if (mensaje == "AUTO") {
    modoActual = MODO_AUTO;
    client.publish(TOPIC_STATUS, "OK: MODO_AUTO");
  } else if (mensaje == "ARMAR") {
    modoActual = MODO_ARMAR;
    client.publish(TOPIC_STATUS, "OK: MODO_ARMAR");
  } else if (mensaje == "SILENCIAR") {
    modoActual = MODO_SILENCIAR;
    client.publish(TOPIC_STATUS, "OK: MODO_SILENCIAR");
  } else {
    client.publish(TOPIC_STATUS, "ERROR: COMANDO_DESCONOCIDO");
  }

  gestionarActuadores();
  actualizarInterfazLCD();
}

void publicarTelemetria() {
  JsonDocument doc;
  doc["device_id"] = CODIGO_PROYECTO;
  doc["variable"] = "nivel analogico de gas";
  doc["value"] = ultimaLecturaGas;
  doc["unit"] = "unidades ADC";
  doc["mode"] = obtenerTextoModo();
  doc["alarm"] = (estadoActual == ESTADO_ALARMA);
  doc["sequence"] = ++secuenciaTelemetria;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  client.publish(TOPIC_TELEMETRY, jsonBuffer);
  Serial.print("Telemetria enviada: ");
  Serial.println(jsonBuffer);
}

const char* obtenerTextoModo() {
  switch (modoActual) {
    case MODO_ARMAR:
      return "ARMAR";
    case MODO_SILENCIAR:
      return "SILEN";
    default:
      return "AUTO";
  }
}