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

  // Enable CRC --> if packet is corrupted, packet gets dropped silently
  LoRa.enableCrc();
  Serial.println("Initialization complete.");
}

// MAIN LOOP
void loop() {  
  Mesh_recvFrame(LoRa.parsePacket());     // Listen to network
  Mesh_client(70);                        // Broadcasts networking data to neighbors every 70 seconds

  if(runEvery(30000)) {                   // every 30 seconds
    generatePayload();                    // generate random GPS data byte array in msgPayload[24]
    sendCoordinates(0xAA);                // send msgPayload[24] to GW (recipient = 0xAA) 
  }
  
}

// FUNCTIONS

int Mesh_recvFrame(int packetSize) { 
  if (packetSize == 0){ 
    return 0; 
  }
  if (packetSize > 30) {
    Serial.println("Received packets are not a Mesh Frame...");
    Serial.println(packetSize);
    return 0;
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
    return 0;
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
    return 41;
  }

  // Type B: Direct GW msg from other LN, do nothing
  if (msgType == 0x42) {
    Serial.println("Received Type B msg");
    Serial.println("Doing nothing...");
    return 42;
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
   return 43;
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
   return 44;
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
   return 45;   
  }  
}


void LoRa_sendPacket(byte msgType, byte recipient) {

  switch (msgType) {

    // Type A: BROADCAST from GW
    case 0x41:
      Serial.println("Sending msgType A...");
      LoRa.beginPacket();          
      LoRa.write(0x41);           // msgType 
      LoRa.write(0xFF);           // recipient, simulating BROADCAST from LNs
      LoRa.write(0xAA);           // sender, simulating GW address 0xAA
      LoRa.write(0x00);           // router, set to 0x00 bc it's a BROADCAST
      LoRa.write(0xAA);           // source is still GW (simulated)
      LoRa.write(0x00);           // sizePayload, set to 0x00 bc no payload to transmit
      LoRa.endPacket(true);
      break;

    // Type B: Direct GW msg from other LN  
    case 0x42:
      Serial.println("Sending Mesh Frame of Type B to recipient...");
      LoRa.beginPacket();
      LoRa.write(0x42);           // msgType 
      LoRa.write(recipient);      // recipient GW, 0xAA
      LoRa.write(localAddress);   // sender, simulating GW address 0xAA
      LoRa.write(0x00);           // router, set to 0x00 bc it's a BROADCAST
      LoRa.write(localAddress);   // source is still localAddress (simulated)
      LoRa.write(24);             // sizePayload
      LoRa.write(msgPayload, 24); // payload         
      LoRa.endPacket(true);
      break;

    // Type C: GW ACK  
    case 0x43:
      Serial.println("Sending msgType C...");
      LoRa.beginPacket();
      LoRa.write(0x43);           // msgType 
      LoRa.write(0x11);           // recipient 0x11
      LoRa.write(0xAA);           // sender, simulating GW address 0xAA
      LoRa.write(0x00);           // router, set to 0x00 bc it's a BROADCAST
      LoRa.write(0xAA);           // source is still GW (simulated)
      LoRa.write(0x00);           // sizePayload, set to 0x00 bc no payload to transmit      
      LoRa.endPacket(true);
      break;

    // Type D: BROADCASTS from other LNs  
    case 0x44:
      Serial.println("Sending msgType D...");
      LoRa.beginPacket();
      LoRa.write(0x44);           // msgType 
      LoRa.write(0xFF);           // recipient, simulating BROADCAST from LNs
      LoRa.write(localAddress);   // sender, simulating GW address 0xAA
      LoRa.write(0x00);           // router, set to 0x00 bc it's a BROADCAST
      LoRa.write(localAddress);   // source is still localAddres (simulated)
      LoRa.write(1);              // sizePayload 
      LoRa.write(3);              // payload, number of GWhops    
      LoRa.endPacket(true);
      break;

    // Type E: Msg to route
    case 0x45:
      Serial.println("Sending msgType E...");
      LoRa.beginPacket();
      LoRa.write(0x45);           // msgType 
      LoRa.write(0xAA);           // recipient, GW
      LoRa.write(localAddress);   // sender, localAddress
      LoRa.write(0x11);           // router, set to 0x11
      LoRa.write(localAddress);   // source, localAddress
      LoRa.write(24);             // sizePayload
      LoRa.write(msgPayload, 24); // payload  
      LoRa.endPacket(true);
      break;  

    // THROW ERROR: "msgType not found..."
    case 0x40:
      Serial.println("Sending unknown msgType ...");
      LoRa.beginPacket();
      LoRa.write(0x40);           // msgType 
      LoRa.write(0xAA);           // recipient, GW
      LoRa.write(localAddress);   // sender, localAddress
      LoRa.write(0x11);           // router, set to 0x11
      LoRa.write(localAddress);   // source, localAddress
      LoRa.write(24);             // sizePayload
      LoRa.write(msgPayload, 24); // payload  
      LoRa.endPacket(true);
      break;   

    // THROW ERROR: "msgType not found..."
    case 0x46:
      Serial.println("Sending unknown msgType...");
      LoRa.beginPacket();
      LoRa.write(0x46);           // msgType 
      LoRa.write(0xAA);           // recipient, GW
      LoRa.write(localAddress);   // sender, localAddress
      LoRa.write(0x11);           // router, set to 0x11
      LoRa.write(localAddress);   // source, localAddress
      LoRa.write(24);             // sizePayload
      LoRa.write(msgPayload, 24); // payload  
      LoRa.endPacket(true);
      break;  

      // THROW ERROR: "Packet too large..."
      case 0x47:
      Serial.println("Sending packet that's too large...");
      LoRa.beginPacket();
      LoRa.write(0x47);           // msgType 
      LoRa.write(0xAA);           // recipient, GW
      LoRa.write(localAddress);   // sender, localAddress
      LoRa.write(0x11);           // router, set to 0x11
      LoRa.write(localAddress);   // source, localAddress
      LoRa.write(24);             // sizePayload
      LoRa.write(msgPayload, 24); // payload  
      LoRa.write(1);              // one byte extra --> packetSize = 31 instead of 30
      LoRa.endPacket(true);
      break;  
  }
}

