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
int received[18];
// Signal Properties
int IRpulse                = 600;    // Basic pulse duration of 600uS MilesTag standard 4*IRpulse for header bit, 2*IRpulse for 1, 1*IRpulse for 0.
int IRfrequency            = 56;     // Frequency in kHz Standard values are: 38kHz, 40kHz. Choose dependant on your receiver characteristics
int IRt                    = 0;      // LED on time to give correct transmission frequency, calculated in setup.
int IRpulses               = 0;      // Number of oscillations needed to make a full IRpulse, calculated in setup.
int header                 = 4;      // Header lenght in pulses. 4 = Miles tag standard

int check                  = 0;      // Variable used in parity checking
int timeOut                = 0;      // Defined in frequencyCalculations (IRpulse + 50)

// Received data
int memory                 = 10;     // Number of signals to be recorded: Allows for the game data to be reviewed after the game, no provision for transmitting / accessing it yet though.
int hitNo                  = 0;      // Hit number
// Byte1
int player[10];                      // Array must be as large as memory
int team[10];                        // Array must be as large as memory
// Byte2
int weapon[10];                      // Array must be as large as memory
int hp[10];                          // Array must be as large as memory
int parity[10];                      // Array must be as large as memory


void setup() {
  Serial.begin(115200);
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
  frequencyCalculations();
  Serial.println("Init Complete.");
}



void loop() {
  // put your main code here, to run repeatedly: 
  int x;
  int targ;
  // check for reset big red button
  if (!digitalRead(BIGREDBUTTON)) {
    reset();
    currentTarget = 1;
  }
  // check for skull button
  if (!digitalRead(2)) {
    blastoOverride(); // override and activate BLASTO
  }

  if (currentTarget == 1) targ = A0;
  if (currentTarget == 2) targ = A1;
  if (currentTarget == 3) targ = A2;
  if (currentTarget == 4) targ = A3;
 
  x = receiveIR(targ); // looking for positive logic
  if (x) {
    activateTarget(currentTarget);
    currentTarget++;
    if (currentTarget == 5) {
      reset();
    }
  }

}

//int checkTarget(int target) {
//  
//  //int result = 1;
//  if (target == 1) return !digitalRead(A0);
//  if (target == 2) return !digitalRead(A1);
//  if (target == 3) return !digitalRead(A2);
//  if (target == 4) return !digitalRead(A3);
//
//}
//void activateTarget1() {
//  Serial.println("Test activating target 1");
//  digitalWrite(7, HIGH); // turn on light
//  digitalWrite(11, HIGH); // activate solenoid
//  delay(100);
//  digitalWrite(11, LOW); // deactivate solenoid
//}

