
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>

#define LED 5 // LED sáng là đã bật điều hòa
 
#define MQTT_TEMP_TOPIC1 "TEMP1" //Đặt topic là HEAT 
#define MQTT_DKTEMP_TOPIC1 "DKTEMP1" // topic mà thiết bị cần sub
#define MQTT_AUTO_TOPIC "AUTO"

#define MQTT_SERVER "192.168.1.14" //Địa chỉ IP của máy tính

const char* ssid = "Icebear"; // Setup wifi và passs
const char* password = "ngaysinhcuaban";
const int oneWireBus = 4; //Setup chân truyền dữ liệu

OneWire oneWire(oneWireBus); // Setup hàm truyền nhận và xử lý dữ liệu
DallasTemperature sensors(&oneWire);

WiFiClient wifiClient; //Setup Client PubSub
PubSubClient client(wifiClient);

float temperatureC; //Biến đo nhiệt
float setupnhiet = 32.5; //Nhiệt độ cài đặt bật điều hòa
int batbom=0;
int auto1=0;
int i=0;


//Hàm kết nối vào wifi
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
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

//Kết nối vào broker
void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Client1";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_DKTEMP_TOPIC1); //Subcribe topic mong muốn
      client.subscribe(MQTT_AUTO_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(1000);
    }
  }
}

//callback từ broker nếu topic thay đổi 
void callback(char* topic, byte *payload, unsigned int length) {
  String status;
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  for(int i = 0; i<length; i++)
  {
    status +=(char)payload[i];
  }
  Serial.println(status);
  if(String(topic) == MQTT_DKTEMP_TOPIC1)  //Xử lý thông tin nhận về
  {
    if(String(status) == "OFF")
    {
      batbom=0;
      digitalWrite(LED, LOW);
    }
    if(String(status) == "ON")
    {
      batbom=1;
      digitalWrite(LED, HIGH);
    }
  }
  if(String(topic) == MQTT_AUTO_TOPIC)
  { 
   if(String(status) == "OFF")
    {
      auto1=0;
    }
    if(String(status) == "ON")
    {
      auto1=1; 
    }
  }
  /*if(String(topic) == MQTT_SETTEMP_TOPIC)
  {
    setupnhiet = String(status).toFloat();
  }*/
}

void setup() {
// put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  Serial.begin(115200); // Setup baurate
  sensors.begin(); //Bật cảm biến
}

void loop() {
// put your main code here, to run repeatedly:
  if (!client.connected()) 
  { //Chưa kết nối broker được thì kết nối lại 
    connect_to_broker();
  }
  i++;
  if(i%100)// Tránh đọc giá trị cảm biến liên tục
  {
    sensors.requestTemperatures(); 
    temperatureC = sensors.getTempCByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    char tempString[8];
    dtostrf(temperatureC, 1, 2, tempString);
    client.publish(MQTT_TEMP_TOPIC1, tempString);
    delay(1000);
  }
  if(auto1 == 1) // Nếu ở chế độ auto
  {
    if(temperatureC > setupnhiet) // Nhiệt độ lớn hơn nhiệt độ đặt
    { 
      if(batbom==0)
      {
      batbom=1;
      digitalWrite(LED, HIGH);
      }
    }
    else if(temperatureC < setupnhiet ) // Nhiệt độ nhỏ hơn nhiệt độ đặt
    {
      if(batbom==1)
      {
      batbom=0;
      digitalWrite(LED, LOW);
      }
    } 
  }  
  client.loop();
  delay(100);
}
