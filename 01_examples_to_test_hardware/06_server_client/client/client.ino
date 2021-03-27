/* 
 Code from: https://techtutorialsx.com/2018/05/17/esp32-arduino-sending-data-with-socket-client/
*/


#include <WiFi.h>
 
const char* ssid = "";
const char* password =  "";
 
const uint16_t port = 8888;
const char * host = "192.168.xx.xx";
 
void setup()
{
 
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }
 
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
 
}
 
void loop()
{
    WiFiClient client;
 
    if (!client.connect(host, port)) {
 
        Serial.println("Connection to host failed");
 
        delay(1000);
        return;
    }
 
    Serial.println("Connected to server successful!");

    delay(1000);
    client.print("Hello from ESP32!");
 
    Serial.println("Disconnecting...");
    client.stop();
 
    delay(10000);
}
