#include <Arduino.h>

#include <WiFi.h>               //Biblioteca para conectar o esp ao wifi
#include <PubSubClient.h>       //Biblioteca com funções de conexão para o MQTT
#include <time.h>               //Biblioteca para modificações do tempo

//#define TOPICO_SUBSCRIBE "GuilhermeTeste"       //tópico MQTT de escuta
#define TOPICO_PUBLISH   "GuilhermeTeste"       //tópico MQTT de envio de informações para Broker                                                   
#define ID_MQTT  "Guilherme_4411:Esp32_1"       //id mqtt: nome dado ao dispositivo

//variaveis
// WIFI
const char* SSID = "Net-Virtua-8328";                 //Rede wifi de Conexão
const char* PASSWORD = "12122002";                    //Senha da rede 
  
// MQTT
const char* BROKER_MQTT = "test.mosquitto.org";       //Link de broker MQTT
int BROKER_PORT = 1883;                               //Porta do Broker MQTT
char mensagem[50];
int cont = 0;
 
//Definindo os 'clientes'
WiFiClient espClient;                                 //Definindo o cliente como esp: Wifi
PubSubClient MQTT(espClient);                         //Definindo o cliente como esp: MQTT

//hora
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = -3600*3;

//Funções
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

void setup() {
  //configuração da serial
    Serial.begin(115200);       //configurando serial para controle

    //configuração do wifi
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();             //Caso não seja sucedida a conexão, realiza novamente

    //configuração do MQTT
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //Conectando no broker
    //MQTT.setCallback(mqtt_callback);            //Função para quando se recebe algo

    //configuração para pegar hora
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            //MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
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

//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

//Função: envia ao Broker o estado atual do output 
//Parâmetros: nenhum
//Retorno: nenhum
void EnviaEstadoOutputMQTT(void)
{
    cont++;
    sprintf(mensagem,"Dado %d\n\r", cont);
    MQTT.publish(TOPICO_PUBLISH, mensagem);
 
    //printLocalTime();
    Serial.println("Fim do envio de mensagem.");
    delay(10000);
}

//void printLocalTime()
// {
//  struct tm timeinfo;
//  if(!getLocalTime(&timeinfo)){
//    Serial.println("Failed to obtain time");
//    return;
//  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
//}

void loop() {
  //garante funcionamento das conexões WiFi e ao broker MQTT
  VerificaConexoesWiFIEMQTT();

  //envia o status de todos os outputs para o Broker no protocolo esperado
  EnviaEstadoOutputMQTT();
 
  //keep-alive da comunicação com broker MQTT
  MQTT.loop();
}