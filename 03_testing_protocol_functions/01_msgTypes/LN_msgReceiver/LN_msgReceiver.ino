/* 
Sources at:
https://github.com/carlobiermann/lora-mesh/tree/main/02_random_data_to_db#sources
*/

#include <SPI.h>              
#include <LoRa.h>

// LoRa Chip Semtech SX1278 pins
#define SCK     5    // GPIO5  -- 
#define MISO    19   
#define MOSI    27   

// More LoRa Chip pins?
#define RADIO_CS_PIN 18
#define RADIO_DI0_PIN 26
#define RADIO_RST_PIN 14

// LoRa EU Frequency
const long frequency = 868E6;  

byte localAddress = 0x11;
byte destinationAddress = 0xAA;

static byte msgPayload[24] = "";
static byte msg[24] = "";

// INIT
void setup() {
  Serial.begin(9600);                   
  while (!Serial);

  SPI.begin(SCK,MISO,MOSI,RADIO_CS_PIN);
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DI0_PIN);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.enableCrc();
}

// MAIN LOOP
void loop() {  
  LoRa_recvPacket(LoRa.parsePacket());
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
  sizePayload = LoRa.read(); // conver to integer
  
  // Type A: BROADCAST from GW, check RSSI
  if (msgType == 0x41) {
    Serial.println("Received Type A msg");
    int rssi = LoRa.packetRssi();
    Serial.print("RSSI from last GW packet: ");
    Serial.println(rssi);
    return;
  }

  // Type B: Direct GW msg from other LN, do nothing
  if (msgType == 0x42) {
    Serial.println("Received Type B msg");
    Serial.println("Doing nothing...");
    return;
  }

  // Type C: GW ACK 
  if (msgType == 0x43) {
   Serial.println("Received Type C msg"); 
   if (recipient == localAddress) {
      Serial.println("ACK received");     
   } else if (router == localAddress) {
      Serial.println("Routing ACK...");  
   } else {
      Serial.println("ACK not intended for me...");
   }
   return;
  }    

  // Type D: BROADCASTS from other LNs, check GWhops & RSSI
  if (msgType == 0x44) {
   Serial.println("Received Type D msg");    
   byte gwHops = LoRa.read();
   int rssi = LoRa.packetRssi();
   Serial.print("Broadcast from: ");
   Serial.println(sender);
   Serial.print("RSSI: ");
   Serial.println(rssi);
   return;
  }

  // Type E: Msg to route
  if (msgType == 0x45) {
   Serial.println("Received Type E msg");
   if (recipient == localAddress) {
    while (LoRa.available()) {
      for(int i = 0; i<=23; i++) {
        byte c = LoRa.read();
          msgPayload[i] = c;
      } 
    }  
   } else if (router == localAddress) {
          Serial.println("Routing payload...");
   }
   return;   
  }  
}



void LoRa_sendMessage(byte *message) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destinationAddress);
  LoRa.write(localAddress);
  LoRa.write(24);                 // size of payload
  LoRa.write(message, 24);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
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

void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
     frac = (val - int(val)) * precision;
   else
      frac = (int(val)- val ) * precision;
   int frac1 = frac;
   while( frac1 /= 10 )
       precision /= 10;
   precision /= 10;
   while(  precision /= 10)
       Serial.print("0");

   Serial.println(frac,DEC) ;
}


/*
  // Print ACK
  for(int i = 0; i <= 3; i++) {
     Serial.println(loraMessage[i], HEX);
  }

*/
