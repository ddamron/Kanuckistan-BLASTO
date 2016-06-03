/* Blasto Control System
   Programmed by Dan Damron for Kanuckistan Camp
   GPL v3
   8 relay outputs 4,5,6,7 on board 1, 8,9,10,11 on board 2.
   Board 2 will need to be slightly modified.

  Wiring as follows:
  Pin 2 <- Skull Switch
  Pin 3 <- Big Red Button
  Pin 4 -> Relay 4 > Target 4 Lights (12V)
  Pin 5 -> Relay 3 > Target 3 Lights (12V)
  Pin 6 -> Relay 2 > Target 2 Lights (12V)
  Pin 7 -> Relay 1 > Target 1 Lights (12V)
  
  Pin 8 -> Relay 8 > Propane Valve (12V)
  Pin 9 -> Relay 7 > Solenoid 3 (24V)
  Pin 10 -> Relay 6 > Solenoid 2 (24V)
  Pin 11 -> Relay 5 > Solenoid 1 (24V)
  Pin 12 -> Big Red Button LED
  Pin 13 -> Skull Eyes (Lit when Fire active)


  TODO:

  Both switches need 1 side GND and other side to pin above
  wire the target lights to relays (12V) (Bottom Board) Relay pins N.O. and C
  Wire the Solenoids to the relays (24V) (TOP Board) Relay pins N.O. and C
  Wire control box - need GND, plus 1 wire for each switch, + 1 wire for each LED.
  Skull Switch - Black = GND, RED = +5V, white = signal to pin 2.
  
*/
#define BLASTOPIN 8
#define SKULLBUTTON 2
#define BIGREDBUTTON 3
#define BIGREDBUTTONLIGHT 12
#define TARGETONTIME 100
#define SKULLLEDS 13
#define BLASTOTIMEOUT 1000



// Variables
int currentTarget = 1; // 1 = hit first target, 2=hit 2nd, 3=hit 3rd target, 4= activate blasto
 
void setup() {
  Serial.begin(38400);
  // put your setup code here, to run once:
  int x;
  for (x = 4; x < 12; x++) {
    pinMode(x, OUTPUT);
    digitalWrite(x, LOW);
  }
  pinMode(BIGREDBUTTON, INPUT_PULLUP); // Big Red Button
  pinMode(SKULLBUTTON, INPUT_PULLUP); // Skull Switch
  pinMode(BIGREDBUTTONLIGHT,OUTPUT); // Big Red Button LED
  pinMode(BIGREDBUTTONLIGHT, HIGH); // turn on big red button LED
  pinMode(SKULLLEDS, OUTPUT);
  digitalWrite(SKULLLEDS, HIGH); // turn on skull eyes
  digitalWrite(BIGREDBUTTONLIGHT, LOW);
  pinMode(A0, INPUT_PULLUP); //Sensor 1 (target)
  pinMode(A1, INPUT_PULLUP); //Sensor 3 (target)
  pinMode(A2, INPUT_PULLUP); //Sensor 3 (target)
  pinMode(A3, INPUT_PULLUP); //Sensor 4 (target for propane)
  Serial.println("Init Complete.");
}



void loop() {
  // put your main code here, to run repeatedly: 
  int x;
  // check for reset big red button
  if (!digitalRead(BIGREDBUTTON)) {
    reset();
    currentTarget = 1;
  }
  // check for skull button
  if (!digitalRead(2)) {
    blastoOverride(); // override and activate BLASTO
  }

  //x = checkTarget(1);
  x = checkTarget(currentTarget); // looking for positive logic
  if (x) {
    //activateTarget1();
    activateTarget(currentTarget);
    currentTarget++;
    if (currentTarget == 5) {
      reset();
    }
  }

}

int checkTarget(int target) {
  
  //int result = 1;
  if (target == 1) return !digitalRead(A0);
  if (target == 2) return !digitalRead(A1);
  if (target == 3) return !digitalRead(A2);
  if (target == 4) return !digitalRead(A3);
// 
//  switch (target) {
//    case 1:
//      result = digitalRead(A0);
//      break;
//    case 2:
//      result = digitalRead(A1);
//      break;
//    case 3:
//      result = digitalRead(A2);
//      break;
//    case 4:
//      result = digitalRead(A3);
//      break;
//    default:
//      break;
//  }
//  if (!result) {
//    Serial.print("Sensor ");
//    Serial.print(target);
//    Serial.println(" Active");
//  }
//  return !result;
}
void activateTarget1() {
  Serial.println("Test activating target 1");
  digitalWrite(7, HIGH); // turn on light
  digitalWrite(11, HIGH); // activate solenoid
  delay(100);
  digitalWrite(11, LOW); // deactivate solenoid
}

void activateTarget(int target) {
  Serial.print("Activate Target ");
  Serial.println("target");
  if (target < 4) {
    digitalWrite(8 - target, HIGH); // turn on Light
    digitalWrite(8 - target+4, HIGH); // activate solenoid
    delay(100); // delay for solenoid
    digitalWrite(8 - target+4, LOW); // deactivate Solenoid
  } else {
    digitalWrite(8 - target, HIGH);
    digitalWrite(8 - target+4, HIGH);
    delay(BLASTOTIMEOUT);
    digitalWrite(8 - target+4, LOW);
  }
}

void blastoOverride() {
  Serial.println("Blaster Override!");
  digitalWrite(BLASTOPIN, HIGH);
  delay(BLASTOTIMEOUT);
  digitalWrite(BLASTOPIN, LOW);
}
void reset() {
  Serial.println("Reset!");
  int x;
  for (x = 4; x < 8; x++) {
    digitalWrite(x, LOW);
    currentTarget = 1;
  }
  
}

void testloop() {
  int x;
  for (x = 0; x < 8; x++) {
    digitalWrite(x+4, HIGH);
    Serial.println(x+4);
    delay(500);
    digitalWrite(x+4, LOW);
    delay(500);
  }
}
