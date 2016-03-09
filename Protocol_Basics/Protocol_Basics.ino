/*
 * Blasto V2 Protocol
 * Modules:  
 * Target
 * Gun
 * Poofer
 * Master Control
 * 
 * ID = ESP8266 Chip ID
 * 
 * TCP Socket Connection Packet Protocol
 * Master Control TCP Server, Modules TCP Clients
 * Master Control listens for connections - all modules connect to master control
 * 
 * Module First Message: "Hello"; Module ; ID
 * Response from Master Control: "ACK" ID
 * 
 * Sender   Destination Message   Data        Notes
 * All      All         Ack       Sender ID   Acknowledged
 * All      All         NAck      Sender ID   Not Acknowledged
 * All      Master      Hello     Sender ID   Response should be Ack.
 * Master   All         StillThere            Check to see if destination still active
 * Gun      Master      Fire!     Sender ID
 * Gun      Master      Click!    Sender ID
 * Master   Gun         Enable    
 * Master   Gun         Disable   
 * Master   Gun         Reset
 * Target   Master      Hit       Sender ID
 * Target   Master      Report    Timer        Time is Time between target becoming active and being hit.
 * Target   Master      T.Elapsed Timer Elapsed
 * Master   Target      Enable    
 * Master   Target      Disable
 * Master   Target      Set Timer / Enable
 * Master   Target      SetColor  0xRRGGBB    set ring color
 * Poofer   Master      Heartbeat (safety heartbeat) - if ack not recieved, go to safe mode.
 * Master   Poofer      Ack
 * Master   Poofer      Poof      Time    
 * 
 * Master Control will need to create 3 tables in memory..
 * Guns, Targets, and Poofers
 * 
 * Guns Table - ID, Active, 
 */


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
