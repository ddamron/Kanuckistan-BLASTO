/*
 * BLASTO V2
 * 
 */



#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <IRremoteESP8266.h>
#include <FastLED.h>
#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
// select mode..
//#define TARGET 1
#define GUN 1
//#define BLASTER 1

// Define max clients
#define MAX_SRV_CLIENTS 10
// hardware pins
#define IRTXPIN 5
#define IRRXPIN 4
#define SOLENOID 13
#define ON 1
#define OFF 0
#define TCPPORT 2016


#ifdef TARGET
  // LEDS
  #define LED_PIN 12
  #define NUM_LEDS 24
  #define BRIGHTNESS 64
  #define LED_TYPE WS2811
  #define COLOR_ORDER GRB
  CRGB leds[NUM_LEDS];
  IRrecv irrx(IRRXPIN);
#endif

#ifdef GUN
  #define TRIGGER 4 // IR rx pin
  IRsend irtx(IRTXPIN);
#endif
IPAddress messageTargetIP(192,168,2,2);
int status = WL_IDLE_STATUS;
const char* ssid = "BLASTO";
const char* password = "Kanuckistan2016";
char command[10]; // array to hold transmit message
char response[10]; // array to hold recieve message
byte packetBuffer[512]; // buffer to hold incoming and outgoing packets
WiFiServer server(2016);
WiFiClient serverClients[MAX_SRV_CLIENTS];
WiFiUDP Udp;
IPAddress ipMulti(239,0,0,57);
unsigned int portMulti = 2016; // local port to listen on

// Gun definitions here..
uint8_t safety = ON;
int timeron = 0;
unsigned long timerstart;
unsigned long timerlength;
uint16_t irbuf[8]; // buffer for IR message
CRGB fgcolor, bgcolor;


void packIRBuff() {
  uint32_t myid = ESP.getChipId();
  // pack the chip id into the first 4 bytes..
  irbuf[0] = (myid & 0xFFFF0000UL) >> 16;
  irbuf[1] = (myid & 0x0000FFFFUL);
  
}
void setup() {
  String chipID = String(system_get_chip_id(), HEX);
  packIRBuff();  
  
  #ifdef GUN
    // create send packet
    pinMode(TRIGGER, INPUT_PULLUP); // led data pin wired to switch to GND.
    pinMode(SOLENOID, OUTPUT); // define solenoid as output possibly for vibration motor kickback
    digitalWrite(SOLENOID, 0); // and make false
    irtx.begin();
    //irrx.enableIRIn(); // enable receive on IR
  #endif
  #ifdef TARGET
    pinMode(SOLENOID, OUTPUT);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    irrx.enableIRIn();
  #endif
  #ifdef BLASTER
    pinMode(SOLENOID, OUTPUT);
  
  #endif

  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to "); Serial.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if (i == 21) {
    Serial.print("Could not connect to "); Serial.println(ssid);
    while(1) delay(500);
  }
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("UDP Multicast server started at: ");
  Serial.print(ipMulti);
  Serial.print(":");
  Serial.println(portMulti);
  Udp.beginMulticast(WiFi.localIP(), ipMulti, portMulti);
  server.begin();
  server.setNoDelay(true);
  Serial1.print("Ready! Use 'telnet ");
  Serial1.print(WiFi.localIP());
  Serial1.println(" 2016' to connect");
  
}

#ifdef TARGET

void color_wipe(int fgpercent) {
  //Serial.println("Inside Color_wipe");
  //Serial.print("Percent = ");
  //Serial.println(fgpercent);
  int i;
  int fgleds = (fgpercent * NUM_LEDS) / 100; // calculation of # of leds using fgcolor
  int blackleds = NUM_LEDS - fgleds;  // calculation of # of leds using bgcolor
  //Serial.print("FGleds:"); Serial.print(fgleds);
  //Serial.print("\tbgleds:"); Serial.println(blackleds);
  for (i = 0; i < blackleds; i++) {
    leds[i] = bgcolor;
  }
  for (i = blackleds; i < NUM_LEDS; i++) {
    leds[i] = fgcolor;
  }
  //Serial.println("Just before show");
  FastLED.show();
  //Serial.println("leaving color_wipe");
}

