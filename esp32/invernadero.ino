#include <WiFi.h>
#include <HTTPClient.h>

// =========================
// CONFIGURACIÓN WIFI
// =========================

const char* ssid = "NOMBRE_DE_TU_WIFI";
const char* password = "CONTRASEÑA_DE_TU_WIFI";

// Dirección de la computadora donde corre FastAPI
const char* servidor = "http://192.168.1.100:8000/lecturas"; // CAMBIAR

// =========================
// PINES
// =========================

const int PIN_HUMEDAD = 34;
const int PIN_LUZ = 35;

// =========================
// CALIBRACIÓN HUMEDAD
// =========================

// Estos valores SON DE EJEMPLO.
// Tu compañero deberá obtenerlos físicamente.

const int HUMEDAD_SECO = 3200;
const int HUMEDAD_HUMEDO = 1400;

// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(115200);

  delay(1000);

  // Configurar ADC
  analogReadResolution(12);

  // Conectar WiFi
  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP de la ESP32: ");
  Serial.println(WiFi.localIP());
}

// =========================
// LOOP
// =========================

void loop() {

  // Leer sensores
  int humedadADC = analogRead(PIN_HUMEDAD);
  int luzADC = analogRead(PIN_LUZ);

  // Convertir humedad a porcentaje
  float humedad = convertirHumedad(humedadADC);

  // Convertir luz a porcentaje relativo
  float luz = convertirLuz(luzADC);

  // Mostrar por monitor serial
  Serial.println("----------------------------");

  Serial.print("Humedad suelo ADC: ");
  Serial.println(humedadADC);

  Serial.print("Humedad suelo: ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.print("Luz ADC: ");
  Serial.println(luzADC);

  Serial.print("Luz ambiental: ");
  Serial.print(luz);
  Serial.println(" %");

  // Enviar al servidor
  enviarDatos(humedad, luz);

  delay(5000);
}

// =========================
// CONVERSIÓN HUMEDAD
// =========================

float convertirHumedad(int valorADC) {

  float porcentaje = map(
    valorADC,
    HUMEDAD_HUMEDO,
    HUMEDAD_SECO,
    100,
    0
  );

  porcentaje = constrain(porcentaje, 0, 100);

  return porcentaje;
}

// =========================
// CONVERSIÓN LUZ
// =========================

float convertirLuz(int valorADC) {

  float porcentaje = (valorADC / 4095.0) * 100.0;

  porcentaje = constrain(porcentaje, 0, 100);

  return porcentaje;
}

// =========================
// ENVIAR DATOS
// =========================

void enviarDatos(float humedad, float luz) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi desconectado");

    return;
  }

  HTTPClient http;

  http.begin(servidor);

  http.addHeader("Content-Type", "application/json");

  // Crear JSON
  String json = "{";

  json += "\"humedad_suelo\":";
  json += String(humedad, 2);

  json += ",";

  json += "\"luz\":";
  json += String(luz, 2);

  json += "}";

  Serial.println("Enviando:");
  Serial.println(json);

  int codigo = http.POST(json);

  Serial.print("Código HTTP: ");
  Serial.println(codigo);

  if (codigo > 0) {

    String respuesta = http.getString();

    Serial.println("Respuesta:");
    Serial.println(respuesta);

  } else {

    Serial.println("Error al enviar datos");

  }

  http.end();
}