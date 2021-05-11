/* 
Sources at:
https://github.com/carlobiermann/lora-mesh/tree/main/02_random_data_to_db#sources
*/

#include <SPI.h>              
#include <LoRa.h>
#include <WiFi.h>
#include <WiFiMulti.h>

// LoRa Chip Semtech SX1278 pins
#define SCK     5    // GPIO5  -- 
#define MISO    19   
#define MOSI    27  

// More LoRa Chip pins?
#define RADIO_CS_PIN 18
#define RADIO_DI0_PIN 26
#define RADIO_RST_PIN 14


WiFiMulti WiFiMulti;

byte localAddress = 0xAA;
byte destinationAddress = 0x11;

// LoRa EU Frequency
const long frequency = 868E6;  

bool loraRecv = false;

static byte msgPayload[24] = "";

// INIT
void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  
  // LORA INIT 
  SPI.begin(SCK,MISO,MOSI,RADIO_CS_PIN);
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DI0_PIN);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  LoRa.enableCrc();

  Serial.println("LoRa init succeeded.");

  //WIFI INIT
  delay(10);

 // We start by connecting to a WiFi network
 /* WiFiMulti.addAP("", "");
  Serial.print("Waiting for WiFi... ");

  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  */

  delay(500);
}


// MAIN LOOP
void loop() {
  
  LoRa_recvPacket(LoRa.parsePacket()); 
  
  if (loraRecv) { 
    Serial.println("Sending to server...");
        /*
    // Sending to Server
    sendServer(&msgPayload[0]);
    // Sending acknowledgement of message transfer back to LoRa Node (LN)
    char ack[4] = "ACK";
    LoRa_sendMessage(&ack[0]);     
    */   
    loraRecv = false;
  }
}



// FUNCTIONS

void LoRa_recvPacket(int packetSize) { 
  if (packetSize == 0){ 
    return; 
  }
  if (packetSize > 30) {
    Serial.println("Packet too large...");
    Serial.println(packetSize);
    return;
  }

  byte msgType = 0x00;
  byte recipient = 0x00;
  byte sender = 0x00;
  byte router = 0x00; 
  byte source = 0x00;          
  byte sizePayload = 0x00;

  // Read first byte of packet
  msgType = LoRa.read();

  // Check if msgType is between A - E  
  if (msgType < 0x41 || msgType > 0x45) {
    Serial.println("msgType not found...");
    return;
  }

  // Read the following bytes
  recipient = LoRa.read();
  sender = LoRa.read();
  router = LoRa.read();
  source = LoRa.read();
  sizePayload = LoRa.read(); // convert to integer
  
  // Type A: BROADCAST from GW, check RSSI
  if (msgType == 0x41) {
    Serial.println("Received Type A msg");
    Serial.println("Doing nothing...");
    return;
  }

  // Type B: Direct GW msg from other LN
  if (msgType == 0x42) {
    Serial.println("Received Type B msg: ");
   if (recipient == localAddress) {
    while (LoRa.available()) {
      for(int i = 0; i<=23; i++) {
        byte c = LoRa.read();
          msgPayload[i] = c;
      } 
    }
    Serial.println("Payload: ");
    for(int i = 0; i <= 23; i++) {
    Serial.println(msgPayload[i], HEX);
    }
    loraRecv = true;          
   } else {
      Serial.println("Not intended for me...");
   }
    return;
  }

  // Type C: GW ACK 
  if (msgType == 0x43) {
   Serial.println("Received Type C msg"); 
   Serial.println("Doing nothing...");
   return;
  }    

  // Type D: BROADCASTS from other LNs, check GWhops & RSSI
  if (msgType == 0x44) {
   Serial.println("Received Type D msg"); 
   Serial.println("Doing nothing...");   
   return;
  }

  // Type E: Msg to route
  if (msgType == 0x45) {
   Serial.println("Received Type E msg");
   Serial.println("Doing nothing...");
   return;   
  }  
}



boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

void sendServer(byte * serverMessage) {

    byte msgBuffer[24] = "";
      
    const uint16_t port = 8888;
    const char * host = "192.168.178.50"; // ip or dns

    Serial.print("Connecting to Server at: ");
    Serial.println(host);

    WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("Connection failed.");
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        return;
    }
    // This will send a request to the server
    client.write(serverMessage, 24);

  int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 1000) {
    maxloops++;
    delay(1); //delay 1 msec
  }
  if (client.available() > 0) {
    //read back one line from the server
    for(int i = 0; i<=23; i++){
      byte b = client.read();
      msgBuffer[i] = b;
    }
  }
  else {
    Serial.println("client.available() timed out ");
  }    
  // Print Server Message
  Serial.println("Printing Server Reply: ");
  for(int i = 0; i <=23; i++) {
    Serial.println(msgBuffer[i], HEX);
  }
  Serial.println("Closing Server connection");
  client.stop();
}
