#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);  
#define UP_BTN     5   
#define DOWN_BTN   6   
#define SELECT_BTN 7   
#define BUZZER_PIN 8
#define SS_PIN     10
const char* menuItems[] = { "1. Bean paste soup", "2. Ramen", "3. Kimchi-pancake", "4. Curry", "5. Tteokbokki", "6. Egg soup", "7. Bulgogi" };
const int menuLength = 7; 
const char* ingredients[] = { "[ Ingredients ]", "- Kimchi: 200g", "- Flour: 150g", "- Kimchi juice: 50g", "- Water: 150g", "- Sugar: 15g", "- Chili pwdr: 10g", "- Cooking oil", "   [ I'm Ready! ]" };
const int ingLength = 9;
int screenState = 0; 
int menuIndex = 0; int menuWindow = 0;
int ingIndex = 0; int ingWindow = 0;
int cookingStep = 1; 
unsigned long timerStart = 0; 
bool isTimerRunning = false;  
void setup() {
  Serial.begin(9600);
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(SELECT_BTN, INPUT_PULLUP); 
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16); 
  lcd.begin(); 
  lcd.backlight(); 
  showIntro();
}
float getWeightFromSlave() {
  digitalWrite(SS_PIN, LOW); 
  SPI.transfer('R'); 
  delayMicroseconds(100); 
  byte high = SPI.transfer(0x00);
  byte low = SPI.transfer(0x00);
  digitalWrite(SS_PIN, HIGH); 
  int val = (high << 8) | low; 
  return val / 10.0; 
}
void sendTareToSlave() {
  digitalWrite(SS_PIN, LOW);
  SPI.transfer('T'); 
  delayMicroseconds(100);
  digitalWrite(SS_PIN, HIGH);
}
void playAlarm() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 2000);
    delay(300);
    noTone(BUZZER_PIN);
    delay(200);
  }
}
void loop() {
  if (screenState == 0) {
    if (digitalRead(SELECT_BTN) == LOW) { screenState = 1; drawMenu(); delay(300); }
  }
  else if (screenState == 1) {
    if (digitalRead(UP_BTN) == LOW) { if (menuIndex > 0) { menuIndex--; if (menuIndex < menuWindow) menuWindow = menuIndex; drawMenu(); } delay(150); }
    if (digitalRead(DOWN_BTN) == LOW) { if (menuIndex < menuLength - 1) { menuIndex++; if (menuIndex >= menuWindow + 4) menuWindow = menuIndex - 3; drawMenu(); } delay(150); }
    if (digitalRead(SELECT_BTN) == LOW) { screenState = 2; showYesNoQuestion(); delay(300); }
  }
  else if (screenState == 2) {
    if (digitalRead(UP_BTN) == LOW) { if (menuIndex == 2) { screenState = 3; ingIndex=0; ingWindow=0; drawIngredients(); } else { screenState = 0; showIntro(); } delay(300); }
    if (digitalRead(DOWN_BTN) == LOW) { screenState = 1; drawMenu(); delay(300); }
  }
  else if (screenState == 3) {
    if (digitalRead(UP_BTN) == LOW) { if (ingIndex > 0) { ingIndex--; if (ingIndex < ingWindow) ingWindow = ingIndex; drawIngredients(); } delay(150); }
    if (digitalRead(DOWN_BTN) == LOW) { if (ingIndex < ingLength - 1) { ingIndex++; if (ingIndex >= ingWindow + 4) ingWindow = ingIndex - 3; drawIngredients(); } delay(150); }
    if (digitalRead(SELECT_BTN) == LOW) { if (ingIndex == ingLength - 1) { screenState = 4; cookingStep = 1; processCookingStep(); } delay(300); }
  }
  else if (screenState == 4) {
    if (cookingStep >= 2 && cookingStep <= 5) {
      float weight = getWeightFromSlave();
      if (weight < 0) weight = 0.0;
      lcd.setCursor(9, 2); lcd.print(F("     ")); lcd.setCursor(9, 2); lcd.print(weight, 1); 
      if (digitalRead(SELECT_BTN) == LOW) {
        sendTareToSlave(); 
        lcd.setCursor(0, 3); lcd.print(F("Zero Set! Next..    "));
        delay(1000);
        cookingStep++;
        processCookingStep();
        delay(300);
      }
      delay(100); 
    }
    else if (isTimerRunning) {
      unsigned long elapsed = (millis() - timerStart) / 1000;
      long remaining = 0;
      if (cookingStep == 7) remaining = 60 - elapsed;
      else if (cookingStep == 8) remaining = 300 - elapsed;
      else if (cookingStep == 9) remaining = 180 - elapsed;
      if (remaining < 0) remaining = 0;
      lcd.setCursor(7, 2); lcd.print(F("   ")); lcd.setCursor(7, 2); lcd.print(remaining);
      if (remaining == 0) { 
        isTimerRunning = false; 
        lcd.setCursor(0, 3); lcd.print(F(" [ Time is Up! ]  ")); 
        playAlarm();
      }
      if (digitalRead(SELECT_BTN) == LOW) { isTimerRunning = false; processCookingStep(); }
    }
    else {
      if (digitalRead(SELECT_BTN) == LOW) {
        if (cookingStep == 10) { screenState = 0; showIntro(); } else { cookingStep++; processCookingStep(); } delay(300);
      }
    }
  }
}
void processCookingStep() {
  lcd.clear(); lcd.noCursor(); lcd.noBlink();
  switch(cookingStep) {
    case 1: lcd.print(F("Step 1/10")); lcd.setCursor(0, 1); lcd.print(F("Put the bowl on")); lcd.setCursor(0, 2); lcd.print(F("the scale.")); showNextBtn(); sendTareToSlave(); break;
    case 2: lcd.print(F("Step 2/10 (Kimchi)")); lcd.setCursor(0, 1); lcd.print(F("Chop & Put in bowl")); showWeightUI(200); break;
    case 3: lcd.print(F("Step 3/10 (Flour)")); lcd.setCursor(0, 1); lcd.print(F("Add Flour")); showWeightUI(150); break;
    case 4: lcd.print(F("Step 4 (Liq)")); lcd.setCursor(0, 1); lcd.print(F("Juice 50g+Water 150g")); showWeightUI(200); break;
    case 5: lcd.print(F("Step 5 (Seasoning)")); lcd.setCursor(0, 1); lcd.print(F("Sugar 15g+Chili 10g")); showWeightUI(25); break;
    case 6: lcd.print(F("Step 6/10")); lcd.setCursor(0, 1); lcd.print(F("Remove bowl from")); lcd.setCursor(0, 2); lcd.print(F("scale & Mix well!")); showNextBtn(); break;
    case 7: lcd.print(F("Step 7 (Preheat)")); lcd.setCursor(0, 1); lcd.print(F("Oil & Heat Pan")); startTimerUI(60); break;
    case 8: lcd.print(F("Step 8 (Cooking)")); lcd.setCursor(0, 1); lcd.print(F("Put dough on pan")); startTimerUI(300); break;
    case 9: lcd.print(F("Step 9 (Flip!)")); lcd.setCursor(0, 1); lcd.print(F("Flip and Cook")); startTimerUI(180); break;
    case 10: lcd.print(F("Step 10/10")); lcd.setCursor(0, 1); lcd.print(F("Enjoy your Meal!")); lcd.setCursor(0, 3); lcd.print(F("   [ Finish ]     ")); break;
  }
}
void showNextBtn() { lcd.setCursor(0, 3); lcd.print(F("      [ Next ]      ")); }
void showWeightUI(int goal) { lcd.setCursor(0, 2); lcd.print(F("Goal:")); lcd.print(goal); lcd.print(F(" Cur:")); lcd.setCursor(0, 3); lcd.print(F("[Tare & Next]")); }
void startTimerUI(int seconds) { timerStart = millis(); isTimerRunning = true; lcd.setCursor(0, 2); lcd.print(F("Timer: ")); lcd.print(seconds); lcd.print(F(" s")); lcd.setCursor(0, 3); lcd.print(F("Wait for finish...")); }
void showIntro() { lcd.clear(); lcd.noCursor(); lcd.noBlink(); lcd.setCursor(0, 0); lcd.print(F("Would you like to")); lcd.setCursor(0, 1); lcd.print(F("start cooking?")); lcd.setCursor(0, 3); lcd.print(F("      [ OK ]      ")); }
void drawMenu() { lcd.clear(); lcd.noCursor(); for (int i = 0; i < 4; i++) { int idx = menuWindow + i; if (idx < menuLength) { lcd.setCursor(0, i); lcd.print(menuItems[idx]); } } lcd.setCursor(0, menuIndex - menuWindow); lcd.cursor(); lcd.blink(); }
void showYesNoQuestion() { lcd.clear(); lcd.noCursor(); lcd.noBlink(); lcd.setCursor(0, 0); lcd.print(F("Do you want to make")); String temp = menuItems[menuIndex]; String foodName = temp.substring(3); lcd.setCursor(0, 1); lcd.print(foodName + "?"); lcd.setCursor(0, 3); lcd.print(F("YES(Btn5)   NO(Btn6)")); }
void drawIngredients() { lcd.clear(); lcd.noCursor(); for (int i = 0; i < 4; i++) { int idx = ingWindow + i; if (idx < ingLength) { lcd.setCursor(0, i); lcd.print(ingredients[idx]); } } lcd.setCursor(0, ingIndex - ingWindow); lcd.cursor(); lcd.blink(); }