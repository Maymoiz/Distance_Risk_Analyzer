#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Pin Definitions
#define ULTRASONIC_SIG 13
#define PIR_PIN 8
#define RED_LED A0
#define YELLOW_LED A4
#define GREEN_LED A5
#define BUZZER 9

#define CONFIRM_BTN A1
#define RESET_BTN A2
#define EMERGENCY_BTN A3

// System State Variables
bool active = false;
bool emergency = false;
float distance = 0;
unsigned long lcdTimer = 0;

void setup() {
  lcd.begin(16, 2);
  
  // LED & Buzzer Setup
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  // Input Setup
  pinMode(PIR_PIN, INPUT);
  pinMode(CONFIRM_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);
  pinMode(EMERGENCY_BTN, INPUT_PULLUP);

  lcd.print("SYSTEM BOOTING");
  delay(1000);
  lcd.clear();
}

float getDistance() {
  pinMode(ULTRASONIC_SIG, OUTPUT);
  digitalWrite(ULTRASONIC_SIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_SIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(ULTRASONIC_SIG, LOW);

  pinMode(ULTRASONIC_SIG, INPUT);
  long duration = pulseIn(ULTRASONIC_SIG, HIGH);
  return duration * 0.034 / 2;
}

void loop() {
  // 1. Check Buttons (LOW = Pressed)
  if (digitalRead(CONFIRM_BTN) == LOW) active = true;
  if (digitalRead(EMERGENCY_BTN) == LOW) emergency = true;
  if (digitalRead(RESET_BTN) == LOW) {
    active = false;
    emergency = false;
    noTone(BUZZER);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    lcd.clear();
  }

  // 2. Logic Branches
  if (emergency) {
    runEmergencyMode();
  } else if (!active) {
    runIdleMode();
  } else {
    runMonitoringMode();
  }
}

void runEmergencyMode() {
  digitalWrite(RED_LED, (millis() % 400 < 200));
  tone(BUZZER, 2500);
  if (millis() - lcdTimer > 500) {
    lcd.setCursor(0,0); lcd.print("!! EMERGENCY !! ");
    lcd.setCursor(0,1); lcd.print("SYSTEM LOCKED   ");
    lcd.clear(); // Prevents gibberish buildup
    lcdTimer = millis();
  }
}

void runIdleMode() {
  digitalWrite(GREEN_LED, (millis() % 2000 < 100));
  noTone(BUZZER);
  if (millis() - lcdTimer > 500) {
    lcd.setCursor(0,0); lcd.print("  SYSTEM READY  ");
    lcd.setCursor(0,1); lcd.print(" PRESS CONFIRM  ");
    lcdTimer = millis();
  }
}

void runMonitoringMode() {
  distance = getDistance();
  bool motion = digitalRead(PIR_PIN);

  // LED Logic
  if (distance < 100) {
    digitalWrite(RED_LED, HIGH); digitalWrite(GREEN_LED, LOW);
    tone(BUZZER, 1000);
  } else {
    digitalWrite(RED_LED, LOW); digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER);
  }

  if (millis() - lcdTimer > 300) {
    lcd.setCursor(0,0);
    lcd.print("Dist: "); lcd.print((int)distance); lcd.print("cm   ");
    lcd.setCursor(0,1);
    lcd.print("PIR: "); lcd.print(motion ? "MOTION " : "STILL  ");
    lcdTimer = millis();
  }
}
