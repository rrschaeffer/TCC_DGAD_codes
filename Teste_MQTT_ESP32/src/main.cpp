#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DGAD_NAME "DGAD_1"

#define PUBLISH_DELAY       1000
#define PUBLISH_TOPIC_1     "Boot_Button_Status"
#define PUBLISH_TOPIC_2     "Digital_Input1_Status"
#define PUBLISH_TOPIC_3     "Analog0-10V_Value"
#define PUBLISH_TOPIC_4     "Analog0-10V_Conversion"
#define ID_MQTT             "DGAD_ESP32_MQTT" // ID MQTT

//DGAD pins define
#define LEDPIN1 4
#define LEDPIN2 5
#define LEDPIN3 2

#define BOOTBUTTONPIN  0

#define ANALOG_V_PIN1 32
#define ANALOG_V_PIN2 33
#define ANALOG_V_PIN3 34
#define ANALOG_A_PIN1 35
#define ANALOG_A_PIN2 36
#define ANALOG_A_PIN3 39

#define GPIO0  12
#define GPIO1  13
#define GPIO2  14
#define GPIO3  15
#define GPIO4  16
#define GPIO5  17
#define GPIO6  18
#define GPIO7  19
#define GPIO8  21
#define GPIO9  22
#define GPIO10 23
#define GPIO11 25

//Wi-Fi config
const char* SSID = "Schaeffer";
const char* PASSWORD = "12345678";

//MQTT broker config
const char *BROKER_MQTT = "192.168.137.179";
int BROKER_PORT = 1883;

unsigned long publishUpdate;

//MQTT topics names str
static char topic_1[30] = {""};
static char topic_2[30] = {""};
static char topic_3[30] = {""};
static char topic_4[30] = {""};

//MQTT topicos content str
static char str_topic_1[10] = {0};
static char str_topic_2[10] = {0};
static char str_topic_3[10] = {0};
static char str_topic_4[10] = {0};

WiFiClient DGADClient; // Cria o objeto espClient
PubSubClient MQTT(DGADClient);

void DGAD_PIN_DEFINE(void);
void initWiFi(void);
void initMQTT(void);
void reconnectMQTT(void);
void reconnectWiFi(void);
void check_connection_WiFI_and_MQTT(void);

//DGAD pin setup
void DGAD_PIN_DEFINE(void){

  pinMode(LEDPIN1,OUTPUT);
  pinMode(LEDPIN2,OUTPUT);
  pinMode(LEDPIN3,OUTPUT);

  pinMode(BOOTBUTTONPIN, INPUT);

  pinMode(GPIO0,  INPUT);
  pinMode(GPIO1,  INPUT);
  pinMode(GPIO2,  INPUT);
  pinMode(GPIO3,  INPUT);
  pinMode(GPIO4,  INPUT);
  pinMode(GPIO5,  INPUT);
  pinMode(GPIO6,  INPUT);
  pinMode(GPIO7,  INPUT);
  pinMode(GPIO8,  INPUT);
  pinMode(GPIO9,  INPUT);
  pinMode(GPIO10, INPUT);
  pinMode(GPIO11, INPUT);

}

void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}

void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT); // Informa qual broker e porta deve ser conectado
 // MQTT.setCallback(callbackMQTT);           // Atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}


void check_connection_WiFI_and_MQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); // se não há conexão com o Broker, a conexão é refeita

  if (WiFi.status() != WL_CONNECTED)
    reconnectWiFi(); // se não há conexão com o WiFI, a conexão é refeita
}

void reconnectWiFi(void)
{

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println(" IP obtido: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT(void)
{
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
    } 
    else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Nova tentativa de conexao em 2 segundos.");
      delay(2000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  DGAD_PIN_DEFINE();
  initWiFi();
  initMQTT();

  sprintf(topic_1, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_1);
  sprintf(topic_2, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_2);
  sprintf(topic_3, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_3);
  sprintf(topic_4, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_4);

}

void loop() {
   if ((millis() - publishUpdate) >= PUBLISH_DELAY) {
    publishUpdate = millis();
    // Verifica o funcionamento das conexões WiFi e ao broker MQTT
    check_connection_WiFI_and_MQTT();

    // Formata as strings a serem enviadas
    sprintf(str_topic_1, "%d", digitalRead(BOOTBUTTONPIN));
    sprintf(str_topic_2, "%d", digitalRead(GPIO1));
    sprintf(str_topic_3, "%d", analogRead(ANALOG_V_PIN1));
    sprintf(str_topic_4, "%.1f", analogRead(ANALOG_V_PIN1)*0.0033);

    // publica as strings contendo os sinais
    MQTT.publish(topic_1, str_topic_1);
    MQTT.publish(topic_2, str_topic_2);
    MQTT.publish(topic_3, str_topic_3);
    MQTT.publish(topic_4, str_topic_4);

    // Keep-alive da comunicação com broker MQTT
    MQTT.loop();
  }
}
