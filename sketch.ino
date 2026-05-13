#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
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

// Debounce (separate per button)
unsigned long lastResetPress = 0;
unsigned long lastConfirmPress = 0;
unsigned long lastEmergencyPress = 0;
const int debounceDelay = 300;

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
const unsigned long zoneDebounceDelay = 500; // Time in ms required to confirm a new zone

unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 250; // Throttle screen refreshes to prevent flickering

void updateDisplay();
void setLEDsAndBuzzer(RiskStatus s);

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
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
  display.setCursor(10, 10);
  display.println("DISTANCE RISK");
  display.setCursor(5, 25);
  display.println("ANALYZER v2.0");
  display.setCursor(15, 45);
  display.println("Starting...");
  display.display();
  delay(2000);
  simulationStartTime = millis();
}

float getRawDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;
  
  float distance = duration * 0.034 / 2;
  if (previousDistance > 0 && fabs(distance - previousDistance) > 100) {
    return previousDistance;
  }
  return distance;
}

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
  zoneDebounceStartTime = millis();
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
    delay(200);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    delay(200);
  }
}

void clearEmergencyWithAnimation() {
  resetCountersAndState();
  noTone(BUZZER);
  for (int i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 800);
    delay(150);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);
    delay(150);
  }
}

void updateDisplay() {
  if (millis() - lastDisplayUpdate < displayInterval) return; // Prevent heavy flickering
  lastDisplayUpdate = millis();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (emergencyState) {
    display.setCursor(10, 5);
    display.println("!!! EMERGENCY !!!");
    display.setCursor(10, 20);
    display.println("Calling Services...");
    display.setCursor(10, 40);
    display.println("Press CONFIRM to");
    display.setCursor(10, 50);
    display.println("clear emergency");
    display.display();
    return;
  }

  if (!interactionActive) {
    display.setCursor(15, 15);
    display.println("Press CONFIRM to");
    display.setCursor(15, 30);
    display.println("begin interaction");
    display.setCursor(15, 45);
    display.println("Idle...");
    display.display();
    return;
  }

  // Status bar
  display.setCursor(0, 0);
  display.print("INT:ON ");
  display.print("PIR:");
  display.print(personPresent ? "1 " : "0 ");
  display.print("EMG:");
  display.print(emergencyState ? "1" : "0");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // Distance display
  display.setCursor(0, 12);
  display.print("Dist: ");
  if (currentDistance >= 999) display.print("NO ECHO");
  else {
    display.print((int)currentDistance);
    display.print(" cm");
  }

  // Status text
  display.setCursor(0, 22);
  display.print("Status: ");
  if (currentDistance >= 999) display.print("ERROR");
  else if (status == RISK_HIGH) display.print("HIGH RISK");
  else if (status == RISK_WARNING) display.print("WARNING");
  else display.print("SAFE");

  // Exposures and risk score
  display.setCursor(0, 34);
  display.print("Exp: "); display.print(totalExposures);
  display.print(" Score: "); display.print(riskScore);
  
  display.setCursor(0, 44);
  display.print("H:"); display.print(highRiskCount);
  display.print(" W:"); display.print(warningCount);
  display.print(" S:"); display.print(safeCount);

  // Time shifted up slightly to make room for progress bar
  unsigned long runTime = (millis() - simulationStartTime) / 1000;
  display.setCursor(0, 54);
  display.print("Time ");
  display.print(runTime / 60); display.print("m ");
  display.print(runTime % 60); display.print("s");

  // Distance bar shifted down to avoid overwriting Text
  if (currentDistance < 999) {
    int barLength = map((int)currentDistance, 0, 200, 128, 0);
    if (barLength < 0) barLength = 0;
    if (barLength > 128) barLength = 128;
    display.fillRect(0, 62, barLength, 2, SSD1306_WHITE);
  }
  display.display();
}

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

void loop() {
  unsigned long now = millis();

  // RESET button (short press)
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
    if (emergencyPressStart == 0) {
      emergencyPressStart = now;
    } else if (!emergencyState && (now - emergencyPressStart > emergencyHoldTime)) {
      emergencyState = true;
      lastEmergencyPress = now;
    }
  } else {
    emergencyPressStart = 0;
  }

  // Emergency mode handling
  if (emergencyState) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 2500);
    updateDisplay();
    return;
  }

  // Idle state handling
  if (!interactionActive) {
    if ((now % 1000) < 120) {
      digitalWrite(GREEN_LED, HIGH);
    } else {
      digitalWrite(GREEN_LED, LOW);
    }
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);
    updateDisplay();
    return;
  }

  // Normal operation
  personPresent = digitalRead(PIR_PIN);
  previousDistance = currentDistance;
  float rawDistance = getRawDistance();

  // Smooth distance filtering
  if (rawDistance >= 999) {
    currentDistance = 999;
  } else {
    if (currentDistance <= 0 || currentDistance >= 999) {
      currentDistance = rawDistance;
    } else {
      currentDistance = (0.7 * currentDistance) + (0.3 * rawDistance);
    }
  }

  // Evaluate instant reading zone
  RiskStatus instantStatus;
  if (currentDistance >= 999) {
    instantStatus = RISK_ERROR;
  } else if (currentDistance < 100) {
    instantStatus = RISK_HIGH;
  } else if (currentDistance < 200) {
    instantStatus = RISK_WARNING;
  } else {
    instantStatus = RISK_SAFE;
  }

  // Debounced status transition tracking
  if (instantStatus != lastStatus) {
    zoneDebounceStartTime = now;
    lastStatus = instantStatus;
  }

  // Update status and increment counters only after stability delay confirmation
  if (status != lastStatus && (now - zoneDebounceStartTime > zoneDebounceDelay)) {
    status = lastStatus; 
    
    if (personPresent && status != RISK_ERROR) {
      totalExposures++;
      if (status == RISK_HIGH) {
        highRiskCount++;
        riskScore += 3;
      } else if (status == RISK_WARNING) {
        warningCount++;
        riskScore += 2;
      } else if (status == RISK_SAFE) {
        safeCount++;
        riskScore += 1;
      }
    }
  }

  setLEDsAndBuzzer(status);
  updateDisplay();
  delay(50); // Lowered delay for punchier physical button responsiveness
}