void activateTarget(CRGB color) {
  bgcolor = CRGB::Black;
  fgcolor = color;
  color_wipe(100);
  irrx.enableIRIn();
}


void deactivateTarget() {
  color_wipe(0);
  irrx.disableIRIn();
}
void color_timer(CRGB backcolor, CRGB forecolor, int timeout) {
  //Serial.println("Inside color_timer");
  fgcolor = forecolor;
  bgcolor = backcolor;
  timerstart = millis();
  timerlength = timeout * 1000;
  timeron = 1;
  color_wipe(100);
  irrx.enableIRIn();
  //Serial.println("End of color_timer");
}

void update_timer() {
  //Serial.println("Inside update_timer");
  unsigned long elapsedtime = millis() - timerstart;
  if (elapsedtime > timerlength) {  // end timer here
    timeron = 0;
    color_wipe(0);
    irrx.disableIRIn();
    //TODO: send packet to MC to indicate timer expired.
  } else {
    //Serial.print("Timer Length:");Serial.print(timerlength);
    //Serial.print("\tElapsed Time:");Serial.print(elapsedtime);
    int percent = (timerlength - elapsedtime)* 100 / timerlength;
    //Serial.print("Update_timer Percent:");Serial.println(percent);
    color_wipe(percent);
  }
//Serial.println("End of update_timer");
}
#endif



void loop() {
/*  uint8_t i;
  Serial.print(".");

  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!wificlient[i] || !wificlient[i].connected()){
        if(wificlient[i]) wificlient[i].stop();
        wificlient[i] = server.available();
        Serial.print("\nNew client: "); Serial.println(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
    Serial.println("No Free Clients - rejected");
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (wificlient[i] && wificlient[i].connected()){
      if(wificlient[i].available()){
        //get data from the telnet client and push it to the UART
        while(wificlient[i].available()) Serial.write(wificlient[i].read());
      }
    }
  }
*/
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial.print("New client: "); Serial.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
        
      }
    }
  }
  //check UART for data
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }
#ifdef GUN
  if (!digitalRead(TRIGGER)) {     // trigger pulled

    irtx.sendDISH(ESP.getChipId(), 32);
    Serial.println("\nTrigger pulled!");
    digitalWrite(SOLENOID, true); // Turn on Vibe
    delay(500);
    digitalWrite(SOLENOID, false); // turn off Vibe
    // send IP Packet

    
  }
  
#endif
#ifdef TARGET
  decode_results results;
  if (irrx.decode(&results)) {
    Serial.print("IR IN:");
    dumpRaw(&results);
    // validate results
    // send IP Packet
    // activate lights
    // delay
    
    irrx.resume();
  }
  if (timeron) {
    //Serial.print("+");
    update_timer();
  } else  {
    
    Serial.println("Activating Color Timer!");
    
//    fgcolor = CRGB::Red;
//    bgcolor = CRGB::Green;
//    color_wipe(100);
//    delay(1000);
//    color_wipe(75);
//    delay(1000);
//    color_wipe(50);
//    delay(1000);
//    color_wipe(25);
//    delay(1000);
//    color_wipe(0);
//    delay(1000);
    safety = false;
    blast(2);
    delay(1000);
    color_timer(CRGB::Blue, CRGB::Red, 1);
    //Serial.println("End of loop");
  }
  blast(-1);

#endif
#ifdef BLASTER
  /* 
   *    wait for a IP packet
   *    is packet disable?
   *      yes - make non active
   *      reset watchdog
   *    is packet fire?
   *      is active?
   *        yes - fire
   *        no - ignore
   *        reset watchdog
   *    is packet enable?
   *      yes - enable blaster.
   *      reset watchdog
   *    check watchdog timer overflow
   *      overflow?
   *        yes - disable
   *        no - ignore
   *   
   
   */
    
