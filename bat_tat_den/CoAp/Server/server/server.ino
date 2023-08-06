#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

const char* ssid = "Icebear";
const char* password = "ngaysinhcuaban";

// UDP và đối tượng CoAP
WiFiUDP udp;
Coap coap(udp);

// Địa chỉ IP và cổng của ESP8266 #1
IPAddress ip(192, 168, 1, 58);
int port = 5683; // Cổng mặc định cho CoAP

// Callback nhận yêu cầu CoAP và hiển thị nhiệt độ
void callback_temperature(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[CoAP Request received]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = '\0';
  
  float temperature = atof(p);
  
  Serial.print("Temperature received: ");
  Serial.println(temperature);
}

void setup() {
  Serial.begin(115200);
  
  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Đăng ký endpoint để nhận yêu cầu CoAP
  coap.server(callback_temperature, "temperature");

  // Khởi động CoAP
  coap.start();
}

void loop() {
  coap.loop();
}
