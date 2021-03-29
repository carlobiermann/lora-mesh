/* 
Sources at:
https://github.com/carlobiermann/lora-mesh/tree/main/02_random_data_to_db#sources
*/

#include <SPI.h>              // include libraries
#include <LoRa.h>

const long frequency = 868E6;  // LoRa Frequency

#define RADIO_CS_PIN 18
#define RADIO_DI0_PIN 26
#define RADIO_RST_PIN 14

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI

byte msg[24];

void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  SPI.begin(SCK,MISO,MOSI,RADIO_CS_PIN);
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DI0_PIN);


  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Node");
  Serial.println("Only receive messages from gateways");
  Serial.println("Tx: invertIQ disable");
  Serial.println("Rx: invertIQ enable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  if (runEvery(10000)) { 

    // generating random payload
    int nodeId = random(1, 9);
    int hops = random(0,4);    
    int iLat = random(52500000, 52530000);
    int iLon = random(13200000, 13600000);
    double lat = iLat;
    double lon = iLon;
    lat = lat/1000000;
    lon = lon/1000000;

    Serial.println("Random node data:");
    Serial.println(nodeId);
    Serial.println(hops);
    printDouble(lat, 10000000);
    printDouble(lon, 10000000);

    // convert to raw byte array
    memcpy(&msg[0], (uint8_t *) &nodeId, sizeof(nodeId));
    memcpy(&msg[4], (uint8_t *) &hops, sizeof(hops));
    memcpy(&msg[8], (uint8_t *) &lat, sizeof(lat));
    memcpy(&msg[16], (uint8_t *) &lon, sizeof(lon));
 
    Serial.println("Sending raw byte array: ");
    for (int i=0; i<=23; i++) {
      Serial.println(msg[i], HEX);
    }

    LoRa_sendMessage(&msg[0]);
  }
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage(byte *message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.write(message, 24);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  Serial.print("Node Receive: ");
  Serial.println(message);
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
