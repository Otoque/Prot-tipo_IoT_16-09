#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include "credentials.h" // Importa as credenciais com segurança (SSID, Senha e API Key)

// Configurações do Sensor e Pinos (ESP32-C3)
#define DHTPIN 4         // Pino GPIO onde o DHT11 está conectado (ajuste se usar outro pino)
#define DHTTYPE DHT11    // Definição do modelo do sensor

// Instanciação do sensor
DHT dht(DHTPIN, DHTTYPE);

// Configurações do ThingSpeak
const char* server = "http://api.thingspeak.com/update";
unsigned long ultimaAtualizacao = 0;
const unsigned long intervaloUpdate = 20000; // 20 segundos (respeitando o limite mínimo de 15s do ThingSpeak)

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  dht.begin();
  
  // Inicializa a conexão Wi-Fi
  conectarWiFi();
}

void loop() {
  // Verifica se o Wi-Fi caiu e tenta reconectar automaticamente (Tratamento de falhas)
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Verifica se já passou o tempo necessário para o próximo envio (Timer não-bloqueante com millis)
  if (millis() - ultimaAtualizacao >= intervaloUpdate) {
    enviarDadosTelemetria();
    ultimaAtualizacao = millis();
  }
}

void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(1000);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Conectado com sucesso!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao conectar. O sistema tentará novamente no próximo ciclo.");
  }
}

void enviarDadosTelemetria() {
  // Leitura das grandezas do microclima
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();
  long rssi = WiFi.RSSI(); // Indicador de integridade do sinal Wi-Fi (Field 3 opcional)

  // Validação dos dados lidos para evitar envio de valores nulos (NaN)
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Erro crítico: Falha na leitura do sensor DHT11!");
    return;
  }

  // Exibe os dados no Monitor Serial para depuração local
  Serial.printf("Temperatura: %.2f °C | Umidade: %.2f %% | RSSI: %d dBm\n", temperatura, umidade, rssi);

  // Montagem e disparo da requisição HTTP REST para o ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + THINGSPEAK_API_KEY + 
                 "&field1=" + String(temperatura) + 
                 "&field2=" + String(umidade) + 
                 "&field3=" + String(rssi);

    http.begin(url);
    int httpCode = http.GET(); // Executa a requisição GET
    
    if (httpCode > 0) {
      Serial.printf("Telemetria enviada com sucesso! Código HTTP: %d\n", httpCode);
    } else {
      Serial.printf("Erro na requisição HTTP: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end(); // Fecha a conexão e libera os recursos de rede
  }
}
