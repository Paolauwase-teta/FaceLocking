/*
 * High-Precision Single Servo Tracker & Standalone Auto-Sweep for ESP8266
 * 
 * Hardware Wiring (ESP8266 NodeMCU / D1 Mini):
 *   - Signal (Yellow/Orange) -> Pin D1 (GPIO 5)
 *   - Power (Red)            -> Pin VIN or 5V (5V USB power)
 *   - Ground (Brown/Black)   -> Pin GND
 * 
 * Communication:
 *   - Baud Rate: 115200 baud
 *   - Protocol: Angle integer followed by newline (e.g. "90\n")
 *   - Range: 15° to 165° (safe range for SG90 / MG90S)
 */

#include <Servo.h>

#ifndef D1
#define D1 5
#endif

// Pin D1 (GPIO 5)
const int SERVO_PIN = D1;
Servo myServo;

int currentAngle = 90;
unsigned long lastCommandTime = 0;
unsigned long lastMoveTime = 0;
int sweepState = 0;
bool pythonControlling = false;

// Fast non-blocking serial receive buffer
char rxBuffer[16];
int rxIndex = 0;

void setup() {
  Serial.begin(115200);

  // Attach servo on D1 with standard 544us to 2400us pulse width
  myServo.attach(SERVO_PIN, 544, 2400);
  myServo.write(90);

  Serial.println("\n=== ESP8266 SERVO TRACKER (PIN D1) READY ===");
  Serial.println("Listening for tracking angles on Serial (115200 baud)...");
}

void loop() {
  // 1. Zero-latency non-blocking serial packet parser
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (rxIndex > 0) {
        rxBuffer[rxIndex] = '\0';
        int angle = atoi(rxBuffer);
        rxIndex = 0;

        // Verify valid angle in SG90 operating bounds
        if (angle >= 10 && angle <= 170) {
          pythonControlling = true;
          lastCommandTime = millis();
          currentAngle = angle;
          myServo.write(currentAngle);

          // Return ACK to Python
          Serial.print("ACK:");
          Serial.println(currentAngle);
        }
      }
    } else if (c >= '0' && c <= '9') {
      if (rxIndex < (int)sizeof(rxBuffer) - 1) {
        rxBuffer[rxIndex++] = c;
      }
    }
  }

  // 2. Standby Timeout: If Python disconnects or stops tracking for > 3.5s,
  // resume standalone test sweep so user knows the hardware is healthy
  if (pythonControlling && (millis() - lastCommandTime > 3500)) {
    pythonControlling = false;
    Serial.println(">> Python tracking idle. Entering standby sweep...");
    sweepState = 0;
    lastMoveTime = millis();
  }

  // 3. Standalone Continuous Sweep when Python is not actively sending angles
  if (!pythonControlling) {
    if (millis() - lastMoveTime > 1200) {
      lastMoveTime = millis();

      if (sweepState == 0) {
        myServo.write(45);
        sweepState = 1;
      } 
      else if (sweepState == 1) {
        myServo.write(90);
        sweepState = 2;
      } 
      else if (sweepState == 2) {
        myServo.write(135);
        sweepState = 3;
      } 
      else {
        myServo.write(90);
        sweepState = 0;
      }
    }
  }
}
