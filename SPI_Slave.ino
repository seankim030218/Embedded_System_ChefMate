#include "HX711.h"
#include <SPI.h>
#define DOUT_PIN  3
#define CLK_PIN   2
#define CALIBRATION_FACTOR -7050.0 
HX711 scale;
volatile int weightBuffer = 0; 
volatile bool tareReq = false;
void setup() {
  Serial.begin(9600);
  scale.begin(DOUT_PIN, CLK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare(); 
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);
  SPI.attachInterrupt();
}
ISR (SPI_STC_vect) {
  byte c = SPDR;
  if (c == 'R') {
    SPDR = highByte(weightBuffer);
  }
  else if (c == 'T') {
    tareReq = true;
  }
  else {
    SPDR = lowByte(weightBuffer);
  }
}
void loop() {
  if (scale.is_ready()) {
    float w = scale.get_units(1);
    if (w < 0) w = 0.0;
    noInterrupts();
    weightBuffer = (int)(w * 10.0);
    interrupts();
  }
  if (tareReq) {
    scale.tare();
    tareReq = false;
  }
}