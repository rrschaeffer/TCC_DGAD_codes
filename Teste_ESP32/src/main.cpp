#include <Arduino.h>
#include <WiFi.h>

#define DELAY 200

#define LEDPIN1 4
#define LEDPIN2 5
#define LEDPIN3 2

#define BUTTONPIN  0

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


// put function declarations here:
// Replace with your network credentials
const char* ssid = "Schaeffer";
const char* password = "12345678";
//const char* password = "";

int analogValue;

void setup() {
  // put your setup code here, to run once:
  pinMode(LEDPIN1,OUTPUT);
  pinMode(LEDPIN2,OUTPUT);
  pinMode(LEDPIN3,OUTPUT);

  pinMode(BUTTONPIN, INPUT);

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

  //pinMode(35, INPUT);
Serial.begin(115200);
Serial.println("\nConectando DGAD a rede WiFi...");
// Set WiFi mode to station
WiFi.mode(WIFI_AP_STA);
WiFi.begin(ssid, password);

// Wait until connected
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}

// Print connection details
Serial.println("\nDGAD Conectado!");
Serial.print("IP do dispositivo: ");
Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(DELAY);
  digitalWrite(LEDPIN1, HIGH);
  delay(DELAY);
  digitalWrite(LEDPIN2, HIGH);
  delay(DELAY);
  digitalWrite(LEDPIN3, HIGH);
  delay(DELAY);
  digitalWrite(LEDPIN1, LOW);
  delay(DELAY);
  digitalWrite(LEDPIN2, LOW);
  delay(DELAY);
  digitalWrite(LEDPIN3, LOW);

  Serial.printf("Wifi status: ");
  Serial.println(WiFi.status());

  Serial.print("BOOT button status: ");
  Serial.println(digitalRead(BUTTONPIN));

  Serial.print("Digital0 status: ");
  Serial.println(digitalRead(GPIO0));

  Serial.print("Digital1 status: ");
  Serial.println(digitalRead(GPIO1));

  Serial.print("Digital2 status: ");
  Serial.println(digitalRead(GPIO2));

  Serial.print("Digital3 status: ");
  Serial.println(digitalRead(GPIO3));

  Serial.print("Digital4 status: ");
  Serial.println(digitalRead(GPIO4));

  Serial.print("Digital5 status: ");
  Serial.println(digitalRead(GPIO5));

  Serial.print("Digital6 status: ");
  Serial.println(digitalRead(GPIO6));

  Serial.print("Digital7 status: ");
  Serial.println(digitalRead(GPIO7));

  Serial.print("Digital8 status: ");
  Serial.println(digitalRead(GPIO8));

  Serial.print("Digital9 status: ");
  Serial.println(digitalRead(GPIO9));

  Serial.print("Digital10 status: ");
  Serial.println(digitalRead(GPIO10));

  Serial.print("Digital11 status: ");
  Serial.println(digitalRead(GPIO11));

  Serial.printf("Valor Analog0 0-10V: ");
  analogValue = analogRead(ANALOG_V_PIN1);
  Serial.println(analogValue);

  Serial.printf("Valor Analog1 0-10V: ");
  analogValue = analogRead(ANALOG_V_PIN2);
  Serial.println(analogValue);

  Serial.printf("Valor Analog2 0-10V: ");
  analogValue = analogRead(ANALOG_V_PIN3);
  Serial.println(analogValue);

  Serial.printf("Valor Analog0 4-20mA: ");
  analogValue = analogRead(ANALOG_A_PIN1);
  Serial.println(analogValue);

  Serial.printf("Valor Analog1 4-20mA: ");
  analogValue = analogRead(ANALOG_A_PIN2);
  Serial.println(analogValue);

  Serial.printf("Valor Analog2 4-20mA: ");
  analogValue = analogRead(ANALOG_A_PIN3);
  Serial.println(analogValue);
  Serial.printf("\n\n");
  
 // digitalWrite(2, digitalRead(0)); 
}