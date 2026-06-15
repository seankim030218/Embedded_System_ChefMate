#include <SoftwareSerial.h>

#define TILT_PIN 7
#define SS_RX 10
#define SS_TX 8

SoftwareSerial mySerial(SS_RX, SS_TX);

void setup() {
  pinMode(TILT_PIN, INPUT_PULLUP);
  mySerial.begin(9600);
}

void loop() {
  int tiltState = digitalRead(TILT_PIN);
  if (tiltState == LOW) {
    mySerial.print("T1#");
  } else {
    mySerial.print("T0#");
  }
  delay(200);
}
