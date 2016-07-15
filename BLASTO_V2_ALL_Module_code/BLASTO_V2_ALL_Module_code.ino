#include <IRremoteESP8266.h>
#include <FastLED.h>

#define TARGET 1
//#define GUN 1
//#define BLASTER 1

// hardware pins
#define IRTXPIN 5
#define IRRXPIN 4
#define SOLENOID 13
#define ON 1
#define OFF 0

// LEDS
#ifdef TARGET
  #define LED_PIN 12
  #define NUM_LEDS 24
  #define BRIGHTNESS 64
  #define LED_TYPE WS2811
  #define COLOR_ORDER GRB
  CRGB leds[NUM_LEDS];
#endif

#ifdef GUN
  #define TRIGGER 12
#endif
IRsend irtx(IRTXPIN);
IRrecv irrx(IRRXPIN);

// Gun definitions here..
uint8_t safety = ON;
int timeron = 0;
unsigned long timerstart;
unsigned long timerlength;
CRGB fgcolor, bgcolor;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SOLENOID, OUTPUT);
  #ifdef GUN
    irtx.begin();
    irrx.enableIRIn(); // enable receive on IR
  #endif
  #ifdef TARGET
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    irrx.enableIRIn();
  #endif
  #ifdef BLASTER
  #endif
}
void color_wipe(int fgpercent) {
  //Serial.println("Inside Color_wipe");
  Serial.print("Percent = ");
  Serial.println(fgpercent);
  int i;
  int fgleds = (fgpercent * NUM_LEDS) / 100; // calculation of # of leds using fgcolor
  int blackleds = NUM_LEDS - fgleds;  // calculation of # of leds using bgcolor
  Serial.print("FGleds:"); Serial.print(fgleds);
  Serial.print("\tbgleds:"); Serial.println(blackleds);
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
    Serial.print("Timer Length:");Serial.print(timerlength);
    Serial.print("\tElapsed Time:");Serial.print(elapsedtime);
    int percent = (timerlength - elapsedtime)* 100 / timerlength;
    Serial.print("Update_timer Percent:");Serial.println(percent);
    color_wipe(percent);
  }
//Serial.println("End of update_timer");
}


void loop() {
  Serial.println("Begin of Loop");
  decode_results results;
  if (irrx.decode(&results)) {
    Serial.print("IR IN:");
    dumpRaw(&results);
    irrx.resume();
  }
  if (timeron) {
    Serial.print("+");
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

