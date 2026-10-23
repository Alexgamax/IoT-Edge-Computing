#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <sys/time.h>

const char* WIFI_SSID = "Totalplay-2.4G-4a08";
const char* WIFI_PASSWORD = "nVm2476ndThz4EfM";
const char* MQTT_HOST = "192.168.100.17"; // IP LAN Mosquitto
const int MQTT_PORT = 1883;

const char* TOPIC_TELEMETRIA = "invernadero/telemetria";
const char* TOPIC_CONTROL = "invernadero/control";

const int PIN_HUMEDAD_SUELO = 34; // Potenciómetro / HW-390
const int PIN_LUZ = 35;           // LDR Casera
const int PIN_LM35 = 32;          // LM35 Temperatura
const int PIN_ACTUADOR = 26;      // Salida lógica para actuación (LED / Relé)

unsigned long SEND_INTERVAL_MS = 500; // Intervalo de envío (ensayar 100, 500 y 1000 ms)

WiFiClient net;
PubSubClient mqtt(net);
unsigned long lastSend = 0;
uint32_t sequenceId = 0;

long epochMillis() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (tv.tv_sec < 1700000000) return 0; // Reloj aún no sincronizado por NTP
  return (long)((long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000);
}

void callback(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  
  if (String(topic) == TOPIC_CONTROL) {
    Serial.printf("Comando de control recibido: %s\n", msg.c_str());
    if (msg.indexOf("\"value\":true") >= 0 || msg.indexOf("ON") >= 0) {
      digitalWrite(PIN_ACTUADOR, HIGH);
      Serial.println("-> Actuador ENCENDIDO");
    } 
    else if (msg.indexOf("\"value\":false") >= 0 || msg.indexOf("OFF") >= 0) {
      digitalWrite(PIN_ACTUADOR, LOW);
      Serial.println("-> Actuador APAGADO");
    }
  }
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Conectado!");
    Serial.print("IP ESP32: "); 
    Serial.println(WiFi.localIP());
    configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  }
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando a Broker MQTT...");
    if (mqtt.connect("ESP32_INVERNADERO_01")) {
      Serial.println(" Conectado!");
      mqtt.subscribe(TOPIC_CONTROL); 
    } else {
      Serial.printf(" Falló rc=%d; reintento en 2s\n", mqtt.state());
      delay(2000);
      if (WiFi.status() != WL_CONNECTED) connectWiFi();
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_ACTUADOR, OUTPUT);
  digitalWrite(PIN_ACTUADOR, LOW);
  
  analogReadResolution(12);
  connectWiFi();
  
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(callback);
  mqtt.setBufferSize(512);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL_MS) {
    lastSend = now;

    int rawSoil = analogRead(PIN_HUMEDAD_SUELO);
    int rawLight = analogRead(PIN_LUZ);
    int rawTemp = analogRead(PIN_LM35);

    float soilPct = constrain(100.0f * (4095.0f - rawSoil) / 4095.0f, 0.0f, 100.0f);
    float lightPct = constrain((rawLight / 4095.0f) * 100.0f, 0.0f, 100.0f);
    float tempC = ((rawTemp / 4095.0f) * 3300.0f) / 10.0f; // LM35: 10mV/°C

    long sentEpoch = epochMillis();

    char payload[384];
    snprintf(payload, sizeof(payload),
      "{\"node_id\":\"ESP32_INVERNADERO_01\",\"sequence_id\":%lu,"
      "\"timestamp_ms\":%lu,\"sent_at_epoch_ms\":%ld,"
      "\"data\":{\"humedad_suelo_pct\":%.2f,\"luz_pct\":%.2f,\"temperatura_c\":%.2f}}",
      (unsigned long)sequenceId++, now, sentEpoch, soilPct, lightPct, tempC);

    bool ok = mqtt.publish(TOPIC_TELEMETRIA, payload);
    Serial.printf("Publish [%s] ID:%lu -> %s\n", ok ? "OK" : "ERROR", sequenceId - 1, payload);
  }
}