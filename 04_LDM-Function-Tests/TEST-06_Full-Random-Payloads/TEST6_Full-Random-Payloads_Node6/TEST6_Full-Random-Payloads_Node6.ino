/*
All Nodes are filling their Routing Tables and Routing Statuses 
dynamically. The local Routing Status will be broadcasted to the 
network every minute, while a random GPS payload will be generated 
and sent to the Parent every two minutes. The payload also contains a 
Node's localHopCount. Every Node's hopCount can therefore be displayed
and observed at the Parent's serial port.

Node 1: 
byte localAddress = 0x11;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

Node 2: 
byte localAddress = 0x22;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

Node 3: 
byte localAddress = 0x33;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

Node 4:
byte localAddress = 0x44;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

Node 5:
byte localAddress = 0x55;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

Node 6:
byte localAddress = 0x66;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

Node 7:
byte localAddress = 0x77;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

Parent: 
byte localAddress = 0xAA;
byte localNextHopID = 0xAA;
byte localHopCount = 0x00;
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

// GLOBAL VARIABLES
int nodeMode = 0;     
byte routingTable[153] = "";
byte payload[24] = "";
byte localAddress = 0x66;
byte localNextHopID = 0x00;
byte localHopCount = 0x00;

// FUNCTION PROTOTYPES
void setup(void);
void loop(void);
bool validateID(byte nodeID);
int idToIndex(byte nodeID);
int ldm_insertRoutingTable(
  byte nodeID,
  byte hopCount,
  byte hopID,
  int Rssi, 
  float snr, 
  unsigned long currentTime
);
void ldm_printRoutingTable();
boolean runEvery(unsigned long interval);
boolean loop_runEvery(unsigned long interval);
void ldm_deleteOldEntries();
bool ldm_checkIfEmpty();
bool ldm_searchParent();
int ldm_findMinHop();
int ldm_findMaxRssi(int minHopCountInt);
int ldm_setRoutingStatus();
int ldm_bcastRoutingStatus(int mode);
int ldm_daemon(unsigned int mode);
int ldm_listener(int frameSize, int mode, bool debug);
bool ldm_checkFrameHeader(
  int mode, 
  bool debug, 
  byte sizeHeader,
  byte type, 
  byte router, 
  byte source, 
  byte recipient, 
  byte sender, 
  byte ttl, 
  byte sizePayload
);
int ldm_framehandler(
  int mode, 
  bool debug, 
  byte type, 
  byte router, 
  byte source, 
  byte recipient, 
  byte sender, 
  byte ttl
);
bool ldm_waitForAck(byte router, bool debug);
int ldm_parsePayload();
int ldm_routePayload(
  int mode, 
  byte recipient,
  byte sender, 
  byte ttl,
  int resend
);
int ldm_ackHandshake(
  int mode,
  byte type, 
  byte router, 
  byte recipient, 
  byte sender, 
  byte ttl,
  int resend
);
bool ldm_waitForAck(byte router, bool debug);
void ldm_sendFrame(
  int mode, 
  byte type, 
  byte router, 
  byte recipient, 
  byte sender, 
  byte ttl
);
void generatePayload(void);
void printDouble(double val, unsigned int precision);
void resultHandler(int result);

// FUNCTION DEFINITIONS

void setup(){
  Serial.begin(9600);                   // initialize serial
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

void loop(){
  int result = ldm_daemon(nodeMode);
  ldm_resultHandler(result);

  if(loop_runEvery(120000)){
    generatePayload();
    result = ldm_routePayload(
      nodeMode,           // Node Mode
      0xAA,               // recipient: Parent
      localAddress,       // sender
      0x0F,               // ttl
      0                   // set resend counter
    );
    if(result != 0){
      Serial.println(result);
    }
  }   
}

bool validateID(byte nodeID){
  bool valid = false;
  switch(nodeID){
    case 0x11:        // Node 1
      valid = true;
      break;
    
    case 0x22:        // Node 2
      valid = true;
      break;
    
    case 0x33:        // Node 3
      valid = true;
      break;
    
    case 0x44:        // Node 4
      valid = true;
      break;
      
    case 0x55:        // Node 5
      valid = true; 
      break;
      
    case 0x66:        // Node 6
      valid = true; 
      break;
      
    case 0x77:        // Node 7
      valid = true;
      break;
      
    case 0xAA:        // Parent Node
      valid = true; 
      break;
    
    case 0xFF:        // BCAST
      valid = true;
      break;
    
    default:        // Invalid ID
      valid = false;
      break;
  }
  return valid;
}


int idToIndex(byte nodeID){
  int index = 0;
  switch(nodeID){
    case 0xAA:
      index = 0;
      break;
      
    case 0x11:
      index = 1;
      break;
      
    case 0x22:
      index = 2;
      break;
      
    case 0x33:
      index = 3;
      break;
      
    case 0x44:
      index = 4;
      break;
      
    case 0x55:
      index = 5;
      break;
      
    case 0x66:
      index = 6;
      break;
      
    case 0x77:
      index = 7;  
      break;
    
  default:
    index = 0;
    break; 
  }
  return index;
}

int ldm_insertRoutingTable(
  byte nodeID,
  byte hopCount,
  byte hopID,
  int Rssi,
  float snr,
  unsigned long currentTime
){

  bool validNode = validateID(nodeID);
  bool validHop = validateID(hopID);
  
  if(validNode && validHop){
    int i = idToIndex(nodeID);
  
    memcpy(&routingTable[i*19], &nodeID, sizeof(nodeID));
    memcpy(&routingTable[(i*19)+1], &hopCount, sizeof(hopCount));
    memcpy(&routingTable[(i*19)+2], &hopID, sizeof(hopID));
    memcpy(&routingTable[(i*19)+3], &Rssi, sizeof(Rssi));
    memcpy(&routingTable[(i*19)+7], &snr, sizeof(snr));
    memcpy(&routingTable[(i*19)+11], &currentTime, sizeof(currentTime));  
    
    return 1;       // Success
  }
  return -9;          // E -9: invalid nodeID
}


void ldm_printRoutingTable(){
  for(int i=0; i<8; i++){
    byte nodeID = routingTable[i*19];
    byte hopCount = routingTable[(i*19)+1];
    byte hopID = routingTable[(i*19)+2];
    int Rssi = *(int*)(&routingTable[(i*19)+3]);
    float snr = *(float*)(&routingTable[(i*19)+7]);
    unsigned long currentTime = *(unsigned long*)(&routingTable[(i*19)+11]);

    Serial.print("Routing Table Entry ");
    Serial.print(i);
    Serial.println(": ");
    
    Serial.print("nodeID: 0x");
    Serial.println(nodeID, HEX);
    
    Serial.print("hopCount: ");
    Serial.println(hopCount);
    
    Serial.print("hopID: 0x");
    Serial.println(hopID,HEX);
    
    Serial.print("rssi: ");
    Serial.println(Rssi);
    
    Serial.print("snr: ");
    Serial.println(snr);
    
    Serial.print("time: ");
    Serial.println(currentTime);
    Serial.println(" ");
  }
}

boolean loop_runEvery(unsigned long interval){
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

void ldm_deleteOldEntries(){
  
  // This function sets all 19 fields of an entry to 0x00
  // when an entry's time value is older than 3 minutes (180000 ms).
  long int currentTime = millis(); 
  long int lastTime = 0;
  int timeIndex = 0;
  int difference = 0;
  int newIndex = 0;
  
  for(int i=0; i<8; i++){
    timeIndex = (i*19) + 11;
    
    lastTime = *(long int*)(&routingTable[timeIndex]);
    difference = currentTime - lastTime;
    
    if(difference > 80000){
      for (int j=0; j<18; j++){
        newIndex = (i*19) + j;
        routingTable[newIndex] = 0x00;
      }
    }
  }
}

bool ldm_checkIfEmpty(){

  // This function goes through every entry of the Routing Table
  // and checks if it's empty or not.
  
  for(int i=0; i<8; i++){
    byte entry = routingTable[i*19];
    if (entry != 0x00){
      return false; 
    }
  }
  return true;
}

bool ldm_searchParent() {

  // This function goes through every entry of the Routing Table 
  // and checks if an entry's nodeID matches the Parent ID 0xAA.

  for(int i=0; i<8; i++){
    byte entry = routingTable[i*19];
    if(entry == 0xAA){
      return true;
    }
  }
  return false;
}

int ldm_findMinHopCount(){
  int minHopCount = 255; 
  int currentHopCount = 0;
  
  for(int i=0; i<8; i++){
    byte hopCount = routingTable[(i*19)+1]; 
    currentHopCount = (int) hopCount;
    if((currentHopCount != 0) && (currentHopCount < minHopCount)){
      minHopCount = currentHopCount;
    }
  }
  return minHopCount;
}

int ldm_findMaxRssi(int minHopCount){

  // This function searches the Routing Table and compares the 
  // RSSI values of the entries containing the minHopCount. 
  // The function makes sure that the nextHop of that entry is 
  // not the localAddress, thus creating a loop.
  // The function will return the Node ID with the minHopCount and
  // highest RSSI value i.e. the best radio link.
  
  // Initial values
  int currentRssi = 0; 
  int maxRssi = -10000000;
  byte bestRoute = 0x00;
  
  for(int i=0; i<8; i++){
    byte hopCount = routingTable[(i * 19) + 1];
    byte nextHopID = routingTable[(i * 19) +2];
  
    // Make sure that the entry's nextHop is not localAddress 
    // and its hopcount matches the passed minHopCount. 
    // If the currentRssi is higher than the former maxRssi, set 
    // the currentRssi to maxRssi.
    
    if((hopCount == minHopCount) && (nextHopID != localAddress)){
      currentRssi = *(int *)(&routingTable[(i*19)+3]);
      if (currentRssi > maxRssi){
        maxRssi = currentRssi;
        bestRoute = routingTable[i * 19];         
      } 
    } 
  }
  return (int)bestRoute;
}

int ldm_setRoutingStatus(){

  // This function goes through the Routing Table and finds 
  // the neighboring Node ID with the best radio link connection and 
  // updates the global variables localHopCount and localNextHop. 
  // If the Parent has an entry inside the Routing Table, then 
  // localHopCount and localNextHop are set accordingly. 
  // If any of the function's returns result in invalid values, then 
  // localHopCount and localNextHop are set to 0x00.

  ldm_deleteOldEntries();
  bool isEmpty = ldm_checkIfEmpty();
  if(!isEmpty){
    bool parentFound = ldm_searchParent();
    if(!parentFound){
      int minHopCountInt = ldm_findMinHopCount();
      int bestRouteInt = ldm_findMaxRssi(minHopCountInt);
      
      byte minHopCount = (byte)minHopCountInt;
      byte bestRoute = (byte)bestRouteInt;
      if(minHopCount != 0x00 && bestRoute !=0x00){
        // Valid minHopCount and bestRoute found
        localHopCount = minHopCount + 1;
        localNextHopID = bestRoute; 
        return 1;
      } else {
        // minHopCount and bestRoute values are not valid
        return -1;    
      }
    } else {
      // Parent is inside the Routing Table
      localNextHopID = 0xAA;
      localHopCount = 0x01;
      return 1;
    } 
  } else {
    // Routing Table is empty
    localNextHopID = 0x00;
    localHopCount = 0x00;
    return -1;  
  }
}

bool ldm_checkFrameHeader(
  int mode,
  byte sizeHeader, 
  byte type, 
  byte router, 
  byte source, 
  byte recipient, 
  byte sender, 
  byte ttl, 
  byte sizePayload
){

  Serial.println("ldm_checkFrameHeader: something happened...");  
  // Check if header values are valid
  if(sizeHeader!= 0x08){
    Serial.println("ldm_checkFrameHeader: invalid sizeHeader");
    return false;
  }
  if(type < 0x41 || type > 0x45){
    Serial.println("ldm_checkFrameHeader: invalid type");
    return false; 
  }
  if(!validateID(router)){
    Serial.println("ldm_checkFrameHeader: invalid router");
    return false;
  }
  if(!validateID(source)){
    Serial.println("ldm_checkFrameHeader: invalid source");
    return false;
  }
  if(!validateID(recipient)){
    Serial.println("ldm_checkFrameHeader: invalid recipient");
    return false;
  }
  if(!validateID(sender)){
    Serial.println("ldm_checkFrameHeader: invalid sender");
    return false;
  }
  if(ttl > 0x0F || ttl == 0x00){
    Serial.println("ldm_checkFrameHeader: invalid ttl");
    return false;
  }
  if(sizePayload!=0x02 && sizePayload!=0x18 && sizePayload!=0x00){
    Serial.println("ldm_checkFrameHeader: invalid sizePayload");
    return false; 
  }

  Serial.println("ldm_checkFrameHeader: valid LDM Header values");
  // Check relationships between Mode, Mode-relevant
  // LDM Frame Types and router ID
  if(mode == 0){              // Node Mode
    if(type==0x43 || type==0x45){   // Type D OR Type E    
      Serial.println("ldm_checkFrameHeader: invalid type for Node Mode");   
      return false;         // Not a relevant Frame
    }
    if(type==0x41 && sender!=0xAA){
       Serial.println("ldm_checkFrameHeader: Invalid Type && sender ID"); 
       return false;
     }
    if(router!=localAddress && router!=0xFF){     
        Serial.println("ldm_checkFrameHeader: Not addressed to local"); 
        return false;       // Not a relevant Frame
    }
    Serial.println("ldm_checkFrameHeader: Relevant Frame");
    return true;            // Relevant LDM Frame 
  }
  
  if(mode == 1){              // Parent Mode
    if(type != 0x43){         // Type C: Direct Parent
      return false;         // Not a relevant Frame
    }
    if(router != localAddress){     
      return false;         // Not a relevant Frame
    }
    return true;            // Relevant LDM Frame
  }
  
  if(mode>16 && mode<171){        // ACK Mode
    if(type != 0x45){         // Type E: ACK  
      return false;         // Not a relevant Frame
    }
    if(router != localAddress){
      return false;         // Not a relevant Frame
    }
    return true;            // Relevant LDM Frame
  }
  return false;             // Not a relevant Frame
}

int ldm_frameHandler(
  int mode, 
  byte type,
  byte router,
  byte source, 
  byte recipient, 
  byte sender, 
  byte ttl
){

  // This function processes the LDM Frame according to a device's
  // mode and the header information. Before this function 
  // ldm_checkFrameHeader is evaluating whether a 
  // received Frame is valid AND relevant for the paritcular device.
  // If that is the case, then the ldm_frameHandler will process 
  // the LDM Frame accordingly, depending on type.

  Serial.println("ldm_frameHandler: something happened...");  
  int result = 0;

  if(mode == 0){              // Node Mode
    if(type == 0x41){         // Type A: Parent BCAST
      int rssi = LoRa.packetRssi();
      float snr = LoRa.packetSnr();
      unsigned long time = millis();
      result = ldm_insertRoutingTable(
        sender,           // nodeID
        0x01,             // hopCount
        0xAA,           // nextHopID
        rssi,             // RSSI
        snr,            // SNR
        time            // time 
      );
      if(result != 1){
        return result;
      }
      result = ldm_setRoutingStatus();            
      return result;          // either 1 or -1
    }
    
    if(type == 0x42){         // Type B: Neighbor BCAST
      byte hopCount = LoRa.read();  // Parsing Payload (2 Bytes)
      byte nextHopID = LoRa.read();   
      int rssi = LoRa.packetRssi();
      float snr = LoRa.packetSnr();
      unsigned long time = millis();
      
      result = ldm_insertRoutingTable(
        sender,           // nodeID
        hopCount,           // hopCount
        nextHopID,          // nextHopID
        rssi,             // RSSI
        snr,            // SNR
        time            // time 
      );
      if(result != 1){
        return result;        // E -9: invalid nodeID
      }
      result = ldm_setRoutingStatus();            
      return result;          // either 1 or -1
    }
    
    if(type == 0x44){         // Type D: Route Request
      ldm_parsePayload();
      result = ldm_routePayload(
        mode,
        recipient,
        sender,
        ttl,
        2    // don't resend when no ack
      );
      
      if(result == 1){
        ldm_sendAckBack(mode, source);
        return result;          // 1: Success
      }else{
        return result;          // E -8: No ACK
      }
    }
    return -5;                // E -5: Node Mode ERR
  } 
  
  if(mode == 1){                // Parent Mode
    if(type == 0x43){           // Type C: Direct PL
      result = ldm_parsePayload();
      if(result == 1){
        ldm_sendAckBack(mode, source);
        return result;          // 1: Success
      }
      return result;            // E -2: Parse ERR
    } 
    return -6;                // E -6: Parent Mode ERR
  }
  
  // Checking the ackMode that was passed from 
  // ldm_waitForAck() which converts the device id's HEX
  // values to DEC values. The address space of all available 
  // devices results in integers between 17 and 170. 
  // Example: 
  // Node 1 --> 0x11 --> 17 (DEC)
  // Node 2 --> 0x22 --> 34 (DEC)
  // Parent Node --> 0xAA --> 170 (DEC)
  // ldm_checkFrameHeader will already check if 
  // router/receiver == localAddress --> otherwise not relevant
  
  if(mode>16 && mode<171){          // ACK Mode
    if(type == 0x45){     
      return 1;             // 1: Success
    } 
    return -3;                // E -3: ACK Mode ERR
  }
  return -4;                  // E -4: frHandler ERR
}

int ldm_listener(int frameSize, int mode){
  if(frameSize != 0){
    Serial.println(frameSize);   
  }
  if(frameSize == 0){
    return 0;             // Nothing to receive
  }
  if(frameSize != 8 && frameSize != 32 && frameSize != 10){    
    return 0;             // Not an LDM Frame
  }

  Serial.println("Passed the frameSize test"); 
  
  // Parse Header
  byte sizeHeader = LoRa.read();
  byte type = LoRa.read();
  byte router = LoRa.read();
  byte source = LoRa.read();
  byte recipient = LoRa.read();
  byte sender = LoRa.read();
  byte ttl = LoRa.read();
  byte sizePayload = LoRa.read();
  
  bool validHeader = ldm_checkFrameHeader(
    mode, 
    sizeHeader,
    type, 
    router, 
    source, 
    recipient, 
    sender,
    ttl,
    sizePayload
  );
  
  
  if(validHeader){
    int result = ldm_frameHandler(
      mode,
      type, 
      router, 
      source, 
      recipient, 
      sender, 
      ttl
    );
    return result;            // Either 1 or E: -6 to -1
  } 
  return 0;               // Not a valid OR relevant
                      // LDM Frame
}

int ldm_parsePayload(){

  // This function parses the 24 byte payload into the 
  // local payload buffer. When the function successfully 
  // parses, it will return 1. However, should there 
  // be a case when there was an error during parsing in 
  // the future, it will return -2.

  if(LoRa.available() == 24){       // Returns # of bytes
    for(int i = 0; i<=23; i++){
      byte c = LoRa.read();
      payload[i] = c;
    }
    return 1;             // Success
  }
  return -2;                // E -2: Payload > 24 bytes
}

int ldm_routePayload(
  int mode, 
  byte recipient, 
  byte sender,
  byte ttl,
  int resend
){
  // This function sends the GPS payload to the Parent or 
  // localNestHopID depending on Routing Status. Therefore
  // this function also sets the TYPE of the LDM Frame (either Type C
  // or Type D) and passes that information to the ldm_ackHandshake()
  // function.
  //
  // The recipient of ALL LDM Payload Frames should always be the  
  // Parent ID. Check first if the localNextHopID is the Parent ID, 
  // meaning that the Node has a Direct Parent connection --> Type C 
  // LDM Frame. If not, then the LDM Frame will be of Type D: Route 
  // Request
  byte type = 0x00;

  if(recipient == localNextHopID){    
    type = 0x43;       // Type C: Direct Parent PL
  } else {
    type = 0x44;       // Type D: Route Request
  }
  
  byte router = localNextHopID;
  
  int result = ldm_ackHandshake(
    mode,
    type,
    router,
    recipient, 
    sender,
    ttl,
    resend
  );
  
  if(result == 1){    // ACK received
    return result;
  } else {
    return -8;      // ERROR -8: No ACK received
  }


}

int ldm_ackHandshake(
  int mode,
  byte type, 
  byte router, 
  byte recipient, 
  byte sender,
  byte ttl,
  int resend
){

  // This function passes on its arguments to ldm_sendFrame() 
  // in order to send a valid LDM Payload Frame. Sending Payload Frames
  // from Nodes (whether they're Routed or Direct) should always be 
  // followed by receiving an ACK Frame from the Payload router.
  
  // If no ACK was received, the Payload Frame will be resent two 
  // more times.
  
  // A successful handshake will return 1, otherwise 0.

  ldm_sendFrame(
    mode,
    type, 
    router, 
    recipient, 
    sender, 
    ttl
  );
  bool ack = ldm_waitForAck(router);
  
  while(!ack && resend<2){
    ldm_sendFrame(
      mode,
      type, 
      router, 
      recipient, 
      sender, 
      ttl
    );
    ack = ldm_waitForAck(router);
    resend++;
  }
  
  if (!ack){
    Serial.println("Did not receive ACK from router.");
    return 0;
  } else {
    return 1;
  }
}

bool ldm_waitForAck(byte router){

  // This function waits for an ACK Frame from the Node ID 
  // that was passed to this function as a router ID. 
  // The function will wait a total 2 * localHopCount seconds for the 
  // relevant ACK Frame. 
  //
  // ldm_listener() will be called in ACK Mode, meaning that the router
  // ID is passed as a Mode, so that ldm_listener ONLY listens for 
  // LDM Frames FROM the router ID.
  
  int maxloops = 0;
  int result = 0;
  int interval = 2000 * localHopCount;
  int ackMode = (int)router;
  
  while(maxloops<interval && result!=1){
    result = ldm_listener(LoRa.parsePacket(), ackMode);
    delay(1);
    maxloops++;
  }
  
  if(result == 1){    // ACK received from router
    return true;    
  }
  return false;
}

void ldm_sendAckBack(int mode, byte source){
  
  // This function prepares the LDM Header information in 
  // order to send an ACK Frame back to the source ID of the 
  // original Payload Frame (either Route Request or Direct Parent PL)
  
  byte type = 0x45;           // Type E: ACK 
  byte router = source;     
  byte recipient = source;
  byte sender = localAddress;
  byte ttl = 0x0F;

  ldm_sendFrame(
    mode,
    type,
    router,
    recipient,
    sender, 
    ttl
  );
}

void ldm_sendFrame(
  int mode,
  byte type, 
  byte router, 
  byte recipient, 
  byte sender, 
  byte ttl  
){
  
  // This function formats its arguments into a LDM Frame Header
  // and sends a complete LDM Frame using the LoRa Protocol with 
  // a random delay between 0 - 39 milliseconds.
  
  // Format the LDM Frame Header without sizePayload
  byte header[8] = ""; 
  header[0] = 0x08;       // sizeHeader
  header[1] = type;     // type
  header[2] = router;     // router 
  header[3] = localAddress;   // source
  header[4] = recipient;    // recipient
  header[5] = sender;     // sender
  header[6] = ttl - 1;    // ttl
  
  // delay the transmission of an LDM Frame for a random time interval
  delay(random(20));
  
  
  // Send the LDM Frame depending on mode using the LoRa Protocol
  if (mode == 0){               // Node Mode
    switch(header[1]){            // check type
    
      case 0x42:              // Type B: RS BCAST
        header[7] = 0x02;       // sizePayload
        LoRa.beginPacket();
        LoRa.write(header, 8);      // write LDM Header
        LoRa.write(localHopCount);    // RS payload
        LoRa.write(localNextHopID);   // RS payload 
        LoRa.endPacket(true);       // non-blocking mode
        break;
      
      case 0x43:              // Type C: Direct PL
        header[7] = 0x18;       // sizePayload
        LoRa.beginPacket();
        LoRa.write(header, 8);      // LDM Header
        LoRa.write(payload, 24);    // GPS PL
        LoRa.endPacket(true);     // non-blocking mode
        break;
        
      case 0x44:              // Type D: RRequest
        header[7] = 0x18;       // sizePayload
        LoRa.beginPacket();
        LoRa.write(header, 8);      // LDM Header
        LoRa.write(payload, 24);    // GPS PL
        LoRa.endPacket(true);     // non-blocking mode
        break;
      
      case 0x45:              // Type E: ACK
        header[7] = 0x00;       // sizePayload      
        LoRa.beginPacket();
        LoRa.write(header, 8);      // LDM Header
        LoRa.endPacket(true);     // non-blocking mode
        break;
        
      default: 
        Serial.println("Not a valid LDM Frame Type for this mode.");
        break;
      }   
  } else if (mode == 1) {           // Parent Mode
    switch(header[1]){            // check type
    
      case 0x41:              // Type A: RS BCAST
        header[7] = 0x00;       // sizePayload
        LoRa.beginPacket();
        LoRa.write(header, 8);      // LDM Header
        LoRa.endPacket(true);     // non-blocking mode
        break;
        
      case 0x45:              // Type E: ACK
        header[7] = 0x00;       // sizePayload
        LoRa.beginPacket();
        LoRa.write(header, 8);      // LDM Header
        LoRa.endPacket(true);     // non-blocking mode
        break; 
      
      default:  
        Serial.println("Not a valid LDM Frame Type for this mode.");
        break;    
    }
  } else {
    Serial.println("Not a valid mode to send an LDM Frame.");
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
  int nodeId = (int) localAddress;
  int hops = (int) localHopCount;   
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
  memcpy(&payload[0], (uint8_t *) &nodeId, sizeof(nodeId));
  memcpy(&payload[4], (uint8_t *) &hops, sizeof(hops));
  memcpy(&payload[8], (uint8_t *) &lat, sizeof(lat));
  memcpy(&payload[16], (uint8_t *) &lon, sizeof(lon));

  
  Serial.println("Generated byte array: ");
  for (int i=0; i<=23; i++) {
    Serial.println(payload[i], HEX);
  }
}

int ldm_bcastRoutingStatus(int mode){
  
  // This function sends a broadcast to the network. 
  // The LDM Frame Type depends on the passed mode value.
  int result = 0;
  if(mode == 0){
    result = ldm_setRoutingStatus();
    if(result == -1){
      return result;  
    }
    ldm_sendFrame(
      mode,       
      0x42,             // type: Type B
      0xFF,             // router: BCAST
      0xFF,             // recipient: BCAST
      localAddress,     // sender
      0x0F               // ttl
    );
  } else if(mode == 1){   // Parent Mode
    ldm_sendFrame(
      mode,       
      0x41,             // type: Type A
      0xFF,             // router: BCAST
      0xFF,             // recipient: BCAST
      localAddress,     // sender
      0x0F               // ttl
    );
  }
  return 1;
}


int ldm_daemon(unsigned int mode){

  int result = 0;
  
  if(mode > 1){
    result = -7;
    return result;                // E -7: Wrong Mode
  }
  
  if(runEvery(60000)){
    result = ldm_bcastRoutingStatus(mode);   
    return result;                // returns 1 or -1
  }
  
  result = ldm_listener(LoRa.parsePacket(), mode);
  return result;                  // returns 0: Nothing
}                                 // relevant or valid
                                  //
                                  // returns 1: Valid 
                                  // processing
                                  //
                                  // returns -8 to -1:
                                  // Processing Errors

void ldm_resultHandler(int result){
  
  switch(result){
    case 0: 
      break;            // Nothing relevant happened
      
    case 1:
      Serial.println("A LDM Frame was processed.");
      break; 
        
    case -1:
      Serial.println("ERROR -1: Routing Status couldn't be set. ");
      break;
        
    case -2:  
      Serial.println("ERROR -2: Parse Error - Payload size not valid.");
      break;
        
    case -3:
      Serial.println("ERROR -3: frameHandler Error - ACK Mode ");
      break;
        
    case -4:
      Serial.println("ERROR -4: frameHandler Error - Not a valid mode.");
      break;
      
    case -5:
      Serial.println("ERROR -5: frameHandler Error - Node Mode - Invalid type");
      break;
      
    case -6:
      Serial.println("ERROR -6: frameHandler Error - Parent Mode - Invalid type");
      break;
      
    case -7:
      Serial.println("ERROR -7: daemon Error - Not a valid daemon mode - only '0' (Node) '1' (Parent) valid");
      break;
        
    case -8:
      Serial.println("ERROR -8: No ACK received.");
      break;
      
    case -9: 
      Serial.println("ERROR -9: Couldn't insert into Routing Table - invalid nodeID");
      break;
          
    default:
      Serial.print("resultHandler Error - unknown result:  ");
      Serial.println(result);
      break;    
  }
}
      
