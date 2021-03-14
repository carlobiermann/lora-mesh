#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  868E6

String packSize = "--";
String packet ;

void setup() {
  Serial.begin(115200);
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  LoRa.setSpreadingFactor(9);
  LoRa.begin(BAND);
  Serial.println("Initializiation completed.");
  delay(1500);
}

void loop() {
  LoRa.beginPacket();
  LoRa.print("Hello from A");
  LoRa.endPacket();
  LoRa.onTxDone(onTxDone);

  delay(5000);
  Serial.println("Next loop");
}

// Functions

void onTxDone(){
  LoRa.receive();
  int packetSize = LoRa.parsePacket();
   
  if (packetSize){ 
    cbk(packetSize);  
  } 
  delay(10);
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  
  for (int i = 0; i < packetSize; i++) { 
    packet += (char) LoRa.read(); 
  }
}
