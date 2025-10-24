#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "52.15.102.67";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define LDR_PIN 34
#define TMP36_PIN A0
#define HUMIDITY_PIN A1
#define LED_VERDE 15
#define LED_AMARELO 16
#define LED_VERMELHO 17
#define BUZZER_PIN 18

#define DHTPIN 13
#define DHTTYPE DHT22

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

const int NUMERO_LEITURAS = 5;
float leituras_temp[NUMERO_LEITURAS];
float leituras_umid[NUMERO_LEITURAS];
int indice_leitura = 0;
unsigned long tempo_ultima_leitura_lcd = 0;

void setup() {
Serial.begin(9600);
pinMode(LED_VERDE, OUTPUT);
pinMode(LED_AMARELO, OUTPUT);
pinMode(LED_VERMELHO, OUTPUT);
pinMode(BUZZER_PIN, OUTPUT);
dht.begin();
lcd.init();
lcd.backlight();
lcd.clear();
lcd.print("Iniciando...");
delay(1000);
// Conexão ao Wi-Fi Wokwi
Serial.println();
Serial.print("Conectando a ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("\nWiFi conectado!");
// Conexão ao Broker MQTT
client.setServer(mqtt_server, mqtt_port);
if (!client.connected()) {
Serial.println("Conectando ao broker MQTT...");
if (client.connect("ESP32Client")) {
Serial.println("Conectado ao broker MQTT.");
} else {
Serial.print("Falha na conexao MQTT. Codigo: ");
Serial.println(client.state());
}
}
}
void loop() {
if (!client.connected()) {
Serial.println("Tentando reconectar ao broker MQTT...");
if (client.connect("ESP32Client")) {
Serial.println("Reconectado!");
} else {
delay(5000);
return;
}
}
client.loop();
int valor_ldr = analogRead(LDR_PIN);
float temperatura = dht.readTemperature();
float umidade = dht.readHumidity();
if (isnan(temperatura) || isnan(umidade)) {
Serial.println("Falha na leitura do sensor DHT!");
lcd.clear();
lcd.print("Erro no sensor!");
delay(1000);
return;
}
leituras_temp[indice_leitura] = temperatura;
leituras_umid[indice_leitura] = umidade;
indice_leitura++;
if (client.connected()) {
char tempStr[10];
dtostrf(temperatura, 4, 1, tempStr);
client.publish("sensores/temperatura", tempStr);

char umidStr[10];
dtostrf(umidade, 4, 1, umidStr);
client.publish("sensores/umidade", umidStr);

char luzStr[10];
dtostrf(valor_ldr, 4, 0, luzStr);
client.publish("sensores/luz", luzStr);
}
if (indice_leitura >= NUMERO_LEITURAS) {
float soma_temp = 0;
float soma_umid = 0;
for (int i = 0; i < NUMERO_LEITURAS; i++) {
soma_temp += leituras_temp[i];
soma_umid += leituras_umid[i];
}
float media_temp = soma_temp / NUMERO_LEITURAS;
float media_umid = soma_umid / NUMERO_LEITURAS;
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Temp: ");
lcd.print(media_temp, 1);
lcd.print("C ");
// Exibe status da temperatura
if (media_temp < 10.0) lcd.print("Baixa");
else if (media_temp > 15.0) lcd.print("Alta");
else lcd.print("OK");
lcd.setCursor(0, 1);
lcd.print("Umid: ");
lcd.print(media_umid, 1);
lcd.print("% ");
// Exibe status da umidade
if (media_umid < 50.0) lcd.print("Baixa");
else if (media_umid > 70.0) lcd.print("Alta");
else lcd.print("OK");
// Reinicia o contador para as próximas 5 leituras
indice_leitura = 0;
delay(5000);
}
bool buzzer_ativo = false;
// 1. Lógica do Fotorresistor (LDR)
if (valor_ldr < 1000) { // Ambiente escuro
digitalWrite(LED_VERDE, HIGH);
digitalWrite(LED_AMARELO, LOW);
digitalWrite(LED_VERMELHO, LOW);
lcd.setCursor(0, 0);
lcd.print("Iluminacao OK  ");
} else if (valor_ldr < 2000) { // Meia luz
digitalWrite(LED_VERDE, LOW);
digitalWrite(LED_AMARELO, HIGH);
digitalWrite(LED_VERMELHO, LOW);
lcd.setCursor(0, 0);
lcd.print("Meia luz      ");
} else { // Ambiente muito claro
digitalWrite(LED_VERDE, LOW);
digitalWrite(LED_AMARELO, LOW);
digitalWrite(LED_VERMELHO, HIGH);
buzzer_ativo = true;
lcd.setCursor(0, 0);
lcd.print("Ambiente claro");
}
if (temperatura < 10.0 || temperatura > 15.0) {
digitalWrite(LED_AMARELO, HIGH);
buzzer_ativo = true;
}
if (umidade < 50.0 || umidade > 70.0) {
digitalWrite(LED_VERMELHO, HIGH);
buzzer_ativo = true;
}
if (buzzer_ativo) {
tone(BUZZER_PIN, 1000); 
} else {
noTone(BUZZER_PIN); 
}
delay(1000); 
}
