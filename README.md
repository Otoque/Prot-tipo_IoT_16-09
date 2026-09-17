# 🌿 PlantSync: 

Protótipo funcional de campo para telemetria de microclima (temperatura e umidade), desenvolvido para o monitoramento de frutas de exportação (como manga e uva de mesa) em packing houses, câmaras de resfriamento e carretas refrigeradas na região do Vale do São Francisco (Petrolina e Juazeiro).

---

## 👥 1. Identificação da Equipe
* **Nome do Time:** PlantSync
* **Integrantes:**
  * Nicolas Tavares Da Silva
  * Ygor Sampio
  * Davi Clemente
  * Jorge Figueredo

---

## 📡 2. Canal de Telemetria
* **Plataforma:** ThingSpeak
* **Link Público do Canal:** [Visualizar Canal ao Vivo](https://thingspeak.mathworks.com/channels/3493443) *(Gráficos de temperatura, umidade e RSSI em tempo real)*

---

## 📸 3. Registro Fotográfico do Artefato
* **Montagem Eletrônica Interna:** *(Insira aqui ou adicione no repositório a foto do circuito interno evidenciando as ligações e pinagem do ESP32-C3 com o sensor DHT11).*
* **Protótipo Final (Upcycling Enclosure):** *(Insira aqui ou adicione no repositório a foto do gabinete fechado, destacando as aberturas de aeração convectiva e o acesso ao cabo USB-C).*

---

## 💻 4. Código-Fonte Documentado e Instruções

### Estrutura de Arquivos do Repositório
```text
PlantSync/
│
├── main.ino
├── credentials.h.example
└── README.md
```

---

## Instruções de Configuração e Upload

---

## 💻 Ambiente

Utilize a Arduino IDE com o suporte à placa ESP32 instalado.

Placa: ESP32 by Espressif Systems

---

## 📚 Bibliotecas Necessárias

Instale a seguinte biblioteca:

DHT sensor library — Adafruit

As bibliotecas abaixo já estão disponíveis no core do ESP32:

WiFi

HTTPClient

---

## 🔐 Configuração de Segredos

Crie um arquivo chamado credentials.h na mesma pasta do projeto.

Utilize o seguinte modelo para preencher suas credenciais:

#define WIFI_SSID "NOME_DA_REDE"
#define WIFI_PASSWORD "SENHA_DA_REDE"
#define THINGSPEAK_API_KEY "SUA_API_KEY"

---

## ⚠️ O arquivo credentials.h está incluído no .gitignore para evitar o vazamento das credenciais e da API Key.
```cpp
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

const char* WIFI_SSID = "SEU_WIFI_SSID_AQUI";
const char* WIFI_PASSWORD = "SUA_SENHA_WIFI_AQUI";
const char* THINGSPEAK_API_KEY = "SUA_THINGSPEAK_API_KEY_AQUI";

#endif
```

---

## Código-Fonte: main.ino
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include "credentials.h" // Importação segura de credenciais (SSID, Senha e API Key)

// Configurações do Sensor e Pinos (ESP32-C3)
#define DHTPIN 4         // Pino GPIO conectado ao pino de dados do DHT11
#define DHTTYPE DHT11    // Definição do modelo do sensor

DHT dht(DHTPIN, DHTTYPE);

const char* server = "[http://api.thingspeak.com/update](http://api.thingspeak.com/update)";
unsigned long ultimaAtualizacao = 0;
const unsigned long intervaloUpdate = 20000; // 20 segundos (respeita o rate limit do ThingSpeak de mín. 15s)

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  dht.begin();
  conectarWiFi();
}

void loop() {
  // Verificação e reconexão automática de rede
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Executa a telemetria respeitando o intervalo estabelecido
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
  long rssi = WiFi.RSSI(); // Indicador de integridade do enlace (Field 3)

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
```

## 📋 5. Memorial Descritivo e Justificativa de Engenharia
### Aplicação no Negócio

O sistema foi concebido para atuar diretamente no polo agrícola do Vale do São Francisco (Petrolina-PE / Juazeiro-BA). O nó sensor pode ser alocado em packing houses, câmaras de resfriamento rápido pós-colheita ou contêineres de transporte refrigerado. O controle rigoroso do microclima protege frutas de alto valor de exportação contra estresse térmico, condensação e degradação fitossanitária.
O que o Protótipo Faz

O ESP32-C3 executa um ciclo autônomo e contínuo:

    Coleta dados brutos digitais do sensor DHT11 (Field 1: Temperatura em °C e Field 2: Umidade relativa em %).

    Afere o nível de sinal da rede Wi-Fi (Field 3: RSSI em dBm como indicador de integridade de enlace).

    Realiza o tratamento de falhas e reconexão automática de rede.

    Empacota os dados e dispara uma requisição HTTP GET para a API do ThingSpeak a cada 20 segundos, alimentando painéis de séries temporais em tempo real.

    Utiliza um gabinete estruturado por princípios de Upcycling (embalagem reaproveitada) com aberturas estratégicas para aeração convectiva e passagem do cabo de alimentação USB-C.

## Limitações Técnicas Identificadas

    Sensor DHT11: Possui faixa operacional limitada (0 a 50°C), baixa precisão na umidade (±5%) e tempo de resposta lento. Sob condições de umidade extrema ou condensação severa comuns em câmaras frias, o sensor tende a saturar. Recomenda-se migração futura para sensores industriais calibrados (ex: série SHT3x).

    Conectividade Wi-Fi: Ambientes industriais agrícolas possuem barreiras físicas densas e interferências eletromagnéticas que atenuam drasticamente o sinal. Além disso, o uso contínuo de Wi-Fi e HTTP consome muita energia, tornando o nó dependente de alimentação USB cabeada (inviabilizando o uso prolongado em baterias em trânsito sem protocolos de baixo consumo como LoRaWAN, NB-IoT ou ciclos de Deep Sleep).

    Carcaça Improvisada (Upcycling): Embora atenda perfeitamente aos requisitos de Prova de Conceito (PoC), o invólucro não possui certificação de grau de proteção IP formal, ficando vulnerável à entrada de poeira fina de galpões ou jatos d'água de limpeza pesada.

## 🚀 6. Próximos Passos (Integração com Cloud do PI)

Na evolução da arquitetura do projeto, os dados coletados e armazenados provisoriamente no ThingSpeak servirão como ingestão de camada primária. O fluxo passará a ser processado por um backend integrador (ex: API em Python Flask ou Node.js), que direcionará os dados de telemetria para um banco de dados relacional em nuvem (Supabase / PostgreSQL ou AWS/Azure). A partir dessa persistência centralizada, serão aplicados modelos analíticos e preditivos para monitorar a qualidade da safra e antecipar perdas na cadeia logística de exportação.

## 🌱 PlantSync Telemetry — Monitoramento inteligente para o agronegócio.
