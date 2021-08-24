LoRaDiMe 0.2 implements Link Hysteresis. This means that
once the localNextHopID (i.e. neighbor/parent link) has been set, 
the highest link quality metric that is measured from this 
node is saved from the RSSI value.
If at any point the RSSI value from localNextHopID falls below the 
LINK_THRESHOLD, then a new Routing Status will be set. If not,
a Node will maintain its link to localNextHopID without any change.

This algorithm change might impact how mobile the Nodes can be within its  
network. 

The network will be tested with varying timing parameters.