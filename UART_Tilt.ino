// ====================================================
// Uno1 - 기울기 감지 -> UART 송신
// 프로토콜: 'T' + 상태('1'=기울어짐 / '0'=정상) + '#'
// 수신측: Uno2 (UART_Buzzer.ino, SoftwareSerial RX=D4)
// ====================================================

#include <SoftwareSerial.h>

#define TILT_PIN 7   // 기울기 센서 입력 (INPUT_PULLUP)
#define SS_RX    10  // 사용 안 함 (단방향 송신)
#define SS_TX    8   // Uno2 D4(RX)에 연결

SoftwareSerial mySerial(SS_RX, SS_TX);

int lastSentState = -1;  // 직전에 전송한 상태 (디바운스용)

void setup() {
  pinMode(TILT_PIN, INPUT_PULLUP);
  mySerial.begin(9600);
}

void loop() {
  int tiltState = digitalRead(TILT_PIN);

  // 상태가 바뀌었을 때만 전송 (채터링 방지)
  if (tiltState != lastSentState) {
    if (tiltState == LOW) {
      mySerial.print("T1#");  // 기울어짐
    } else {
      mySerial.print("T0#");  // 정상
    }
    lastSentState = tiltState;
  }

  delay(50);
}
