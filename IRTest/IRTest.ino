int IRreceivePin = 12;
bool state = 0;
long start;
int received[30];
void setup() {
  Serial.begin(115200);
  Serial.println("Ready");
}
//void loop() {
//  if (digitalRead(pin) == LOW)
//  if (digitalRead(pin) != state) {
//    state = !state;
//    if (state) {
//      start = millis();
//    } else {
//      
//    }
//  }
//}

int irlength() {
//  while (digitalRead(IRreceivePin) == HIGH); // wait for pin to go low
//  start = millis();
//  while (digitalRead(IRreceivePin) == LOW)
//  return (millis() - start);
  return (pulseIn(IRreceivePin, LOW, 1000));
} 
void loop() {
    if (digitalRead(IRreceivePin) == LOW) { // check to see if it's low
     
    while(digitalRead(IRreceivePin) == LOW){}             //wait for it to go high again 
    for(int i = 1; i <= 20; i++) {                        // Repeats several times to make sure the whole signal has been received
      //received[i] = irlength();
      received[i] = pulseIn(IRreceivePin, LOW, 10000);  // pulseIn command waits for a pulse and then records its duration in microseconds.
    }
   
    Serial.print("sensor: ");                            // Prints if it was a head shot or not.
    for(int i = 1; i <= 17; i++) {                        // Repeats several times to make sure the whole signal has been received
      Serial.print(received[i]);  // pulseIn command waits for a pulse and then records its duration in microseconds.
      Serial.print(":");
    }
    Serial.println();
  }
}
