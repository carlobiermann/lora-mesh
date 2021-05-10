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
void ldm_printRoutingTable(void);
boolean runEvery(unsigned long interval);


// FUNCTION DEFINITIONS

void setup(){
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  
}

void loop(){
  // TEST: ldm_insertRoutingTable
  
  int result = 0;
  if(runEvery(5000)){   
    int currentTime = millis();        // current time
    // TEST 1: valid insert
    result = ldm_insertRoutingTable(
      0x44,             // nodeID
      0x01,             // hopCount
      0xAA,             // hopID
      -110,             // rssi
      -3.44,            // snr
      currentTime       // time
    );
    Serial.print("Result: ");
    Serial.println(result);       // Expect: 1
    
    // TEST 2: invalid nodeID
    result = ldm_insertRoutingTable(
      0x99,             // nodeID
      0x22,             // hopCount
      0xAA,             // hopID
      -110,             // rssi
      -7.32,            // snr
      currentTime       // time
    );
    Serial.print("Result: ");
    Serial.println(result);       // Expect: -9
    
    // TEST 3: invalid hopID
    result = ldm_insertRoutingTable(
      0x66,             // nodeID
      0x22,             // hopCount
      0xBB,             // hopID
      -110,             // rssi
      -1.29,            // snr
      currentTime       // time
    );
    Serial.print("Result: ");
    Serial.println(result);       // Expect: -9
    
    // TEST 4: large hopCount 
    result = ldm_insertRoutingTable(
      0x55,              // nodeID
      0xFE,             // hopCount
      0x66,             // hopID
      -109,             // rssi
      -6.38,            // snr
      currentTime       // time
    );
    
    Serial.print("Result: ");
    Serial.println(result);       // Expect: 1  
    
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
