#include <Arduino.h>

#include <WiFi.h>               //Biblioteca para conectar o esp ao wifi
#include <PubSubClient.h>       //Biblioteca com funções de conexão para o MQTT
#include <time.h>               //Biblioteca para modificações do tempo
#include <DHT.h>                //Biblioteca do sensor de temperatura e umidade

#define TOPICO_SUBSCRIBE       "IotTro/Guilherme/Data_request"        //Tópico de request dos dados
#define TOPICO_PUBLISH_CONF    "IotTro/Guilherme/Casa_2/Data_confirm" //Tópico de confirmação de requisição de dados
#define TOPICO_PUBLISH_DHTT    "IotTro/Guilherme/Casa_2/DHT_temp"     //Tópico de envio dos dados de temperatura
#define TOPICO_PUBLISH_DHTU    "IotTro/Guilherme/Casa_2/DHT_umid"     //Tópico de envio dos dados de umidade
#define TOPICO_PUBLISH_LDR     "IotTro/Guilherme/Casa_2/LDR"          //Tópico de envio dos dados de luminosidade
#define TOPICO_PUBLISH_CHUVA   "IotTro/Guilherme/Casa_2/Chuva"        //Tópico de envio dos dados de chuva                                              
#define ID_MQTT  "IotTro_Guilherme_4411:esp32_2"                      //id mqtt: nome dado ao dispositivo

#define DHTPIN 0                //Definindo o pino de dados conectado ao sensor DHT
#define DHTTYPE DHT11           //Definindo o tipo de sensor DHT (no meu caso, 11)

#define PinoLDR 34            //Definição para ESP32 LoRa
#define PinoCHUVA 35          //Definição para ESP32 LoRa    

//variaveis
// WIFI
const char* SSID = "Net-Virtua-8328";                 //Rede wifi de Conexão
const char* PASSWORD = "12122002";                    //Senha da rede 
  
// MQTT
const char* BROKER_MQTT = "mqtt.eclipse.org";         //Link de broker MQTT
int BROKER_PORT = 1883;                               //Porta do Broker MQTT
char mensagem[50];
String msg = "";
int cont = 0;
 
//hora
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = -3600*3;
struct tm timeinfo;

//Definindo os 'clientes'
WiFiClient espClient;                                 //Definindo o cliente como esp: Wifi
PubSubClient MQTT(espClient);                         //Definindo o cliente como esp: MQTT
DHT dht(DHTPIN, DHTTYPE);                             //Definindo o cliente como DHT11 e pino 04

//Funções
void dados_temp_umid();
void dados_ldr();
void dados_chuva();
void ConnectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

/*
* Na função setup realizamos a conexão com a rede wifi
* e setamos as configurações para conexão no broker MQTT,
* também pegamos da rede wifi a data e hora, através de 
* uma conexão com o site pool.ntp.org. Esse site permite
* pegar a data e hora conforme o Meridiano, por isso devemos
* realizar uma configuração para a localização Brasil.
*/
void setup() {

    Serial.begin(115200);                       //vamos utilizar a serial para verificar se as
                                                //conexões estão sendo estabelecidas de maneira correta
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    ConnectWiFi();                             //Função de conexão/reconexão no wifi

    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //Conectando no broker
    MQTT.setCallback(mqtt_callback);            //Função para quando se recebe algo

    dht.begin();                                //Habilitando o sensor de temperatura e umidade

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);       //Configurando o tempo pra Brasil (GMT-3:00)
    if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
}

/*
* Função utilizada para realizar a leitura do sensor DHT11
* de umidade e temperatura, retornando essas informações.
*/
void dados_temp_umid()
{
    float t = dht.readTemperature();                    //Função para medida de temperatura
    float h = dht.readHumidity();                       //Função para medida de umidade
    char msg_t[10];
    char msg_h[10];
    
    if (isnan(h) || isnan(t))                           //Verifica se os dados foram lidos
    {                                                   //Caso não tenham sido lidos, envia "200"
        t = 200;                                        //para o broker que irá compreender que não
        h = 200;                                        //ocorreu a leitura correta.
    }
    sprintf(msg_t, "%.2f", t);
    sprintf(msg_h, "%.2f", h);
    MQTT.publish(TOPICO_PUBLISH_DHTT, msg_t);       //Envia para o tópico o dado temperatura
    MQTT.publish(TOPICO_PUBLISH_DHTU, msg_h);       //Envia para o tópico o dado umidade
}

/*
* Função utilizada para leitura do sensor LDR, através de conversão ADC
*/
void dados_ldr()
{
    int luz = analogRead(PinoLDR);                  //Leitura do sensor LDR
    char msg_l[10];

    sprintf(msg_l, "%i", luz);
    MQTT.publish(TOPICO_PUBLISH_LDR, msg_l);        //Envia para o tópico o dado luz
}

/*
* Função utilizada para leitura do sensor de chuva, através de conversão ADC
*/
void dados_chuva()
{
    int chuva = analogRead(PinoCHUVA);                  //Leitura do sensor LDR
    char msg_c[10];

    sprintf(msg_c, "%i", chuva);
    MQTT.publish(TOPICO_PUBLISH_CHUVA, msg_c);        //Envia para o tópico o dado luz
}

/*
* Essa função reconecta o cliente(esp) no broker MQTT
* e avisa no terminal serial caso a conexão foi bem sucedida.
*/
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 10s");
            delay(10000);
        }
    }
}

/*
* Função utilizada para conexão e reconexão na rede WiFi.
* Caso o cliente já esteja conectado, ocorre o retorno da função,
* caso não exista nenhuma conexão, é feita uma nova tentativa.
*/
void ConnectWiFi() 
{
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);                 // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

/*
* Verifica se há conexão WiFi e MQTT
*/
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     ConnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

/* 
* Função para recebimento de mensagem através do tópico 
* "IotTro/Guilherme/Data_request", que serve para requerir
* os dados dos sensores, ou seja, serve para que o dispositivo
* envie os dados após a requisição.
*/
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    msg = "";
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    char msg_c[20];   

    sprintf(msg_c, "Pedido recebido");
    MQTT.publish(TOPICO_PUBLISH_CONF, msg_c);
 
    Serial.print("Chegou a seguinte string via MQTT: ");
    Serial.println(msg);
}

/*
* Função responsável pela publicação das informações dos sensores
* nos tópicos conforme a seguir:
* "IotTro/Guilherme/Casa_1/DHT_temp"
* "IotTro/Guilherme/Casa_1/DHT_umid"
* "IotTro/Guilherme/Casa_1/LDR"
* "IotTro/Guilherme/Casa_1/Chuva"
*
*/
void mqtt_data(void)
{
    cont++;
    dados_temp_umid();
    dados_ldr();
    dados_chuva();
    //printLocalTime();
    Serial.println("Fim do envio de mensagem.");
    msg = ""; 
}

void loop() {
  
  VerificaConexoesWiFIEMQTT(); //garante funcionamento das conexões WiFi e ao broker MQTT

  if(msg == "1")
  {
      mqtt_data(); //envia o status de todos os outputs para o Broker no protocolo esperado
  }

  MQTT.loop(); //keep-alive da comunicação com broker MQTT
}