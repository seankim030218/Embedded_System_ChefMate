#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 2);
const int pinBuzzer = 8;
char receivedChar;
String receivedString = "";
bool started = false;
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(pinBuzzer, OUTPUT);
  digitalWrite(pinBuzzer, LOW);
}
void loop() {
  while (mySerial.available()) {
    receivedChar = mySerial.read();
    if (receivedChar == 'T') {
      receivedString = "";
      started = true;
    } else if (started && receivedChar == '#') {
      started = false;
      if (receivedString.length() > 0) {
        char direction = receivedString.charAt(0);
        if (direction == '1') {
          tone(pinBuzzer, 1000, 500);
        } else {
          noTone(pinBuzzer);
        }
      }
    } else if (started) {
      receivedString += receivedChar;
    }
  }
  delay(50);
}