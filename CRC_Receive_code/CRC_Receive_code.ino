//      Start of code (copy and paste into arduino sketch)
//
//                       Duino Tag release V1.01
//          Laser Tag for the arduino based on the Miles Tag Protocol.
//           By J44industries:     www.J44industries.blogspot.com
// For information on building your own Duino Tagger go to: http://www.instructables.com/member/j44/
//
// Much credit deserves to go to Duane O'Brien if it had not been for the excellent Duino Tag tutorials he wrote I would have never been able to write this code.
// Duane's tutorials are highly recommended reading in order to gain a better understanding of the arduino and IR communication. See his site http://aterribleidea.com/duino-tag-resources/
//
// This code sets out the basics for arduino based laser tag system and tries to stick to the miles tag protocol where possible.
// Miles Tag details: http://www.lasertagparts.com/mtdesign.htm
// There is much scope for expanding the capabilities of this system, and hopefully the game will continue to evolve for some time to come.
// Licence: Attribution Share Alike: Give credit where credit is due, but you can do what you like with the code.
// If you have code improvements or additions please go to http://duinotag.blogspot.com
//



// Digital IO's
int triggerPin             = 3;      // Push button for primary fire. Low = pressed
int IRtransmitPin          = 2;      // Primary fire mode IR transmitter pin: Use pins 2,4,7,8,12 or 13. DO NOT USE PWM pins!! More info: http://j44industries.blogspot.com/2009/09/arduino-frequency-generation.html#more
int IRreceivePin           = 12;     // The pin that incoming IR signals are read from
// Minimum gun requirements: trigger, receiver, IR led, hit LED.

// Player and Game details
int myTeamID               = 1;      // 1-7 (0 = system message)
int myPlayerID             = 5;      // Player ID
int myGameID               = 0;      // Interprited by configureGane subroutine; allows for quick change of game types.
int myWeaponID             = 0;      // Deffined by gameType and configureGame subroutine.
int myWeaponHP             = 0;      // Deffined by gameType and configureGame subroutine.
int maxAmmo                = 0;      // Deffined by gameType and configureGame subroutine.
int maxLife                = 0;      // Deffined by gameType and configureGame subroutine.
int automatic              = 0;      // Deffined by gameType and configureGame subroutine. Automatic fire 0 = Semi Auto, 1 = Fully Auto.
int automatic2             = 0;      // Deffined by gameType and configureGame subroutine. Secondary fire auto?

//Incoming signal Details
int received[18];                    // Received data: received[0] = which sensor, received[1] - [17] byte1 byte2 parity (Miles Tag structure)
int check                  = 0;      // Variable used in parity checking

// Stats
int ammo                   = 0;      // Current ammunition
int life                   = 0;      // Current life

// Code Variables
int timeOut                = 0;      // Deffined in frequencyCalculations (IRpulse + 50)
int FIRE                   = 1;      // 0 = don't fire, 1 = Primary Fire, 2 = Secondary Fire
int TR                     = 0;      // Trigger Reading
int LTR                    = 0;      // Last Trigger Reading
int T2R                    = 0;      // Trigger 2 Reading (For secondary fire)
int LT2R                   = 0;      // Last Trigger 2 Reading (For secondary fire)

// Signal Properties
int IRpulse                = 600;    // Basic pulse duration of 600uS MilesTag standard 4*IRpulse for header bit, 2*IRpulse for 1, 1*IRpulse for 0.
int IRfrequency            = 56;     // Frequency in kHz Standard values are: 38kHz, 40kHz. Choose dependant on your receiver characteristics
int IRt                    = 0;      // LED on time to give correct transmission frequency, calculated in setup.
int IRpulses               = 0;      // Number of oscillations needed to make a full IRpulse, calculated in setup.
int header                 = 4;      // Header lenght in pulses. 4 = Miles tag standard
int maxSPS                 = 10;     // Maximum Shots Per Seconds. Not yet used.
int TBS                    = 0;      // Time between shots. Not yet used.

// Transmission data
int byte1[8];                        // String for storing byte1 of the data which gets transmitted when the player fires.
int byte2[8];                        // String for storing byte1 of the data which gets transmitted when the player fires.
int myParity               = 0;      // String for storing parity of the data which gets transmitted when the player fires.

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
  // Serial coms set up to help with debugging.
  Serial.begin(115200);              
  Serial.println("Startup...");
  // Pin declarations
  pinMode(triggerPin, INPUT);
  pinMode(IRtransmitPin, OUTPUT);
  pinMode(IRreceivePin, INPUT_PULLUP);
 
  frequencyCalculations();   // Calculates pulse lengths etc for desired frequency
  configureGame();           // Look up and configure game details
  tagCode();                 // Based on game details etc works out the data that will be transmitted when a shot is fired
 
 
  digitalWrite(triggerPin, HIGH);      // Not really needed if your circuit has the correct pull up resistors already but doesn't harm
  
  Serial.println("Ready....");
}


// Main loop most of the code is in the sub routines
void loop(){
  //Serial.print(".");
  receiveIR();
  //triggers();
}


// SUB ROUTINES




