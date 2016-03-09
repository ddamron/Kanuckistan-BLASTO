template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }


const PROGMEM unsigned char test[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);

}

void loop() {
  // put your main code here, to run repeatedly:
  int x;
  for (x = 0; x < 11; x++) {
    Serial << x << ":" << pgm_read_byte(test + x) << "\t";
  }
  Serial << "\n";
  delay(2000);
}
