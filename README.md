# 🛡️ Distance Risk Analyzer

Real‑Time Proximity, Exposure & Safety Monitoring System
The Distance Risk Analyzer is an embedded safety‑monitoring system designed to detect human proximity, classify risk levels, track exposure events, and provide real‑time visual and audio alerts,
using an ESP32, HC‑SR04 ultrasonic sensor, PIR motion sensor, OLED display, LED indicators, and buzzer, the system simulates how distance‑based risk detection works in real environments such as health‑safety zones, industrial spaces, or crowd‑monitoring scenarios.

This project also supports an optional Emergency Button for critical situations, enabling rapid escalation when dangerous conditions or symptoms are detected.

------------------------------------------------------------------------------------------------------------------------------------
🚀 Key Features
------------------------------------------------------------------------------------------------------------------------------------

🔍 1. Real‑Time Distance Measurement
- Uses the HC‑SR04 ultrasonic sensor
- Filters noise and ignores invalid readings
- Displays distance in centimeters or “NO ECHO”

👤 2. Human Presence Detection
- PIR sensor confirms whether a person is actually present
- Prevents false exposure counts

🚦 3. Intelligent Risk Classification
- Distance	Status	Indicators
< 100 cm	HIGH RISK	Red LED + Loud Buzzer
100–200 cm	WARNING	Yellow LED + Soft Buzzer
> 200 cm	SAFE	Green LED

📊 4. Exposure Tracking
- Total exposures
- High‑risk events
- Warning events
- Safe detections
- Runtime counter

🖥️ 5. OLED Dashboard
Displays:
- Distance
- Status
- Exposure statistics
- System uptime
- Sensor errors

🔁 6. Reset Button
- Clears all counters
- Resets runtime
- Flashes LEDs for confirmation

🚨 7. Optional Emergency Button
- Can be added to trigger emergency workflows
- Ideal for simulations involving symptoms or danger
- Supports UI‑based or hardware‑based activation

----------------------------------------------------------------------
🧩 Hardware Components
----------------------------------------------------------------------
- ESP32 Dev Module
- HC‑SR04 Ultrasonic Sensor
- PIR Motion Sensor
- SSD1306 OLED Display (128×64)
- Active Buzzer
- 3× LEDs (Green, Yellow, Red)
- Pushbutton (Reset)
  
----------------------------------------------------------------------------------------
🧠 System Logic Overview
-----------------------------------------------------------------------------------------

1. Detect Presence
- PIR sensor checks if a person is nearby.
2. Measure Distance
- Ultrasonic sensor calculates distance in centimeters.
3. Classify Risk Level
- Based on thresholds:
< 100 cm → High Risk
100–200 cm → Warning
> 200 cm → Safe
4. Trigger Alerts
- LEDs
- Buzzer
- OLED updates
5. Log Exposure Events
-Only when a person is detected.
6. Emergency Handling (Optional)
- Emergency button can escalate critical conditions.
  
----------------------------------------------------------------------------
🛠️ Installation & Setup
----------------------------------------------------------------------------

1. Clone the Repository
- bash
git clone https://github.com/yourusername/distance-risk-analyzer
2. Install Required Libraries
- Adafruit_GFX
- Adafruit_SSD1306
3. Select Board
-Board: ESP32 Dev Module
4. Upload Code
- Connect your ESP32 and upload the sketch.
  
-----------------------------------------------------------------------------------------
📷 OLED Display Output
------------------------------------------------------------------------------------------

The OLED shows:

* Distance (cm or NO ECHO)
* Status (SAFE / WARNING / HIGH RISK / SENSOR ERR)
* Exposure counters
* Runtime (minutes + seconds)
  
-------------------------------------------------------------------------------------------------
🧪 Use Cases
--------------------------------------------------------------------------------------------------

* Health‑safety simulations
* Social‑distance monitoring
* Industrial proximity alerts
  
-------------------------------------------------------------------------------------------------------
🧱 Project Architecture
--------------------------------------------------------------------------------------------------------

Distance Risk Analyzer
│
├── Sensors
│   ├── Ultrasonic (Distance)
│   └── PIR (Presence)
│
├── Processing
│   ├── Risk Classification
│   ├── Exposure Logging
│   └── Error Handling
│
├── Outputs
│   ├── OLED Dashboard
│   ├── LEDs (3‑level risk)
│   └── Buzzer Alerts
│
└── Controls
    ├── Reset Button
    └── Emergency Button 
    
--------------------------------------------------------------------------------------------------------------------
🧯 Emergency Button Integration 
---------------------------------------------------------------------------------------------------------------------

Add a dedicated emergency button that:

* Activates when symptoms or danger are detected
* Allows the user to request help
* Can be mirrored in a dashboard for remote monitoring
* Includes a confirmation step to prevent false alarms
* This makes the system suitable for health‑risk simulations or assistive‑safety devices.

------------------------------------------------------------------------------------------------------------------------
👩‍💻 Author
------------------------------------------------------------------------------------------------------------------------

Moisha Ndlovu  
Frontend Developer • Data Science & Analytics Student • AI Learning Tools Builder
Pretoria, South Africa

--------------------------------------------------------------------------------------------------------------------------

