#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define PIR_PIN 19
#define GREEN_LED 13
#define YELLOW_LED 12
#define RED_LED 14
#define BUZZER 27
#define RESET_BTN 4
#define CONFIRM_BTN 15
#define EMERGENCY_BTN 2

// Simulation Variables
int totalExposures = 0;
int highRiskCount = 0;
int warningCount = 0;
int safeCount = 0;
int riskScore = 0;

unsigned long simulationStartTime = 0;
bool personPresent = false;

float currentDistance = 0;
float previousDistance = 0;

// Debounce
unsigned long lastResetPress = 0;
unsigned long lastConfirmPress = 0;
unsigned long lastEmergencyPress = 0;
const int debounceDelay = 250;

// Long-press emergency
unsigned long emergencyPressStart = 0;
const unsigned long emergencyHoldTime = 1500;

// Interaction State
bool interactionActive = false;
bool emergencyState = false;

// Risk status enum
enum RiskStatus { RISK_SAFE, RISK_WARNING, RISK_HIGH, RISK_ERROR };
RiskStatus status = RISK_SAFE;
RiskStatus lastStatus = RISK_SAFE;

unsigned long zoneDebounceStartTime = 0;
const unsigned long zoneDebounceDelay = 450;

unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 200;

// Forward declarations
void updateDisplay();
void setLEDsAndBuzzer(RiskStatus s);

// ------------------------------------------------------------
// INITIALIZATION
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);  // ESP32 I2C pins

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found!");
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(RESET_BTN, INPUT_PULLUP);
  pinMode(CONFIRM_BTN, INPUT_PULLUP);
  pinMode(EMERGENCY_BTN, INPUT_PULLUP);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 10);
  display.println("DISTANCE RISK");
  display.setCursor(10, 25);
  display.println("ANALYZER v2.5");
  display.setCursor(20, 45);
  display.println("Starting...");
  display.display();

  delay(1500);
  simulationStartTime = millis();
}

// ------------------------------------------------------------
// ULTRASONIC SENSOR (Optimized + Stabilized)
// ------------------------------------------------------------
float getRawDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(12);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;

  float distance = duration * 0.034 / 2;

  // Reject impossible jumps
  if (previousDistance > 0 && fabs(distance - previousDistance) > 80) {
    return previousDistance;
  }

  return distance;
}

// ------------------------------------------------------------
// RESET SYSTEM
// ------------------------------------------------------------
void resetCountersAndState() {
  totalExposures = 0;
  highRiskCount = 0;
  warningCount = 0;
  safeCount = 0;
  riskScore = 0;

  interactionActive = false;
  emergencyState = false;

  simulationStartTime = millis();
  status = RISK_SAFE;
  lastStatus = RISK_SAFE;

  currentDistance = 0;
  previousDistance = 0;
}

void resetSimulation() {
  resetCountersAndState();
  noTone(BUZZER);

  for (int i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    delay(150);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    delay(150);
  }
}

// ------------------------------------------------------------
// EMERGENCY CLEAR
// ------------------------------------------------------------
void clearEmergencyWithAnimation() {
  resetCountersAndState();
  noTone(BUZZER);

  for (int i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 900);
    delay(120);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);
    delay(120);
  }
}

// ------------------------------------------------------------
// OLED DISPLAY (Optimized Layout)
// ------------------------------------------------------------
void updateDisplay() {
  if (millis() - lastDisplayUpdate < displayInterval) return;
  lastDisplayUpdate = millis();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Emergency Screen
  if (emergencyState) {
    display.setCursor(10, 10);
    display.println("!!! EMERGENCY !!!");
    display.setCursor(10, 30);
    display.println("Calling Services...");
    display.setCursor(10, 50);
    display.println("Hold CONFIRM to clear");
    display.display();
    return;
  }

  // Idle Screen
  if (!interactionActive) {
    display.setCursor(20, 20);
    display.println("Press CONFIRM");
    display.setCursor(20, 35);
    display.println("to begin");
    display.display();
    return;
  }

  // Status Bar
  display.setCursor(0, 0);
  display.print("INT:ON PIR:");
  display.print(personPresent ? "1 " : "0 ");
  display.print("EMG:");
  display.print(emergencyState ? "1" : "0");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // Distance
  display.setCursor(0, 12);
  display.print("Dist: ");
  if (currentDistance >= 999) display.print("NO ECHO");
  else {
    display.print((int)currentDistance);
    display.print(" cm");
  }

  // Status
  display.setCursor(0, 24);
  display.print("Status: ");
  if (currentDistance >= 999) display.print("ERROR");
  else if (status == RISK_HIGH) display.print("HIGH");
  else if (status == RISK_WARNING) display.print("WARN");
  else display.print("SAFE");

  // Exposure Stats
  display.setCursor(0, 36);
  display.print("Exp:");
  display.print(totalExposures);
  display.print(" Score:");
  display.print(riskScore);

  display.setCursor(0, 48);
  display.print("H:");
  display.print(highRiskCount);
  display.print(" W:");
  display.print(warningCount);
  display.print(" S:");
  display.print(safeCount);

  // Time
  unsigned long runTime = (millis() - simulationStartTime) / 1000;
  display.setCursor(0, 58);
  display.print(runTime / 60);
  display.print("m ");
  display.print(runTime % 60);
  display.print("s");

  // Distance Bar
  if (currentDistance < 999) {
    int barLength = map((int)currentDistance, 0, 200, 128, 0);
    barLength = constrain(barLength, 0, 128);
    display.fillRect(0, 56, barLength, 8, SSD1306_WHITE);
  }

  display.display();
}

