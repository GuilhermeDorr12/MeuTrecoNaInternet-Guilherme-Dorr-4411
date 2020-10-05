/**
 * @file main1.cpp
 * @author Guilherme Mathias Dörr (17000058@liberato.com.br)
 * @brief Codigo fonte da aplicacao MeuTrecoNaInternet - ESP1
 * @version 1.0
 * @date 2020-10-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */


/***************************************************************
 *                      INCLUDES
 **************************************************************/
/* Biblioteca para aportar o ESP */
#include <Arduino.h>
/* Biblioteca para conectar o wifi no ESP */
#include <WiFi.h>               
/* Biblioteca para funcoes do MQTT no ESP */
#include <PubSubClient.h>       
/* Biblioteca para funcoes do sensor DHT */
#include <DHT.h>                

/***************************************************************
 *                      DEFINES
 **************************************************************/
/** @brief: Os defines abaixo referem-se, cada um, a um topico e esses, a 
* uma instância do programa, sendo elas: requisicao de dados, 
* confirmacao de recebimento de dados, sensor de temperatura, 
* sensor de umidade, sensor de luz e sensor de chuva. Por fim, 
* o ID do ESP para o servidor MQTT. */

#define TOPICO_SUBSCRIBE       "IotTro/Guilherme/Data_request"     
#define TOPICO_PUBLISH_CONF    "IotTro/Guilherme/Casa_1/Data_confirm" 
#define TOPICO_PUBLISH_DHTT    "IotTro/Guilherme/Casa_1/DHT_temp"  
#define TOPICO_PUBLISH_DHTU    "IotTro/Guilherme/Casa_1/DHT_umid"  
#define TOPICO_PUBLISH_LDR     "IotTro/Guilherme/Casa_1/LDR"       
#define TOPICO_PUBLISH_CHUVA   "IotTro/Guilherme/Casa_1/Chuva"                                                  
#define ID_MQTT  "IotTro_Guilherme_4411:esp32_1"                   

/**
 * @brief: Os defines abaixo referem-se ao pino do ESP ao qual
 * o DHT esta conectado e o tipo de DHT, no caso, o 11.
 */
#define DHTPIN 0                
#define DHTTYPE DHT11           

/**
 * @brief: Os defines abaixo referem-se aos pinos dos sensores
 * de luz e chuva.
 */
#define PinoLDR 35              
#define PinoCHUVA 32            

/***************************************************************
 *                      VARIABLES
 **************************************************************/         

/**
 * @brief: As variaveis abaixo sao referentes as informacoes da
 * rede wifi: o nome (SSID) e senha (PASSWORD).
 */
const char* SSID = "TeutoNET_Vanduir";                    
const char* PASSWORD = "Cafepele7667";                    
  
/**
 * @brief As variaveis abaixo sao referentes as informacoes do
 * servidor MQTT: link (BROKER_MQTT), porta (BROKER_PORT). E
 * variaveis auxiliares para as informacoes que serao passadas
 * pelo MQTT: mensagem e msg.
 */
const char* BROKER_MQTT = "mqtt.eclipse.org";         
int BROKER_PORT = 1883;    
String msg;

/***************************************************************
 *                      LIBRARY DEFINITIONS
 **************************************************************/

/**
 * @brief: Configuracao do cliente da biblioteca MQTT, 
 * e definicoes da biblioteca do DHT.
 */
WiFiClient espClient;                                 
PubSubClient MQTT(espClient);                         
DHT dht(DHTPIN, DHTTYPE);                             

/***************************************************************
 *                          FUNCTIONS
 **************************************************************/
/**
 * @brief: Abaixo temos as funcoes que sao utilizadas no programa,
 * cada uma delas sera especificada quando sao determinas as 
 * funcionalidades, aqui temos apenas as declaracoes.
 */

void dados_temp_umid();
void dados_ldr();
void dados_chuva();
void ConnectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

/**
* @brief: Na funcao setup realizamos a conexao com a rede wifi
* e setamos as configuracoes para conexao no broker MQTT e 
* configuracoes de tempo. Iniciamos tambem a Serial que sera utilizada
* para mostrar possíveis erros.
*/
void setup() {

    Serial.begin(115200);
    delay(10);
    /**
     * @brief: Abaixo temos a conexao wifi atraves de ConnectWiFi().
     * As informacoes sao exibidas na serial
     */
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    ConnectWiFi();                             
    /**
     * @brief: Abaixo temos a conexao no servidor mqtt, tambem
     * e estabelecido a funcao de retorno, quando se recebe algo.
     */
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   
    MQTT.setCallback(mqtt_callback);            
    /**
     * @brief: Abaixo temos a inicializacao do sensor DHT.
     */
    dht.begin();                                
}

