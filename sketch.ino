#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

// Simulation Variables
int totalExposures = 0;
int highRiskCount = 0;
int warningCount = 0;
int safeCount = 0;
unsigned long simulationStartTime = 0;
bool personPresent = false;
float currentDistance = 0;
float previousDistance = 0;

// Debounce
unsigned long lastBtnPress = 0;
const int debounceDelay = 300;

// For tracking state changes
String lastStatus = "";

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
  }

  // Setup pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RESET_BTN, INPUT_PULLUP);

  // Welcome screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("VIRUS SPREAD");
  display.setCursor(5, 25);
  display.println("SIMULATOR v1.0");
  display.setCursor(15, 45);
  display.println("Starting...");
  display.display();
  delay(2000);

  simulationStartTime = millis();
  Serial.println("Virus Spread Simulator Started!");
  Serial.println("Drag the HC-SR04 slider to change distance!");
  Serial.println("-------------------------------------------");
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return 999; // No echo

  float distance = duration * 0.034 / 2;

  // Noise filtering
  if (previousDistance > 0 && abs(distance - previousDistance) > 100) {
    return previousDistance;
  }

  return distance;
}

void resetSimulation() {
  totalExposures = 0;
  highRiskCount = 0;
  warningCount = 0;
  safeCount = 0;
  simulationStartTime = millis();

  Serial.println("=== SIMULATION RESET ===");

  // Flash LEDs
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

void updateDisplay() {
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Distance Detector");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Distance
  display.setCursor(0, 14);
  display.print("Distance: ");
  if (currentDistance >= 999) {
    display.print("NO ECHO");
  } else if (currentDistance > 200) {
    display.print(">200 cm");
  } else {
    display.print((int)currentDistance);
    display.print(" cm");
  }

  // Status
  display.setCursor(0, 24);
  display.print("Status: ");
  if (currentDistance >= 999) {
    display.print("SENSOR ERR");
  } else if (currentDistance < 100) {
    display.print("HIGH RISK");
  } else if (currentDistance < 200) {
    display.print("WARNING");
  } else {
    display.print("SAFE");
  }

  // Stats
  display.setCursor(0, 37);
  display.print("Exposures: ");
  display.print(totalExposures);

  display.setCursor(0, 47);
  display.print("HighRisk: ");
  display.print(highRiskCount);
  display.print("  Warn: ");
  display.print(warningCount);

  // Time
  unsigned long runTime = (millis() - simulationStartTime) / 1000;
  display.setCursor(0, 57);
  display.print("Time: ");
  display.print(runTime / 60);
  display.print("m ");
  display.print(runTime % 60);
  display.print("s");

  display.display();
}

void setLEDsAndBuzzer(String status) {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);

  if (status == "HIGH_RISK") {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 2000);
  }
  else if (status == "WARNING") {
    digitalWrite(YELLOW_LED, HIGH);
    tone(BUZZER, 1000);
  }
  else if (status == "SAFE") {
    digitalWrite(GREEN_LED, HIGH);
  }
  else if (status == "ERROR") {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 300);
  }
}

void loop() {
  // Reset button
  if (digitalRead(RESET_BTN) == LOW) {
    if (millis() - lastBtnPress > debounceDelay) {
      resetSimulation();
      lastBtnPress = millis();
    }
  }

  // PIR
  personPresent = digitalRead(PIR_PIN);

  // Distance
  previousDistance = currentDistance;
  currentDistance = getDistance();

  // Determine status
  String status = "";
  if (currentDistance >= 999) status = "ERROR";
  else if (currentDistance < 100) status = "HIGH_RISK";
  else if (currentDistance < 200) status = "WARNING";
  else status = "SAFE";

  // Exposure counting
  if (personPresent && status != "ERROR") {
    if (status != lastStatus) {
      totalExposures++;

      if (status == "HIGH_RISK") highRiskCount++;
      else if (status == "WARNING") warningCount++;
      else if (status == "SAFE") safeCount++;
    }
  }

  lastStatus = status;

  // Outputs
  setLEDsAndBuzzer(status);
  updateDisplay();

  delay(200);
}