void receiveIR() { // Void checks for an incoming signal and decodes it if it sees one.
  int error = 0;
 
   
  while(digitalRead(IRreceivePin) == LOW){
  
    for(int i = 1; i <= 17; i++) {                        // Repeats several times to make sure the whole signal has been received
      received[i] = pulseIn(IRreceivePin, LOW, timeOut);  // pulseIn command waits for a pulse and then records its duration in microseconds.
    }
   
    Serial.print("sensor: ");                            // Prints if it was a head shot or not.
    Serial.print(received[0]); 
    Serial.print("...");
   
    for(int i = 1; i <= 17; i++) {  // Looks at each one of the received pulses
      int receivedTemp[18];
      receivedTemp[i] = 2;
      if(received[i] > (IRpulse - 200) &&  received[i] < (IRpulse + 200)) {receivedTemp[i] = 0;}                      // Works out from the pulse length if it was a data 1 or 0 that was received writes result to receivedTemp string
      if(received[i] > (IRpulse + IRpulse - 200) &&  received[i] < (IRpulse + IRpulse + 200)) {receivedTemp[i] = 1;}  // Works out from the pulse length if it was a data 1 or 0 that was received  
      received[i] = 3;                   // Wipes raw received data
      received[i] = receivedTemp[i];     // Inputs interpreted data
     
      Serial.print(" ");
      Serial.print(received[i]);         // Print interpreted data results
    }
    Serial.println("");                  // New line to tidy up printed results
   
    // Parity Check. Was the data received a valid signal?
    check = 0;
    for(int i = 1; i <= 16; i++) {
      if(received[i] == 1){check = check + 1;}
      if(received[i] == 2){error = 1;}
    }
    // Serial.println(check);
    check = check >> 0 & B1;
    // Serial.println(check);
    if(check != received[17]){error = 1;}
    if(error == 0){Serial.println("Valid Signal");}
    else{Serial.println("ERROR");}
    if(error == 0){interpritReceived();}
    //digitalWrite(hitPin,LOW);
  }

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



void sendPulse(int pin, int length){ // importing variables like this allows for secondary fire modes etc.
// This void genertates the carrier frequency for the information to be transmitted
  int i = 0;
  int o = 0;
  while( i < length ){
    i++;
    while( o < IRpulses ){
      o++;
      digitalWrite(pin, HIGH);
      delayMicroseconds(IRt);
      digitalWrite(pin, LOW);
      delayMicroseconds(IRt);
    }
  }
}


void triggers() { // Checks to see if the triggers have been presses
  //Serial.print("Triggers");
  LTR = TR;       // Records previous state. Primary fire
  TR = digitalRead(triggerPin);      // Looks up current trigger button state
  // Code looks for changes in trigger state to give it a semi automatic shooting behaviour
  if(TR != LTR && TR == LOW){
    FIRE = 1;
  }
  if(TR == LOW && automatic == 1){
    FIRE = 1;
  }
  if (FIRE != 0) {
    Serial.print("FIRE==");
    Serial.println(FIRE);
  }
}


void configureGame() { // Where the game characteristics are stored, allows several game types to be recorded and you only have to change one variable (myGameID) to pick the game.
  if(myGameID == 0){
    myWeaponID = 1;
    maxAmmo = 30;
    ammo = 30;
    maxLife = 3;
    life = 3;
    myWeaponHP = 1;
  }
  if(myGameID == 1){
    myWeaponID = 1;
    maxAmmo = 100;
    ammo = 100;
    maxLife = 10;
    life = 10;
    myWeaponHP = 2;
  }
}


void frequencyCalculations() { // Works out all the timings needed to give the correct carrier frequency for the IR signal
  IRt = (int) (500/IRfrequency);  
  IRpulses = (int) (IRpulse / (2*IRt));
  IRt = IRt - 4;
  // Why -4 I hear you cry. It allows for the time taken for commands to be executed.
  // More info: http://j44industries.blogspot.com/2009/09/arduino-frequency-generation.html#more

  Serial.print("Oscilation time period /2: ");
  Serial.println(IRt);
  Serial.print("Pulses: ");
  Serial.println(IRpulses);
  timeOut = IRpulse + 50; // Adding 50 to the expected pulse time gives a little margin for error on the pulse read time out value
}


void tagCode() { // Works out what the players tagger code (the code that is transmitted when they shoot) is
  byte1[0] = myTeamID >> 2 & B1;
  byte1[1] = myTeamID >> 1 & B1;
  byte1[2] = myTeamID >> 0 & B1;

  byte1[3] = myPlayerID >> 4 & B1;
  byte1[4] = myPlayerID >> 3 & B1;
  byte1[5] = myPlayerID >> 2 & B1;
  byte1[6] = myPlayerID >> 1 & B1;
  byte1[7] = myPlayerID >> 0 & B1;


  byte2[0] = myWeaponID >> 2 & B1;
  byte2[1] = myWeaponID >> 1 & B1;
  byte2[2] = myWeaponID >> 0 & B1;

  byte2[3] = myWeaponHP >> 4 & B1;
  byte2[4] = myWeaponHP >> 3 & B1;
  byte2[5] = myWeaponHP >> 2 & B1;
  byte2[6] = myWeaponHP >> 1 & B1;
  byte2[7] = myWeaponHP >> 0 & B1;

  myParity = 0;
  for (int i=0; i<8; i++) {
   if(byte1[i] == 1){myParity = myParity + 1;}
   if(byte2[i] == 1){myParity = myParity + 1;}
   myParity = myParity >> 0 & B1;
  }

  // Next few lines print the full tagger code.
  Serial.print("Byte1: ");
  Serial.print(byte1[0]);
  Serial.print(byte1[1]);
  Serial.print(byte1[2]);
  Serial.print(byte1[3]);
  Serial.print(byte1[4]);
  Serial.print(byte1[5]);
  Serial.print(byte1[6]);
  Serial.print(byte1[7]);
  Serial.println();

  Serial.print("Byte2: ");
  Serial.print(byte2[0]);
  Serial.print(byte2[1]);
  Serial.print(byte2[2]);
  Serial.print(byte2[3]);
  Serial.print(byte2[4]);
  Serial.print(byte2[5]);
  Serial.print(byte2[6]);
  Serial.print(byte2[7]);
  Serial.println();

  Serial.print("Parity: ");
  Serial.print(myParity);
  Serial.println();
}









