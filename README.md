# Projeto PlantSync: Nó Sensor Edge para o Agronegócio

Protótipo funcional de campo para telemetria de microclima (temperatura e umidade) voltado ao monitoramento de frutas de exportação em packing houses e câmaras de resfriamento.

---

## 1. Identificação da Equipe
* **Nome do Time:** PlantSync Telemetry
* **Integrantes:**
  * Nicolas Tavares Da Silva
  * Ygor Sampio
  * Davi Clemente
  * Jorge Figueredo

---

## 2. Canal de Telemetria
* **Plataforma:** ThingSpeak
* **Link Público do Canal:** [Visualizar Canal ao Vivo](https://thingspeak.mathworks.com/channels/3493443)

---

## 3. Instruções e Código-Fonte

### Estrutura de Arquivos do Repositório
* `main.ino`: Código principal do firmware (ESP32-C3).
* `credentials.h.example`: Arquivo modelo de configuração de rede.

### Código-Fonte: `main.ino`
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include "credentials.h" // Importa as credenciais com segurança (SSID, Senha e API Key)

// Configurações do Sensor e Pinos (ESP32-C3)
#define DHTPIN 4         // Pino GPIO onde o DHT11 está conectado
#define DHTTYPE DHT11    // Definição do modelo do sensor

DHT dht(DHTPIN, DHTTYPE);

const char* server = "[http://api.thingspeak.com/update](http://api.thingspeak.com/update)";
unsigned long ultimaAtualizacao = 0;
const unsigned long intervaloUpdate = 20000; // 20 segundos (respeitando o limite mínimo de 15s)

void setup() {
  Serial.begin(115200);
  delay(1000);
  dht.begin();
  conectarWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

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
    Serial.println("\nFalha ao conectar. Tentando novamente no próximo ciclo.");
  }
}

void enviarDadosTelemetria() {
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();
  long rssi = WiFi.RSSI();

  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Erro crítico: Falha na leitura do sensor DHT11!");
    return;
  }

  Serial.printf("Temperatura: %.2f °C | Umidade: %.2f %% | RSSI: %d dBm\n", temperatura, umidade, rssi);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + THINGSPEAK_API_KEY + 
                 "&field1=" + String(temperatura) + 
                 "&field2=" + String(umidade) + 
                 "&field3=" + String(rssi);

    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      Serial.printf("Telemetria enviada com sucesso! Código HTTP: %d\n", httpCode);
    } else {
      Serial.printf("Erro na requisição HTTP: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