void activateTarget(int target) {
  Serial.print("Activate Target ");
  Serial.println("target");
  if (target < 4) {
    digitalWrite(8 - target, HIGH); // turn on Light
    digitalWrite(8 - target+4, HIGH); // activate solenoid
    delay(500); // delay for solenoid
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

//void testloop() {
//  int x;
//  for (x = 0; x < 8; x++) {
//    digitalWrite(x+4, HIGH);
//    Serial.println(x+4);
//    delay(500);
//    digitalWrite(x+4, LOW);
//    delay(500);
//  }
//}

int receiveIR(int pin) { // Void checks for an incoming signal and decodes it if it sees one.
  int error = 0;
  int result = 0;
  if (digitalRead(pin) == LOW) { // check to see if it's low
     
    while(digitalRead(pin) == LOW)             //wait for it to go high again 
    for(int i = 1; i <= 15; i++) {                        // Repeats several times to make sure the whole signal has been received
      received[i] = pulseIn(pin, LOW, 5000);  // pulseIn command waits for a pulse and then records its duration in microseconds.
    }
   
    Serial.print("raw: ");                            // Prints if it was a head shot or not.
    for(int i = 1; i <= 15; i++) {                        // Repeats several times to make sure the whole signal has been received
      Serial.print(received[i]);  // pulseIn command waits for a pulse and then records its duration in microseconds.
      Serial.print(":");
    }
    Serial.println();
  
   
    for(int i = 1; i <= 15; i++) {  // Looks at each one of the received pulses
      int receivedTemp[18];
      receivedTemp[i] = 2;
      //Serial.print(received[i]); Serial.print("==");Serial.println(IRpulse);
      if(received[i] > (IRpulse - 200) &&  received[i] < (IRpulse + 200)) {receivedTemp[i] = 0;}                      // Works out from the pulse length if it was a data 1 or 0 that was received writes result to receivedTemp string
      if(received[i] > (IRpulse + IRpulse - 200) &&  received[i] < (IRpulse + IRpulse + 200)) {receivedTemp[i] = 1;}  // Works out from the pulse length if it was a data 1 or 0 that was received  
      //received[i] = 3;                   // Wipes raw received data
      received[i] = receivedTemp[i];     // Inputs interpreted data
     
      Serial.print(" ");
      //Serial.print(received[i]);         // Print interpreted data results
    }
    Serial.println("");                  // New line to tidy up printed results
    
    // Parity Check. Was the data received a valid signal?
    check = 0;
    for(int i = 1; i <= 15; i++) {
      if(received[i] == 1){check = check + 1;}
      if(received[i] == 2){error = 1;}
    }
    // Serial.println(check);
    check = check >> 0 & B1;
    // Serial.println(check);
    if(check != received[15]){error = 1;}
    if(error == 0){
      Serial.println("Valid Signal");
      result = 1;}
    else{Serial.println("ERROR");}
    if(error == 0){interpritReceived();}
    //digitalWrite(hitPin,LOW);
  }
  return result;
}


void interpritReceived(){  // After a message has been received by the ReceiveIR subroutine this subroutine decidedes how it should react to the data
  if(hitNo == memory){hitNo = 0;} // hitNo sorts out where the data should be stored if statement means old data gets overwritten if too much is received
  team[hitNo] = 0;
  player[hitNo] = 0;
  weapon[hitNo] = 0;
  hp[hitNo] = 0;
  // Next few lines Effectivly converts the binary data into decimal
  // Im sure there must be a much more efficient way of doing this
  if(received[1] == 1){team[hitNo] = team[hitNo] + 4;}
  if(received[2] == 1){team[hitNo] = team[hitNo] + 2;}
  if(received[3] == 1){team[hitNo] = team[hitNo] + 1;} 

  if(received[4] == 1){player[hitNo] = player[hitNo] + 16;}
  if(received[5] == 1){player[hitNo] = player[hitNo] + 8;}
  if(received[6] == 1){player[hitNo] = player[hitNo] + 4;}
  if(received[7] == 1){player[hitNo] = player[hitNo] + 2;}
  if(received[8] == 1){player[hitNo] = player[hitNo] + 1;}
   
  if(received[9] == 1){weapon[hitNo] = weapon[hitNo] + 4;}
  if(received[10] == 1){weapon[hitNo] = weapon[hitNo] + 2;}
  if(received[11] == 1){weapon[hitNo] = weapon[hitNo] + 1;} 

  if(received[12] == 1){hp[hitNo] = hp[hitNo] + 16;}
  if(received[13] == 1){hp[hitNo] = hp[hitNo] + 8;}
  if(received[14] == 1){hp[hitNo] = hp[hitNo] + 4;}
  if(received[15] == 1){hp[hitNo] = hp[hitNo] + 2;}
  if(received[16] == 1){hp[hitNo] = hp[hitNo] + 1;}
   
  parity[hitNo] = received[17];

  Serial.print("Hit No: ");
  Serial.print(hitNo);
  Serial.print("  Player: ");
  Serial.print(player[hitNo]);
  Serial.print("  Team: ");
  Serial.print(team[hitNo]);
  Serial.print("  Weapon: ");
  Serial.print(weapon[hitNo]);
  Serial.print("  HP: ");
  Serial.print(hp[hitNo]);
  Serial.print("  Parity: ");
  Serial.println(parity[hitNo]);
 
 
//  //This is probably where more code should be added to expand the game capabilities at the moment the code just checks that the received data was not a system message and deducts a life if it wasn't.
//  if (player[hitNo] != 0){hit();}
//  hitNo++ ;
}

void frequencyCalculations() { // Works out all the timings needed to give the correct carrier frequency for the IR signal
  IRt = (int) (500/IRfrequency);  
  IRpulses = (int) (IRpulse / (2*IRt));
  IRt = IRt - 4;
  // Why -4 I hear you cry. It allows for the time taken for commands to be executed.
  // More info: http://j44industries.blogspot.com/2009/09/arduino-frequency-generation.html#more

  Serial.print("Oscilation time period /2: ");
  Serial.println(IRt);
  Serial.print("IRPulses: ");
  Serial.println(IRpulses);
  timeOut = IRpulse + 100; // Adding 50 to the expected pulse time gives a little margin for error on the pulse read time out value
}