// ------------------------------------------------------------
// LED + BUZZER CONTROL
// ------------------------------------------------------------
void setLEDsAndBuzzer(RiskStatus s) {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);

  switch (s) {
    case RISK_HIGH:
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 2000);
      break;

    case RISK_WARNING:
      digitalWrite(YELLOW_LED, HIGH);
      tone(BUZZER, 1000);
      break;

    case RISK_SAFE:
      digitalWrite(GREEN_LED, HIGH);
      break;

    case RISK_ERROR:
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 300);
      break;
  }
}

// ------------------------------------------------------------
// MAIN LOOP
// ------------------------------------------------------------
void loop() {
  unsigned long now = millis();

  // RESET button
  if (digitalRead(RESET_BTN) == LOW && now - lastResetPress > debounceDelay) {
    resetSimulation();
    lastResetPress = now;
  }

  // CONFIRM button
  if (digitalRead(CONFIRM_BTN) == LOW && now - lastConfirmPress > debounceDelay) {
    if (emergencyState) {
      clearEmergencyWithAnimation();
    } else if (!interactionActive) {
      interactionActive = true;
      simulationStartTime = millis();
    }
    lastConfirmPress = now;
  }

  // EMERGENCY button (long press)
  if (digitalRead(EMERGENCY_BTN) == LOW) {
    if (now - lastEmergencyPress > 50) {
      if (emergencyPressStart == 0) emergencyPressStart = now;
      if (!emergencyState && (now - emergencyPressStart > emergencyHoldTime)) {
        emergencyState = true;
      }
    }
  } else {
    emergencyPressStart = 0;
  }

  // Emergency Mode
  if (emergencyState) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 2500);
    updateDisplay();
    return;
  }

  // Idle Mode
  if (!interactionActive) {
    digitalWrite(GREEN_LED, (now % 1000 < 150));
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);
    updateDisplay();
    return;
  }

  // Normal Operation
  personPresent = digitalRead(PIR_PIN);

  previousDistance = currentDistance;
  float rawDistance = getRawDistance();

  // Smooth filtering
  if (rawDistance >= 999) {
    currentDistance = 999;
  } else {
    if (currentDistance <= 0 || currentDistance >= 999) {
      currentDistance = rawDistance;
    } else {
      currentDistance = (0.7 * currentDistance) + (0.3 * rawDistance);
    }
  }

  // Determine instant status
  RiskStatus instantStatus;
  if (currentDistance >= 999) instantStatus = RISK_ERROR;
  else if (currentDistance < 100) instantStatus = RISK_HIGH;
  else if (currentDistance < 200) instantStatus = RISK_WARNING;
  else instantStatus = RISK_SAFE;

  // Debounce transitions
  if (instantStatus != lastStatus) {
    zoneDebounceStartTime = now;
    lastStatus = instantStatus;
  }

  if (status != lastStatus && (now - zoneDebounceStartTime > zoneDebounceDelay)) {
    status = lastStatus;

    bool personDetected = (currentDistance < 300) || personPresent;

    if (personDetected && status != RISK_ERROR) {
      totalExposures++;

      if (status == RISK_HIGH) {
        highRiskCount++;
        riskScore += 3;
      } else if (status == RISK_WARNING) {
        warningCount++;
        riskScore += 2;
      } else {
        safeCount++;
        riskScore += 1;
      }
    }
  }

  setLEDsAndBuzzer(status);
  updateDisplay();
  delay(40);
}