#endif

}

void sendCommand(IPAddress target, char* command)
{
  if (!serverClients[0].connect(target, TCPPORT)) {
    Serial.print("Connection Failed");
    return;
  }
  Serial.print("Connected in sendCommand");
   // create URI
   String url = String(system_get_chip_id(), HEX) + ":" + str(command) + "\r";
   // send request to server
   Serial.println();
   serverClients[0].print(url);
   unsigned timeout = millis();
   while (serverClients[0].available() == 0) {
     if (millis() == timeout > 5000) {
       Serial.println("Client Timeout!");
       serverClients[0].stop();
       return;
     }
   }
   // read all data back
   while (serverClients[0].available()) {
    String received = serverClients[0].readStringUntil('\r');
    Serial.print(received);
    serverClients[0].stop();
   }
}


//+=============================================================================
// Display encoding type
//
void  encoding (decode_results *results)
{
  switch (results->decode_type) {
    default:
    case UNKNOWN:      Serial.print("UNKNOWN");       break ;
    case NEC:          Serial.print("NEC");           break ;
    case SONY:         Serial.print("SONY");          break ;
    case RC5:          Serial.print("RC5");           break ;
    case RC6:          Serial.print("RC6");           break ;
    case DISH:         Serial.print("DISH");          break ;
    case SHARP:        Serial.print("SHARP");         break ;
    case JVC:          Serial.print("JVC");           break ;
    case SANYO:        Serial.print("SANYO");         break ;
    case MITSUBISHI:   Serial.print("MITSUBISHI");    break ;
    case SAMSUNG:      Serial.print("SAMSUNG");       break ;
    case LG:           Serial.print("LG");            break ;
    case WHYNTER:      Serial.print("WHYNTER");       break ;
    case AIWA_RC_T501: Serial.print("AIWA_RC_T501");  break ;
    case PANASONIC:    Serial.print("PANASONIC");     break ;
  }
}


//+=============================================================================
// Dump out the decode_results structure.
//

void  dumpRaw (decode_results *results)
{
  // Print Raw data
  Serial.print("Timing[");
  Serial.print(results->rawlen-1, DEC);
  Serial.println("]: ");

  for (int i = 1;  i < results->rawlen;  i++) {
    unsigned long  x = results->rawbuf[i] * USECPERTICK;
    if (!(i & 1)) {  // even
      Serial.print("-");
      if (x < 1000)  Serial.print(" ") ;
      if (x < 100)   Serial.print(" ") ;
      Serial.print(x, DEC);
    } else {  // odd
      Serial.print("     ");
      Serial.print("+");
      if (x < 1000)  Serial.print(" ") ;
      if (x < 100)   Serial.print(" ") ;
      Serial.print(x, DEC);
      if (i < results->rawlen-1) Serial.print(", "); //',' not needed for last one
    }
    if (!(i % 8))  Serial.println("");
  }
  Serial.println("");                    // Newline
}
void processIR(decode_results *results) {
}

void fire() {

  if (!safety) { // check safety flag == off
    //safety is off - fire here
    // first, send packet to MC
    
    // then, send IR packet
    Serial.println("FIRING");
    
    irtx.sendNEC(0x5a5a, 14);
  }
}

void blast(int timeout) { // function to blow flame
  static uint16_t blasttime;
  static long starttime ;
  if (!safety) { // check safety flag == off
    // safety is off - good to fire
    if (timeout == -1) { // check for maintain in loop
      if ((starttime + blasttime) > millis()) {
        // end fire here
        digitalWrite(SOLENOID, LOW);
      }
    } else {
      // initial call of blast - timeout is not -1
      starttime = millis();
      blasttime = timeout * 1000;
      // start fire here
      digitalWrite(SOLENOID, HIGH);
      
    }
  }
}

