#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"

// Mapeamento dos pinos para o ESP32
#define sensor_ldr       34  
#define led_verde        15
#define led_amarelo      16
#define led_vermelho     17
#define buzzer           18

// DHT22 - Pino de dados e tipo de sensor
#define DHTPIN           13 
#define DHTTYPE          DHT22

// O pino SDA do LCD I2C é GPIO21 e o SCL é GPIO22 no ESP32
LiquidCrystal_I2C lcd(0x27, 16, 2); 

DHT dht(DHTPIN, DHTTYPE);

// 🥂 Caracteres personalizados
byte wineGlass[8] = {
  B00000,
  B01110,
  B01110,
  B01110,
  B00100,
  B00100,
  B01110,
  B00000
};

byte wineBottle[8] = {
  B00100,
  B00100,
  B01110,
  B01110,
  B01110,
  B01110,
  B01110,
  B00000
};

unsigned long tempo_buzina_alerta = 0;
unsigned long tempo_buzina_problema = 0;
unsigned long tempo_lcd = 0;
unsigned long tempo_tela = 0;

bool mostrarInfos = true;

void setup() {
  Serial.begin(9600);
  pinMode(led_verde, OUTPUT);
  pinMode(led_amarelo, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);

  dht.begin();
  
  // Inicialização do LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Cria os ícones personalizados
  lcd.createChar(0, wineGlass);
  lcd.createChar(1, wineBottle);
}

void loop() {
  unsigned long agora = millis();

  // Lê os sensores e armazena os valores
  int leitura_sensor_ldr = analogRead(sensor_ldr);
  int variacao_luz = map(leitura_sensor_ldr, 0, 4095, 0, 100);

  float temperatura_celsius = dht.readTemperature();
  float umidade_percentual = dht.readHumidity();

  // Alterna a mensagem a cada 3 segundos
  if (agora - tempo_tela >= 3000) {
    tempo_tela = agora;
    mostrarInfos = !mostrarInfos;
  }
  
  if (agora - tempo_lcd >= 1000) {
    tempo_lcd = agora;

    if (isnan(temperatura_celsius) || isnan(umidade_percentual)) {
      Serial.println("Falha na leitura do sensor DHT22!");
    } else {
      Serial.print("Luz: ");
      Serial.print(variacao_luz);
      Serial.print("% | Temp: ");
      Serial.print(temperatura_celsius);
      Serial.print("C | Umidade: ");
      Serial.print(umidade_percentual);
      Serial.println("%");
    }

    lcd.clear();
    if (mostrarInfos) {
      // Exibe todas as informações
      lcd.setCursor(0, 0);
      lcd.print("Luz:");
      lcd.print(variacao_luz);
      lcd.print("% T:");
      lcd.print((int)temperatura_celsius);
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Umid:");
      lcd.print(umidade_percentual);
      lcd.print("% ");
      lcd.write(byte(0)); // taça
      lcd.write(byte(1)); // garrafa
    } else {
      static unsigned long tempo_mensagem = 0;
      static bool mostrar_umidade = false;

      if (millis() - tempo_mensagem >= 2000) {
        mostrar_umidade = !mostrar_umidade;
        tempo_mensagem = millis();
      }

      if (!mostrar_umidade) {
        // Primeiros 2s: iluminação e temperatura
        lcd.setCursor(0, 0);
        if (variacao_luz <= 30) {
          lcd.print("Amb escuro");
        } else if (variacao_luz <= 70) {
          lcd.print("Meia luz");
        } else {
          lcd.print("Amb claro");
        }

        lcd.setCursor(0, 1);
        if (temperatura_celsius >= 10 && temperatura_celsius <= 15) {
          lcd.print("Temp OK ");
        } else if (temperatura_celsius > 15) {
          lcd.print("Temp Alta ");
        } else {
          lcd.print("Temp Baixa ");
        }
      } else {
        // Depois dos 2s: só umidade
        lcd.setCursor(0, 0);
        lcd.print("Umidade:");

        lcd.setCursor(0, 1);
        if (umidade_percentual >= 50 && umidade_percentual <= 70) {
          lcd.print("Umid OK ");
        } else if (umidade_percentual > 70) {
          lcd.print("Umid Alta ");
        } else {
          lcd.print("Umid Baixa ");
        }
      }
    }
  }
    
  // Lógica para acender os LEDs e ativar o buzzer
  if (variacao_luz <= 70 && temperatura_celsius >= 10 && temperatura_celsius <= 15 && umidade_percentual >= 50 && umidade_percentual <= 70) {
    status_ok();
  } else if ((variacao_luz > 70 && variacao_luz <= 85) || temperatura_celsius > 15 || temperatura_celsius < 10 || umidade_percentual > 70 || umidade_percentual < 50) {
    status_alerta();
  } else {
    status_problema();
  }
  
  atualiza_buzinas();
  delay(100);
}

void status_ok() {
  digitalWrite(led_verde, HIGH);
  digitalWrite(buzzer, LOW);
  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_vermelho, LOW);
}

void status_alerta() {
  digitalWrite(led_amarelo, HIGH);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
}

void status_problema() {
  digitalWrite(led_vermelho, HIGH);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_amarelo, LOW);
}

void atualiza_buzinas() {
  unsigned long agora = millis();

  if (digitalRead(led_amarelo) == HIGH) {
    if (agora - tempo_buzina_alerta >= 500 && agora - tempo_buzina_alerta < 3000) {
      digitalWrite(buzzer, HIGH);
    }
    if (agora - tempo_buzina_alerta >= 3000) {
      digitalWrite(buzzer, LOW);
      tempo_buzina_alerta = agora;
    }
  }

  if (digitalRead(led_vermelho) == HIGH) {
    if (agora - tempo_buzina_problema >= 500 && agora - tempo_buzina_problema < 3000) {
      digitalWrite(buzzer, HIGH);
    }
    if (agora - tempo_buzina_problema >= 3000) {
      digitalWrite(buzzer, LOW);
      tempo_buzina_problema = agora;
    }
  }
}
