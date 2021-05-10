// GLOBAL VARIABLES
int mode = 0;
byte localAddress = 0x11;

byte routingTable[153] = "";
byte localNextHopID = 0x22;
byte localHopCount = 0x04;


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
void ldm_deleteOldEntries();
bool ldm_checkIfEmpty();
bool ldm_searchParent();
int ldm_findMinHop();
int ldm_findMaxRssi(int minHopCountInt);
int ldm_setRoutingStatus();

// FUNCTION DEFINITIONS

void setup(){
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  
}

void loop(){
  // TEST: ldm_setRoutingStatus
  
  int result = 0;
  if(runEvery(5000)){   
       
  // Routing Table is empty
  result = ldm_setRoutingStatus();
  Serial.print("Expecting -1: ");
  Serial.print(result);
  
  Serial.print("localNextHopID - Expecting 0x00: 0x");
  Serial.println(localNextHopID, HEX);
  
  Serial.print("localHopCount - Expecting 0x00: 0x");
  Serial.println(localHopCount, HEX);
       
  // Fill Routing Table with entries

  // Entry 1
    result = ldm_insertRoutingTable(
      0x77,             // nodeID
      0x04,             // hopCount
      0x22,             // hopID
      -300,             // rssi
      -5.24,            // snr
      180600       // time
    );
    Serial.print("Expecting 1: ");
    Serial.println(result);       // Expect: 1

  // Routing Table has one entry
  result = ldm_setRoutingStatus();
  Serial.print("Expecting 1: ");
  Serial.println(result);
  
  Serial.print("localNextHopID - Expecting 0x77: 0x");
  Serial.println(localNextHopID, HEX);
  
  Serial.print("localHopCount - Expecting 0x05: 0x");
  Serial.println(localHopCount, HEX);

  // Entry 2
    result = ldm_insertRoutingTable(
      0x66,             // nodeID
      0x04,             // hopCount
      0x33,             // hopID
      -100,             // rssi
      -4.75,            // snr
      180700       // time
    );
    Serial.print("Result: ");
    Serial.println(result);       // Expect: 1
  
      
  // Routing Table has two entries
  result = ldm_setRoutingStatus();
  Serial.print("Expecting 1: ");
  Serial.println(result);
  
  Serial.print("localNextHopID - Expecting 0x66: 0x");
  Serial.println(localNextHopID, HEX);
  
  Serial.print("localHopCount - Expecting 0x05: 0x");
  Serial.println(localHopCount, HEX);
  
  // Entry 3
    result = ldm_insertRoutingTable(
      0xAA,             // nodeID
      0x00,             // hopCount
      0xAA,             // hopID
      -300,             // rssi
      -1.29,            // snr
      180700       // time
    );
    Serial.print("Result: ");
    Serial.println(result);       // Expect: 1
  
  // Routing Table has three entries
  result = ldm_setRoutingStatus();
  Serial.print("Expecting 1: ");
  Serial.println(result);
  
  Serial.print("localNextHopID - Expecting 0xAA: 0x");
  Serial.println(localNextHopID, HEX);
  
  Serial.print("localHopCount - Expecting 0x01: 0x");
  Serial.println(localHopCount, HEX);
    
  // Entry 4
    result = ldm_insertRoutingTable(
      0xAA,             // nodeID
      0x00,             // hopCount
      0xAA,             // hopID
      -124,             // rssi
      -6.21,            // snr
      179999           // time
    );
    Serial.print("Result: ");
    Serial.println(result);       // Expect: 1
    
  // Routing Table has three entries
  result = ldm_setRoutingStatus();
  Serial.print("Expecting 1: ");
  Serial.println(result);
  
  // Because Parent entry is now more than 3 minutes old
  Serial.print("localNextHopID - Expecting 0x66: 0x");
  Serial.println(localNextHopID, HEX);
  
  Serial.print("localHopCount - Expecting 0x05: 0x");
  Serial.println(localHopCount, HEX);
  
    ldm_printRoutingTable();      // Expect: two valid entries

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
  
  for(int i=0; i<8; i++){
    long int currentTime = millis(); 
    long int lastTime = *(long int*)(&routingTable[(i*19)+11]);
    
    if((currentTime - lastTime) > 180000){
      for (int j=0; i<18; i++){
        routingTable[(i*19)+j] = 0x00;
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
        localHopCount = 0x00;
        localNextHopID = 0x00; 
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
