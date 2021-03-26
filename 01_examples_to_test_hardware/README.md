
## Examples to test the hardware

|Folder    |Description |Sources |
|----------|------------|-------|
|[01_sender-receiver-trials](01_sender-receiver-trials)|Example sketches of simple LoRa *sender* and *receiver* programs by Heltec and LilyGO|[Heltec Sender](https://github.com/HelTecAutomation/Heltec_ESP32/blob/master/examples/LoRa/LoRaSender/LoRaSender.ino), [Heltec Receiver](https://github.com/HelTecAutomation/Heltec_ESP32/blob/master/examples/LoRa/LoRaReceiver/LoRaReceiver.ino), [T-Beam Sender](https://github.com/LilyGO/TTGO-T-Beam/blob/master/OLED_LoRa_Sender/OLED_LoRa_Sender.ino), [T-Beam Receiver](https://github.com/LilyGO/TTGO-T-Beam/blob/master/OLED_LoRa_Receive/OLED_LoRa_Receive.ino)|
|[02_sender-receiver-interoperable](02_sender-receiver-interoperable)|Same programs as above, but using the same *Spreading Factor* to make the devices interoperable | Same as above |
|[03_RxGPS_t-beam](03_RxGPS_t-beam)|Example sketch provided by LilyGO to test the GPS module |[LilyGO GPS](https://github.com/LilyGO/TTGO-T-Beam/blob/master/GPS/GPS.ino)|
|[04_RxGPS_t-beam_update](04_RxGPS_t-beam_update)|Updated GPS example by Xinyuan-LilyGO|[Xinyuan-LilyGO GPS](https://github.com/Xinyuan-LilyGO/LilyGO-T-Beam/blob/master/examples/GPS/TinyGPS_Example/TinyGPS_Example.ino)|
|[05_multipleTx_singleRx](05_multipleTx_singleRx)|Checking the behaviour of a *single* receiver with *multiple* senders| Based on same sources as [01_sender-receiver-trials](01_sender-receiver-trials) |
|[06_server_client](06_server_client)|Simple Client-Server connection between a T-Beam client and a socket server program in C| [ESP32 Socket Client Example](https://techtutorialsx.com/2018/05/17/esp32-arduino-sending-data-with-socket-client/), [Socket Server Example in C](https://www.binarytides.com/server-client-example-c-sockets-linux/)|
|[07_alternating_Rx-Tx](07_alternating_Rx-Tx)|Duplex LoRa connection between two T-Beams. Both alternating between *sender* and *receiver* mode| [Duplex T-Beam A](https://hutscape.com/tutorials/lora-duplex-a-esp32-t-beam) [Duplex T-Beam B](https://hutscape.com/tutorials/lora-duplex-b-esp32-t-beam)|
|[08_alternating_Rx-Tx_callback](08_alternating_Rx-Tx_callback)|Same as above but using *callbacks* |[LoRa.h Duplex Callback example](https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaDuplexCallback/LoRaDuplexCallback.ino) |
|[09_gateway-node-duplex](09_gateway-node-duplex)|Simple duplex connection between a T-Beam *gateway* and a T-Beam *node* based on callbacks|[LoRa.h SimpleGateway](https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaSimpleGateway/LoRaSimpleGateway.ino) [LoRa.h SimpleNode](https://github.com/sandeepmistry/arduino-LoRa/tree/master/examples/LoRaSimpleNode) |
