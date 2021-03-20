#include <SPI.h>              
#include <LoRa.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#define RADIO_CS_PIN 18
#define RADIO_DI0_PIN 26
#define RADIO_RST_PIN 14

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI

WiFiMulti WiFiMulti;

const long frequency = 868E6;  // LoRa Frequency
bool loraRecv = false;
static char loraMessage[12] = "";
static char serverMsg[26] = "";


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

  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Gateway");
  Serial.println("Only receive messages from nodes");
  Serial.println("Tx: invertIQ enable");
  Serial.println("Rx: invertIQ disable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

  //WIFI INIT
  delay(10);

  // We start by connecting to a WiFi network
  WiFiMulti.addAP("", "");
  Serial.print("Waiting for WiFi... ");

  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
}


// MAIN LOOP

void loop() {
  if (loraRecv) {

    // Sending to Server
    sendServer(&serverMsg[0]);
    Serial.println(serverMsg);

    // Sending Msg confirmation back to Node
    String message = "Server Confirmation";
    message += serverMsg;
    message += millis();

    LoRa_sendMessage(message);     
    loraRecv = false;
  }
}



// FUNCTIONS

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  
  while (LoRa.available()) {
    for(int i = 0; i<25; i++){
      char c = LoRa.read();
      loraMessage[i] = c;
  }
  Serial.print("Gateway Receive: ");
  Serial.println(loraMessage);
  loraRecv = true;
  }
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
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

void sendServer(char * serverMessage) {

    char msgBuffer[26] = "";
      
    const uint16_t port = 8888;
    const char * host = "192.168.178.50"; // ip or dns

    Serial.print("Connecting to ");
    Serial.println(host);

    WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("Connection failed.");
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        return;
    }
    // This will send a request to the server
    client.print(loraMessage);

  int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 1000) {
    maxloops++;
    delay(1); //delay 1 msec
  }
  if (client.available() > 0) {
    //read back one line from the server
    for(int i = 0; i<25; i++){
      char c = client.read();
      msgBuffer[i] = c;
    }
  }
  else {
    Serial.println("client.available() timed out ");
  }
    Serial.println("Closing connection.");
    client.stop();
    strcpy(serverMessage, msgBuffer);
}