/**
 * @brief: Abaixo temos a funcao utilizada para ler os dados 
 * adquiridos pelo sensor DHT atraves dos trechos dht.readTemperature()
 * e dht.readHumidity(). Em seguida, temos a verificacao se 
 * ocorreu a leitura correta, caso nao seja verdadeiro, o valor atribuido
 * e de 200 para ambas variaveis ("t" e "h"). Em seguida e realizada
 * a publicacao nos topicos relacionados as variaveis.
 */
void dados_temp_umid()
{
    float t = dht.readTemperature();                    
    float h = dht.readHumidity();                       
    char msg_t[10];
    char msg_h[10];
    
    if (isnan(h) || isnan(t))                           
    {                                                   
        t = 200;                                        
        h = 200;                                        
    }
    sprintf(msg_t, "%.2f", t);
    sprintf(msg_h, "%.2f", h);
    MQTT.publish(TOPICO_PUBLISH_DHTT, msg_t);       
    MQTT.publish(TOPICO_PUBLISH_DHTU, msg_h);       
}

/**
 * @brief: A funcao abaixo e utilizada para ler o sensor LDR,
 * essa leitura e feita traves de um ADC. Em seguida o valor
 * e publicado no topico. 
 */
void dados_ldr()
{
    int luz = analogRead(PinoLDR);                  
    char msg_l[10];

    sprintf(msg_l, "%i", luz);
    MQTT.publish(TOPICO_PUBLISH_LDR, msg_l);        
}

/**
 * @brief: A funcao abaixo e utilizada para ler o sensor de chuva,
 * essa leitura e feita traves de um ADC. Em seguida o valor
 * e publicado no topico. 
 */
void dados_chuva()
{
    int chuva = analogRead(PinoCHUVA);                 
    char msg_c[10];

    sprintf(msg_c, "%i", chuva);
    MQTT.publish(TOPICO_PUBLISH_CHUVA, msg_c);        
}

/**
 * @brief: a funcao abaixo e utilizada para realizar a reconexao
 * do servidor MQTT caso ocorra a desconexao e comunica na
 * serial se a resposta foi positiva ou nao. A tentativa e feita
 * a cada 10 segundos.
 */
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("\nConectando-se ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conexao realizada com sucesso!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("A conexao foi perdida.");
            Serial.println("Em 10s ocorrera nova tentativa de conexao.");
            delay(10000);
        }
    }
}

/**
 * @brief: a funcao abaixo e utilizada para realizar a conexao
 * com a rede Wifi. e passado na serial as informacoes caso
 * tenha sido bem sucedida ou nao. 
 */
void ConnectWiFi() 
{
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);                 
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("O ESP foi conectado a rede Wifi ");
    Serial.print(SSID);
}

/**
 * @brief: A funcao abaixo e responsavel por manter a
 * conexao com o WiFi e MQTT e caso ocorra desconexao,
 * tentar novamente. 
 */
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); 
     
     ConnectWiFi(); 
}

/**
 * @brief: A funcao abaixo e responsavel por receber uma
 * mensagem, essa mensagem serve para pedir os dados da aplicacao
 * e e feita a cada 15 minutos. Em seguida publica na serial
 * o que foi recebido.
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    char msg_c[20];

    sprintf(msg_c, "Pedido recebido");
    MQTT.publish(TOPICO_PUBLISH_CONF, msg_c);
 
    Serial.print("Dado recebido: ");
    Serial.println(msg);
}

/**
 * @brief: Aqui temos a chamada de cada funcao, definida
 * como MQTT data. Em seguida envia na serial uma confirmacao
 * do envio de informacoes. 
 */
void mqtt_data(void)
{
    dados_temp_umid();
    dados_ldr();
    dados_chuva();
    Serial.println("Mensagem enviada.");
    msg = ""; 
}

/**
 * @brief: Aqui temos a funcao principal da aplicacao, que fica
 * em looping. Primeiro ocorre a verificacao de conexao, em seguida,
 * caso seja recebida uma mensagem envia as informacoes e por fim
 * o keep-alive da comunicacao MQTT. 
 */
void loop() {
  
  VerificaConexoesWiFIEMQTT(); 

  if(msg == "1")
  {
      mqtt_data(); 
  }

  MQTT.loop(); 
}