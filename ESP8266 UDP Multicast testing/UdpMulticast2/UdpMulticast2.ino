/*
 * 31 mar 2015
 * This sketch display UDP packets coming from an UDP client.
 * On a Mac the NC command can be used to send UDP. (nc -u 192.168.1.101 2390). 
 *
 * Configuration : Enter the ssid and password of your Wifi AP. Enter the port number your server is listening on.
 *
  13 apr 2015. Partly working. Packets are received and displayed, but the confirmation packet is not received by the client.
  Tested with Mac as cleint using netcat (nc -u 192.168.1.101 2390)

 */
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

int status = WL_IDLE_STATUS;
const char* ssid = "SBI";  //  your network SSID (name)
const char* pass = "3nd0fl!ne";       // your network password

byte packetBuffer[512]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

// Multicast declarations
IPAddress ipMulti(239, 0, 0, 57);
unsigned int portMulti = 12345;      // local port to listen on

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  // setting up Station 
  WiFi.begin(ssid, pass);
  
  // Wait for connect to AP
  Serial.print("[Connecting]");
  Serial.print(ssid);
  int tries=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tries++;
    if (tries > 30){
      break;
    }
  }
  Serial.println();


printWifiStatus();

  Serial.println("Connected to wifi");
  Serial.print("Udp Multicast server started at : ");
  Serial.print(ipMulti);
  Serial.print(":");
  Serial.println(portMulti);
  Udp.beginMulticast(WiFi.localIP(),  ipMulti, portMulti);
}

void loop()
{
  int noBytes = Udp.parsePacket();
  if ( noBytes ) {
    Serial.print(millis() / 1000);
    Serial.print(":Packet of ");
    Serial.print(noBytes);
    Serial.print(" received from ");
    Serial.print(Udp.remoteIP());
    Serial.print(":");
    Serial.println(Udp.remotePort());
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,noBytes); // read the packet into the buffer

    // display the packet contents in HEX
    for (int i=1;i<=noBytes;i++){
      Serial.print(packetBuffer[i-1],HEX);
      if (i % 32 == 0){
        Serial.println();
      }
      else Serial.print(' ');
    } // end for
    Serial.println();
    Serial.println("Sendind packet now..");
    // send a reply, to the IP address and port that sent us the packet we received
//    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.beginPacket(ipMulti, portMulti);
    Udp.write("ESP-");
    Udp.print(ESP.getChipId(),HEX);
    Udp.print(":");
    Udp.print(WiFi.localIP());
    Udp.endPacket(); //- See more at: http://www.esp8266.com/viewtopic.php?f=29&t=2464#sthash.EacJR4iq.dpuf
  } // end if



}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}
