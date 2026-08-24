#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ==========================================
// CONFIGURACIÓN DE PINES Y CONSTANTES (1.3)
// ==========================================
const uint8_t PIN_GAS = 34;      
const uint8_t PIN_BUZZER = 18;   

// Parámetros del proyecto IOT-EF4EEA4151
const char CODIGO_PROYECTO[] = "IOT-EF4EEA4151";
const uint16_t UMBRAL_ALERTA = 2650;        // [ADC]
const uint8_t  MARGEN_HISTESIS = 4;         // [ADC]
const uint16_t UMBRAL_DESACTIVACION = UMBRAL_ALERTA - MARGEN_HISTESIS; // 2646 ADC
const uint16_t INTERVALO_MUESTREO = 1450;   // [ms]
const uint8_t  LECTURAS_REQUERIDAS = 6;      // Lecturas consecutivas
const uint16_t FRECUENCIA_TONO = 1000;      // [Hz]

enum EstadoSistema {
  ESTADO_NORMAL,
  ESTADO_EVALUANDO,
  ESTADO_ALARMA
};

EstadoSistema estadoActual = ESTADO_NORMAL;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long ultimaLecturaMs = 0;
uint8_t contadorLecturasAltas = 0;

void actualizarInterfazLCD(uint16_t lecturaADC, EstadoSistema estado);
void gestionarActuadores(EstadoSistema estado);
void procesarMuestraGas();

void setup() {
  Serial.begin(115200);
  delay(500);

  // Mensaje de arranque obligatorio con el código del proyecto
  Serial.println("==========================================");
  Serial.print("INICIALIZANDO PROYECTO: ");
  Serial.println(CODIGO_PROYECTO);
  Serial.println("==========================================");

  Wire.begin(21, 22); 

  lcd.init();
  lcd.backlight();

  // Configuración simple del pin del buzzer
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Estado seguro inicial: Buzzer silenciado
  gestionarActuadores(ESTADO_NORMAL);
}

void loop() {
  unsigned long tiempoActualMs = millis();

  if (tiempoActualMs - ultimaLecturaMs >= INTERVALO_MUESTREO) {
    ultimaLecturaMs = tiempoActualMs;
    procesarMuestraGas();
  }
}

void procesarMuestraGas() {
  int lecturaRaw = analogRead(PIN_GAS);

  // Validación elemental de la lectura analógica (ESP32 ADC: 0 a 4095)
  bool esValido = (lecturaRaw >= 0 && lecturaRaw <= 4095);
  uint16_t lecturaGas = esValido ? (uint16_t)lecturaRaw : 0;

  // Formato de salida serial requerido por la actividad
  Serial.print("Variable: nivel analogico de gas | Valor: ");
  Serial.print(lecturaGas);
  Serial.print(" | Unidad: unidades ADC | Validez: ");
  Serial.println(esValido ? "VALIDO" : "NO_VALIDO");

  if (!esValido) return;

  // Lógica de confirmación de 6 lecturas consecutivas
  switch (estadoActual) {
    case ESTADO_NORMAL:
      if (lecturaGas >= UMBRAL_ALERTA) {
        contadorLecturasAltas = 1;
        estadoActual = ESTADO_EVALUANDO;
      }
      break;

    case ESTADO_EVALUANDO:
      if (lecturaGas >= UMBRAL_ALERTA) {
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
      if (lecturaGas < UMBRAL_DESACTIVACION) {
        contadorLecturasAltas = 0;
        estadoActual = ESTADO_NORMAL;
      }
      break;
  }

  gestionarActuadores(estadoActual);
  actualizarInterfazLCD(lecturaGas, estadoActual);
}

void gestionarActuadores(EstadoSistema estado) {
  if (estado == ESTADO_ALARMA) {
    tone(PIN_BUZZER, FRECUENCIA_TONO); // Mantiene el sonido sostenido en Wokwi
  } else {
    noTone(PIN_BUZZER); // Apaga completamente el sonido
  }
}

void actualizarInterfazLCD(uint16_t lecturaADC, EstadoSistema estado) {
  lcd.setCursor(0, 0);
  lcd.printf("Gas: %4d ADC   ", lecturaADC);

  lcd.setCursor(0, 1);
  if (estado == ESTADO_ALARMA) {
    lcd.print("ESTADO: ALARMA  ");
  } else if (estado == ESTADO_EVALUANDO) {
    lcd.printf("CONFIRM: %d/6   ", contadorLecturasAltas);
  } else {
    lcd.print("ESTADO: NORMAL  ");
  }
}