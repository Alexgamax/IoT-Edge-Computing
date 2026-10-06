#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Totalplay-2.4G-4a08";
const char* password = "nVm2476ndThz4EfM";

// Direccion
const char* servidor = "http://192.168.100.17:8000/lecturas"; // CAMBIAR POR TU IP

const int PIN_HUMEDAD = 34; // Capacitive Soil Moisture Sensor
const int PIN_LUZ = 35;     // Fotorresistencia (LDR)
const int PIN_LM35 = 32;    // Sensor de Temperatura LM35 (Pin Central VOUT)

const int HUMEDAD_SECO = 3200;
const int HUMEDAD_HUMEDO = 1400;

float convertirHumedad(int valorADC);
float convertirLuz(int valorADC);
float convertirTemperatura(int valorADC);
void enviarDatos(float humedad, float luz, float temperatura);

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado con éxito");
  Serial.print("IP asignada a la ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  int humedadADC = analogRead(PIN_HUMEDAD);
  int luzADC = analogRead(PIN_LUZ);
  int tempADC = analogRead(PIN_LM35);

  float humedad = convertirHumedad(humedadADC);
  float luz = convertirLuz(luzADC);
  float temperatura = convertirTemperatura(tempADC);

  Serial.println("\n--- Lecturas de Sensores ---");

  Serial.print("Humedad Suelo ADC: ");
  Serial.print(humedadADC);
  Serial.print(" | ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.print("Luz Ambiental ADC: ");
  Serial.print(luzADC);
  Serial.print(" | ");
  Serial.print(luz);
  Serial.println(" %");

  Serial.print("Temperatura LM35 ADC: ");
  Serial.print(tempADC);
  Serial.print(" | ");
  Serial.print(temperatura);
  Serial.println(" °C");

  enviarDatos(humedad, luz, temperatura);

  delay(5000); 
}

float convertirHumedad(int valorADC) {
  float porcentaje = map(
    valorADC,
    HUMEDAD_HUMEDO,
    HUMEDAD_SECO,
    100,
    0
  );

  return constrain(porcentaje, 0, 100);
}

float convertirLuz(int valorADC) {
  float porcentaje = (valorADC / 4095.0) * 100.0;
  return constrain(porcentaje, 0, 100);
}

float convertirTemperatura(int valorADC) {
  // El ADC mide de 0 a 3.3V (3300 mV) en 4095 pasos.
  // El LM35 entrega 10 mV por cada 1 °C.
  float voltajemV = (valorADC / 4095.0) * 3300.0;
  float tempC = voltajemV / 10.0;
  return tempC;
}

void enviarDatos(float humedad, float luz, float temperatura) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: WiFi desconectado. Reintentando...");
    return;
  }

  HTTPClient http;
  http.begin(servidor);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"humedad_suelo\":" + String(humedad, 2) + ",";
  json += "\"luz\":" + String(luz, 2) + ",";
  json += "\"temperatura\":" + String(temperatura, 2);
  json += "}";

  Serial.println("Enviando JSON:");
  Serial.println(json);

  int codigoRespuesta = http.POST(json);

  Serial.print("Código de respuesta HTTP: ");
  Serial.println(codigoRespuesta);

  if (codigoRespuesta > 0) {
    String respuesta = http.getString();
    Serial.println("Respuesta del Servidor:");
    Serial.println(respuesta);
  } else {
    Serial.print("Error en envío HTTP: ");
    Serial.println(http.errorToString(codigoRespuesta).c_str());
  }

  http.end();
}