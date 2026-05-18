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
<<<<<<< HEAD

void setup() {
  lcd.begin(16, 2);
  
=======

// CSV or Human-readable mode
bool csvMode = false;

// ANSI Colors
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

void setup() {
  lcd.begin(16, 2);

  Serial.begin(9600);
  Serial.println("System Booting...");

>>>>>>> cf1ccf64362cf4573aac6a395b047450ab9bf789
  // LED & Buzzer Setup
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
<<<<<<< HEAD
  
=======

>>>>>>> cf1ccf64362cf4573aac6a395b047450ab9bf789
  // Input Setup
  pinMode(PIR_PIN, INPUT);
  pinMode(CONFIRM_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);
  pinMode(EMERGENCY_BTN, INPUT_PULLUP);

  lcd.print("SYSTEM BOOTING");
  delay(1000);
  lcd.clear();
<<<<<<< HEAD
=======

  Serial.println("System Ready.");
>>>>>>> cf1ccf64362cf4573aac6a395b047450ab9bf789
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
<<<<<<< HEAD
  return duration * 0.034 / 2;
=======

  if (duration == 0) return 999;

  return duration * 0.034 / 2;
}

// ----------------------
// SERIAL LOGGING FUNCTION
// ----------------------
void logData(const char* stateName, bool motion) {
  unsigned long t = millis();

  if (csvMode) {
    // CSV: time,distance,pir,state
    Serial.print(t); Serial.print(",");
    Serial.print(distance); Serial.print(",");
    Serial.print(motion); Serial.print(",");
    Serial.println(stateName);
  } else {
    // Color-coded readable output
    Serial.print(CYAN);
    Serial.print("["); Serial.print(t); Serial.print(" ms] ");
    Serial.print(RESET);

    Serial.print(YELLOW);
    Serial.print("STATE: ");
    Serial.print(stateName);
    Serial.print(" | ");
    Serial.print(RESET);

    Serial.print(GREEN);
    Serial.print("Dist: ");
    Serial.print(distance);
    Serial.print(" cm | ");
    Serial.print(RESET);

    Serial.print(motion ? RED : GREEN);
    Serial.print("PIR: ");
    Serial.println(motion ? "MOTION" : "STILL");
    Serial.print(RESET);
  }
}

// ----------------------
// EMERGENCY MODE
// ----------------------
void runEmergencyMode() {
  digitalWrite(RED_LED, (millis() % 400 < 200));
  tone(BUZZER, 2500);

  if (millis() - lcdTimer > 500) {
    lcd.setCursor(0,0); lcd.print("!! EMERGENCY !! ");
    lcd.setCursor(0,1); lcd.print("SYSTEM LOCKED   ");
    lcd.clear();
    lcdTimer = millis();
  }

  logData("EMERGENCY", false);
}

// ----------------------
// IDLE MODE
// ----------------------
void runIdleMode() {
  digitalWrite(GREEN_LED, (millis() % 2000 < 100));
  noTone(BUZZER);

  if (millis() - lcdTimer > 500) {
    lcd.setCursor(0,0); lcd.print("  SYSTEM READY  ");
    lcd.setCursor(0,1); lcd.print(" PRESS CONFIRM  ");
    lcdTimer = millis();
  }

  logData("IDLE", false);
}

// ----------------------
// MONITORING MODE
// ----------------------
void runMonitoringMode() {
  distance = getDistance();
  bool motion = digitalRead(PIR_PIN);

  // LED Logic
  if (distance < 100) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    tone(BUZZER, 1000);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER);
  }

  if (millis() - lcdTimer > 300) {
    lcd.setCursor(0,0);
    lcd.print("Dist: "); lcd.print((int)distance); lcd.print("cm   ");
    lcd.setCursor(0,1);
    lcd.print("PIR: "); lcd.print(motion ? "MOTION " : "STILL  ");
    lcdTimer = millis();
  }

  logData("MONITORING", motion);
>>>>>>> cf1ccf64362cf4573aac6a395b047450ab9bf789
}

// ----------------------
// MAIN LOOP
// ----------------------
void loop() {
<<<<<<< HEAD
  // 1. Check Buttons (LOW = Pressed)
  if (digitalRead(CONFIRM_BTN) == LOW) active = true;
  if (digitalRead(EMERGENCY_BTN) == LOW) emergency = true;
=======
  if (digitalRead(CONFIRM_BTN) == LOW) active = true;
  if (digitalRead(EMERGENCY_BTN) == LOW) emergency = true;

>>>>>>> cf1ccf64362cf4573aac6a395b047450ab9bf789
  if (digitalRead(RESET_BTN) == LOW) {
    active = false;
    emergency = false;
    noTone(BUZZER);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    lcd.clear();
  }

<<<<<<< HEAD
  // 2. Logic Branches
=======
>>>>>>> cf1ccf64362cf4573aac6a395b047450ab9bf789
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
