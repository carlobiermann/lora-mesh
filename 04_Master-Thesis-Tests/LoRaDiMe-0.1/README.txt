LoRaDiMe 0.1 

Whenever a Node receives a Routing Status/Parent Broadcast, an entry 
into its Routing Table is made. After that a Node will first look for an 
entry of the Parent. If the Parent is not present, then it will look into 
for the lowest hop count of all its RT entries. Then entry with the lowest 
hop count AND highest RSSI value will be the localNextHopID.

Setting the Routing Status upon every Broadcast reception and before every
may result in quite frequent topology changes as the RSSI values also vary broadly. 

The network will be tested with varying timing parameters.