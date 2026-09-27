#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

// ==========================================
// PARÁMETROS ASIGNADOS (SECCIÓN 3.3 Y 3.4)
// ==========================================
const char* DEVICE_ID            = "IOT-EF4EEA4151";
const char* VARIABLE_NAME        = "nivel analogico de gas";
const char* UNIT_NAME            = "unidades ADC";

const int   PIN_SENSOR_MQ2       = 36;
const int   PIN_BUZZER           = 25;

const uint16_t UMBRAL_PRINCIPAL  = 2650;
const uint16_t MARGEN_RETORNO    = 4;
const uint16_t UMBRAL_RETORNO    = UMBRAL_PRINCIPAL - MARGEN_RETORNO;

const uint32_t INTERVALO_MUESTREO  = 1450;
const uint32_t PERIODO_PUBLICACION = 18000;
const uint32_t INTERVALO_RECONEXION = 9000;
const uint8_t  CONFIRMACIONES_REQ  = 6;

const char* TOPIC_TELEMETRY = "iot/ef4eea4151/telemetry";
const char* TOPIC_COMMAND   = "iot/ef4eea4151/command";
const char* TOPIC_STATUS    = "iot/ef4eea4151/status";
const char* TOPIC_ALERT     = "iot/ef4eea4151/alert";

// ==========================================
// OBJETOS Y VARIABLES
// ==========================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

uint32_t lastSampleTime         = 0;
uint32_t lastPublishTime        = 0;
uint32_t lastReconnectAttempt   = 0;

uint16_t currentReading         = 0;
uint8_t  consecutiveHighCount   = 0;
bool     showConfirmationComplete = false;
uint32_t confirmationCompleteAt = 0;
bool     buzzerActive           = false;
String   currentMode            = "AUTO";
bool     alarmState             = false;
uint32_t sequenceNumber         = 0;

void setupWiFiAndMQTT();
void handleNetworkReconnection();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void processLocalLogic();
void publishTelemetry();
void updateLCD();
void applyAlarmOutput();

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    updateLCD();

    setupWiFiAndMQTT();
}

void loop() {
    uint32_t currentMillis = millis();

    // 1. Muestreo Local (1450 ms)
    if (currentMillis - lastSampleTime >= INTERVALO_MUESTREO) {
        lastSampleTime = currentMillis;
        processLocalLogic();
        updateLCD();
    }

    if (showConfirmationComplete && currentMillis - confirmationCompleteAt >= 1000) {
        showConfirmationComplete = false;
        updateLCD();
    }

    // 2. Reconexión MQTT (9s)
    if (!WiFi.isConnected() || !mqttClient.connected()) {
        if (currentMillis - lastReconnectAttempt >= INTERVALO_RECONEXION) {
            lastReconnectAttempt = currentMillis;
            handleNetworkReconnection();
        }
    } else {
        mqttClient.loop();
    }

    // 3. Publicación (18s)
    if (currentMillis - lastPublishTime >= PERIODO_PUBLICACION) {
        lastPublishTime = currentMillis;
        publishTelemetry();
    }
}

void processLocalLogic() {
    int rawValue = analogRead(PIN_SENSOR_MQ2);

    if (rawValue < 0 || rawValue > 4095) {
        Serial.println("[ERROR] Lectura fuera del rango.");
        if(mqttClient.connected()){
            mqttClient.publish(TOPIC_ALERT, "{\"error\":\"Dato no valido de referencia\"}");
        }
        return;
    }

    currentReading = rawValue;

    if (currentMode == "AUTO") {
        if (currentReading >= UMBRAL_PRINCIPAL) {
            if (consecutiveHighCount < CONFIRMACIONES_REQ) {
                consecutiveHighCount++;
            }
            if (consecutiveHighCount >= CONFIRMACIONES_REQ && !alarmState) {
                alarmState = true;
                showConfirmationComplete = true;
                confirmationCompleteAt = millis();
            }
        } else {
            consecutiveHighCount = 0;
            if (alarmState && currentReading < UMBRAL_RETORNO) {
                alarmState = false;
                showConfirmationComplete = false;
            }
        }
    } else if (currentMode == "ARMAR") {
        alarmState = true;
    } else if (currentMode == "SILENCIAR") {
        alarmState = false;
    }

    applyAlarmOutput();
}

void applyAlarmOutput() {
    if (buzzerActive == alarmState) return;
    buzzerActive = alarmState;
    if (buzzerActive) tone(PIN_BUZZER, 2000);
    else noTone(PIN_BUZZER);
}

void setupWiFiAndMQTT() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

void handleNetworkReconnection() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        return;
    }

    if (!mqttClient.connected()) {
        // Última Voluntad (LWT) en caso de desconexión abrupta
        if (mqttClient.connect(DEVICE_ID, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 1, true, "{\"device_id\":\"IOT-EF4EEA4151\",\"status\":\"OFFLINE\"}")) {
            mqttClient.subscribe(TOPIC_COMMAND);
            mqttClient.publish(TOPIC_STATUS, "{\"device_id\":\"IOT-EF4EEA4151\",\"status\":\"ONLINE\"}", true);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    String cmd = String(message);
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "AUTO" || cmd == "ARMAR" || cmd == "SILENCIAR") {
        if (cmd != currentMode) {
            consecutiveHighCount = 0;
            showConfirmationComplete = false;
            alarmState = (cmd == "ARMAR");
        }
        currentMode = cmd;
        applyAlarmOutput();
        updateLCD();
    } else {
        // Desvío de comando desconocido a topic de alertas
        StaticJsonDocument<128> doc;
        doc["device_id"] = DEVICE_ID;
        doc["error"] = "COMANDO_DESCONOCIDO";
        doc["raw_received"] = cmd;
        char buffer[128];
        serializeJson(doc, buffer);
        mqttClient.publish(TOPIC_ALERT, buffer);
    }
}

void publishTelemetry() {
    if (!mqttClient.connected()) return;

    sequenceNumber++;
    StaticJsonDocument<256> doc;
    doc["device_id"] = DEVICE_ID;
    doc["variable"]  = VARIABLE_NAME;
    doc["value"]     = (float)currentReading; // Casting a float exigido
    doc["unit"]      = UNIT_NAME;
    doc["mode"]      = currentMode;
    doc["alarm"]     = alarmState;
    doc["sequence"]  = sequenceNumber;

    char buffer[256];
    serializeJson(doc, buffer);
    mqttClient.publish(TOPIC_TELEMETRY, buffer);
}

void updateLCD() {
    lcd.setCursor(0, 0);
    lcd.print("ADC:");
    lcd.print(currentReading);
    lcd.print("    ");
    lcd.setCursor(10, 0);
    lcd.print(currentMode.substring(0, 6));

    lcd.setCursor(0, 1);
    if (showConfirmationComplete) lcd.print("CONF: 6/6       ");
    else if (alarmState) lcd.print("ALARM: ACTIVADA ");
    else if (consecutiveHighCount > 0 && currentMode == "AUTO") {
        lcd.print("CONF: "); lcd.print(consecutiveHighCount); lcd.print("/"); lcd.print(CONFIRMACIONES_REQ); lcd.print("        ");
    } else lcd.print("ALARM: SEGURA   ");
}