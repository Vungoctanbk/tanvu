#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h> //Library communication with temperature sensor
#include <DallasTemperature.h>
const int oneWireBus = 4;     

//Define a OneWire path to connect to DS18B20 temperature sensor
OneWire oneWire(oneWireBus);

//Transferring the reference of this path to the sensors object of the Dallas Temperature library.
DallasTemperature sensors(&oneWire);
#define BUT1 0
#define BUT2 16
#define LED1 17
#define LED2 5
String ledStatus1 = "ON";
String ledStatus2 = "ON";

const char* ssid = "Icebear";
const char* password = "ngaysinhcuaban";
 
#define MQTT_SERVER "192.168.1.14"
#define MQTT_PORT 1883
#define MQTT_USER "mabattu123"
#define MQTT_PASSWORD "12345678"
 
#define MQTT_LED1_TOPIC "Home/LED1"
#define MQTT_LED2_TOPIC "Home/LED2"
#define Temp "Home/Temp"
 
unsigned long previousMillis = 0; 
const long interval = 5000;
 
WiFiClient wifiClient;
PubSubClient client(wifiClient);
 
 
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_LED1_TOPIC);
      client.subscribe(MQTT_LED2_TOPIC);
      client.subscribe(Temp);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
 
void callback(char* topic, byte *payload, unsigned int length) {
  char status[20];
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  for(int i = 0; i<length; i++)
  {
    status[i] = payload[i];
  }
  Serial.println(status);
  if(String(topic) == MQTT_LED1_TOPIC)
  {
    if(String(status) == "OFF")
    {
      ledStatus1 = "OFF";
      digitalWrite(LED1, LOW);
      Serial.println("LED1 OFF");
    }
    else if(String(status) == "ON")
    {
      ledStatus1 = "ON";
      digitalWrite(LED1, HIGH);
      Serial.println("LED1 ON");
    }
  }
 
  if(String(topic) == MQTT_LED2_TOPIC)
  {
    if(String(status) == "OFF")
    {
      ledStatus2 = "OFF";
      digitalWrite(LED2, LOW);
      Serial.println("LED2 OFF");
    }
    else if(String(status) == "ON")
    {
      ledStatus2 = "ON";
      digitalWrite(LED2, HIGH);
      Serial.println("LED2 ON");
    }
  }
  if(String(topic)== Temp){
    Serial.println(status);
  }
   
}
 
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT );
  client.setCallback(callback);
  connect_to_broker();
  Serial.println("Start transfer");
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
}
 
  
void loop() {
  client.loop();
  sensors.begin();
  delay(2000);
  sensors.requestTemperatures();
  String payload = String(sensors.getTempCByIndex(0));
  client.publish(Temp, payload.c_str());
  if (!client.connected()) {
    connect_to_broker();
  }
 
  if(digitalRead(BUT1) == 0) 
  {
    while (digitalRead(BUT1) == 0)
    {
      /* cho nut dc nha */
    }
    if(ledStatus1 == "ON")
    {

 
      ledStatus1 = "OFF";
    }
    else if(ledStatus1 == "OFF")
    {
      client.publish(MQTT_LED1_TOPIC, "ON");
      ledStatus1 = "ON";
    }
  }
  if(digitalRead(BUT2) == 0) 
  {
    while (digitalRead(BUT2) == 0)
    {
      /* cho nut dc nha */
    }
    if(ledStatus2 == "ON")
    {
      client.publish(MQTT_LED2_TOPIC, "OFF");
      ledStatus2 = "OFF";
    }
    else if(ledStatus2 == "OFF")
    {
      client.publish(MQTT_LED2_TOPIC, "ON");
       ledStatus2 = "ON";
    }
  }
  
  
  delay(2000);
}
