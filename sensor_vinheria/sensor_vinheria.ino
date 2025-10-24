#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"
#include <WiFi.h>
#include <PubSubClient.h>

// --- Configurações do Broker MQTT ---
const char* mqtt_server = "3.142.187.234"; // IP da VM (AWS)
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

// Tópico para receber comandos do MyMQTT
#define TOPICO_COMANDO "sensores/comando"

// --- Configurações do Wi-Fi ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- Mapeamento dos Pinos ---
#define LDR_PIN 34
#define TMP36_PIN A0      // Não utilizado, estamos usando o DHT
#define HUMIDITY_PIN A1   // Não utilizado, estamos usando o DHT
#define LED_VERDE 15      // Controlado pelo LDR
#define LED_AMARELO 16    // Controlado por LDR/Temp
#define LED_VERMELHO 17   // Controlado por LDR/Umid
#define LED_CONFERE 5     // Controlado pelo MyMQTT
#define BUZZER_PIN 18     // Controlado por Temp/Umid/LDR
#define DHTPIN 13         // Pino de dados do DHT22
#define DHTTYPE DHT22

// --- Objetos ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// --- Variáveis Globais ---
// Para a média do LCD
const int NUMERO_LEITURAS = 5;
float leituras_temp[NUMERO_LEITURAS];
float leituras_umid[NUMERO_LEITURAS];
int indice_leitura = 0;

// Para o temporizador (substitui o delay() no loop)
unsigned long tempo_ultima_leitura = 0;
const long INTERVALO_LEITURA = 1000; // 1 segundo

// Função: Chamada quando uma mensagem MQTT é recebida
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensagem);

  // Verifica o tópico e a mensagem para controlar o LED_CONFERE
  if (String(topic) == TOPICO_COMANDO) {
    if (mensagem == "LED_CONFERE_ON") {
      Serial.println("Comando: Ligando LED Confere.");
      digitalWrite(LED_CONFERE, HIGH);
    } else if (mensagem == "LED_CONFERE_OFF") {
      Serial.println("Comando: Desligando LED Confere.");
      digitalWrite(LED_CONFERE, LOW);
    }
  }
}

// Função: Reconecta ao Broker MQTT (e se inscreve no tópico)
void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando conectar ao Broker MQTT...");
    // Tenta conectar com um ID de cliente
    if (client.connect("ESP32Client_Wokwi")) {
      Serial.println("Conectado ao Broker!");
      
      // Se inscreve no tópico de comando
      client.subscribe(TOPICO_COMANDO);
      Serial.print("Inscrito no topico: ");
      Serial.println(TOPICO_COMANDO);
      
    } else {
      Serial.print("Falha na conexao, rc=");
      Serial.print(client.state());
      Serial.println(". Nova tentativa em 5s.");
      delay(5000); // Pausa antes de tentar de novo
    }
  }
}

// =======================
// --- SETUP ---
// =======================
void setup() {
  Serial.begin(115200); // Inicia comunicação serial
  
  // Define os pinos de saída
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_CONFERE, OUTPUT); 
  
  dht.begin(); // Inicia o sensor DHT
  
  // Inicia o LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Iniciando...");
  delay(1000);
  
  // Inicia conexão Wi-Fi
  Serial.println();
  Serial.print("Conectando a rede ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
  
  // Configura o servidor e a função de callback do MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 
}

// =======================
// --- LOOP ---
// =======================
void loop() {
  // 1. Garante que o MQTT está conectado
  if (!client.connected()) {
    reconnect(); 
  }
  client.loop(); // Processa mensagens MQTT e mantém a conexão

  // 2. Temporizador (substitui o delay)
  // O código abaixo só roda a cada 'INTERVALO_LEITURA' (1s)
  unsigned long agora = millis();
  if (agora - tempo_ultima_leitura < INTERVALO_LEITURA) {
    return; // Ainda não deu o tempo, sai do loop
  }
  tempo_ultima_leitura = agora;

  // --- 3. Leitura dos Sensores ---
  int valor_ldr = analogRead(LDR_PIN);
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Falha na leitura do DHT!");
    lcd.clear();
    lcd.print("Erro no DHT!");
    delay(1000); 
    return;
  }
  
  // --- 4. Envio de Dados via MQTT (Publicação) ---
  if (client.connected()) {
    char buffer[10]; // Buffer para converter números em texto
    
    dtostrf(temperatura, 4, 1, buffer);
    client.publish("sensores/temperatura", buffer);

    dtostrf(umidade, 4, 1, buffer);
    client.publish("sensores/umidade", buffer);

    dtostrf(valor_ldr, 4, 0, buffer);
    client.publish("sensores/luz", buffer);
  }
  
  // --- 5. Lógica do Display LCD (Média de 5 leituras) ---
  leituras_temp[indice_leitura] = temperatura;
  leituras_umid[indice_leitura] = umidade;
  indice_leitura++;
  
  if (indice_leitura >= NUMERO_LEITURAS) {
    float soma_temp = 0, soma_umid = 0;
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
    
    if (media_temp < 10.0) lcd.print("Baixa");
    else if (media_temp > 15.0) lcd.print("Alta");
    else lcd.print("OK");
    
    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.print(media_umid, 1);
    lcd.print("% ");
    
    if (media_umid < 50.0) lcd.print("Baixa");
    else if (media_umid > 70.0) lcd.print("Alta");
    else lcd.print("OK");
    
    indice_leitura = 0; // Reinicia o contador da média
  }
  
  // --- 6. Lógica de Controle Automático (LEDs e Buzzer) ---
  // Esta lógica NÃO afeta o LED_CONFERE
  bool buzzer_ativo = false;
  
  // Lógica do LDR
  if (valor_ldr < 1000) { // Escuro
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, LOW);
  
  } else if (valor_ldr < 2000) { // Meia-luz
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARELO, HIGH);
    digitalWrite(LED_VERMELHO, LOW);

  } else { // Claro
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    buzzer_ativo = true;
  }
  
  // Lógica de Temperatura
  if (temperatura < 10.0 || temperatura > 15.0) {
    digitalWrite(LED_AMARELO, HIGH);
    buzzer_ativo = true;
  }
  
  // Lógica de Umidade
  if (umidade < 50.0 || umidade > 70.0) {
    digitalWrite(LED_VERMELHO, HIGH);
    buzzer_ativo = true;
  }
  
  // Controle final do Buzzer
  if (buzzer_ativo) {
    tone(BUZZER_PIN, 1000);
  } else {
    noTone(BUZZER_PIN);
  }
}