boolean Mesh_recvAck(){
  
  // Parses LoRa Packets and checks if they're ACK msgs
  // addressed at localAddress. 
  // 
  // Waits for 1 second to receive ACK Frame.
  //
  // TODO: check if LN is also ACK recipient
  
  int maxloops = 0;
  int msgType = 0;

  Serial.println("waiting 2 seconds for ACK...");
  while(msgType != 42 && maxloops < 2000) {
    msgType = Mesh_recvFrame(LoRa.parsePacket());
    delay(1);
    maxloops++;
  }

  // TODO: 
  // This only checks if a GENERAL ACK was received.
  // Check if ACK recipent == localAddress
  if(msgType == 42){
    Serial.println("ACK received");
    return true;
  } 
  return false;
}

void Mesh_sendFrame(byte msgType, byte recipient){
  
  // Sends different Types of Mesh Frames, should re-name msgType to frameType
  
  if(msgType == 0x42){
    
    // Type B msg, waits for ACK from GW
    Serial.println("Packaging payload...");  
    int resends = 0;
    LoRa_sendPacket(msgType, recipient);
    boolean ack = Mesh_recvAck();
    while(!ack && resends < 2) {
      Serial.println("ACK not received yet...Resending...");
      LoRa_sendPacket(msgType, recipient);  
      ack = Mesh_recvAck();   
      resends++; 
    }
    if(!ack) {
        Serial.println("ACK not received after 2 resends."); 
        Serial.println("Updating Routing Table to 'No Connection to recipient/GW'...");
    }    
  } else {
    Serial.println("msgType not defined yet...");
  } 
}

void sendCoordinates(byte recipient){
  
  // Sends a Type B msg (0x42) addressed to GW with msgPayload[24]
  
  Serial.println("Sending coordinates...");
  Mesh_sendFrame(0x42, recipient);
  Serial.println("Resending new coordinates in 30 seconds...");
}

void Mesh_client(int seconds){ 
  
  // Every LN implements a Mesh client that broadcasts their
  // Routing/Connection Information to their neighbors at
  // defined intervals in seconds.
  
  unsigned long long int milliSeconds = seconds * 1000;  
  if(client_runEvery(milliSeconds)) {
    
    // TODO: write broadcast function
    
    Serial.println("Broadcasting Routing Table...");
    Serial.println("Re-broadcasting Routing Table in 70 seconds.");
    return;
  } else {
    // Do nothing
    //Serial.println("Time hasn't passed...");
  }
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

void generatePayload(){
  Serial.println("Generating random GPS coordinates");
  int nodeId = random(1, 9);
  int hops = random(0,4);    
  int iLat = random(52500000, 52530000);
  int iLon = random(13200000, 13600000);
  double lat = iLat;
  double lon = iLon;
  lat = lat/1000000;
  lon = lon/1000000;

  /*
  Serial.println("Random node data:");
  Serial.println(nodeId);
  Serial.println(hops);
  printDouble(lat, 10000000);
  printDouble(lon, 10000000);
  */

  // convert to raw byte array
  memcpy(&msgPayload[0], (uint8_t *) &nodeId, sizeof(nodeId));
  memcpy(&msgPayload[4], (uint8_t *) &hops, sizeof(hops));
  memcpy(&msgPayload[8], (uint8_t *) &lat, sizeof(lat));
  memcpy(&msgPayload[16], (uint8_t *) &lon, sizeof(lon));

  /* 
  Serial.println("Generated byte array: ");
  for (int i=0; i<=23; i++) {
    Serial.println(msgPayload[i], HEX);
  }
  */
}

boolean client_runEvery(unsigned long interval){
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

boolean runEvery(unsigned long interval){
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
