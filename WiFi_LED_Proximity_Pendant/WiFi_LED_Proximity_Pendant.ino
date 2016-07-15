/*
 *  Modified by Dan Damron for Kanuckistan Camp Prosimity sensors
 *  
 *  This sketch demonstrates how to scan WiFi networks. 
 *  The API is almost the same as with the WiFi Shield library, 
 *  the most obvious difference being the different file you need to include:
 *  
 *  
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <FastLED.h>
const char * ssid = "KANUCK";
const char *password = "thereisnospoon";
#define LED_PIN 12
#define NUM_LEDS 12
#define BRIGHTNESS 64
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
}

void loop() {
  int friend_count = 0;
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
      friend_count++;
    }
  }
  Serial.println("");
  if (friend_count > 12) friend_count = 12;
  Serial.print("Friends:");
  Serial.println(friend_count);
  for (int a = 0; a > friend_count; a++) {
    leds[a] = CRGB::Red;
  }
  for (int a = friend_count; a<12; a++) {
    leds[a] = CRGB::Black;
  }
  FastLED.show();
  // Wait a bit before scanning again
  delay(1000);
  
}
