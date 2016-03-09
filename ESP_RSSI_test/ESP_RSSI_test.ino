#include <SPI.h>
#include <WiFi.h>

//SSID of your network
char ssid[] = "";
//password of your WPA Network
char pass[] = "";

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  Serial.println("Connecting...");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Couldn't get a wifi connection");
    while(true);
  }
  // if you are connected, print out info about the connection:
  else {
   // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);
  }
}

void loop () 
{
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);
    
  }
