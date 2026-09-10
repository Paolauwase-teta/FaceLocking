/*
 * Single Servo Pan Controller & Standalone Auto-Sweep for ESP8266
 * 
 * Pin Configuration:
 *   - Signal (Yellow/Orange) -> D1 (GPIO 5)
 *   - Power (Red)            -> VIN / VU (5V from USB)
 *   - Ground (Brown/Black)   -> GND
 */

#include <Servo.h>

#ifndef D1
#define D1 5
#endif

// Exactly 1 Servo on Pin D1
const int SERVO_PIN = D1;
Servo myServo;

int currentAngle = 90;
unsigned long lastMoveTime = 0;
int sweepState = 0;
bool pythonControlling = false;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(5);

  // Attach the 1 single servo on pin D1
  myServo.attach(SERVO_PIN, 544, 2400);
  myServo.write(90);

  Serial.println("\n=== SINGLE SERVO TEST (PIN D1) READY ===");
}

void loop() {
  // 1. If Python is running, track Python angle commands
  if (Serial.available() > 0) {
    int angle = Serial.parseInt();
    if (angle >= 10 && angle <= 170) {
      pythonControlling = true;
      currentAngle = angle;
      myServo.write(currentAngle);
      
      Serial.print("ACK:");
      Serial.println(currentAngle);
    }
  }

  // 2. Standalone Continuous Sweep when Python is not sending commands
  if (!pythonControlling) {
    if (millis() - lastMoveTime > 1200) {
      lastMoveTime = millis();

      if (sweepState == 0) {
        Serial.println(">> Moving to 45° (LEFT)");
        myServo.write(45);
        sweepState = 1;
      } 
      else if (sweepState == 1) {
        Serial.println(">> Moving to 90° (CENTER)");
        myServo.write(90);
        sweepState = 2;
      } 
      else if (sweepState == 2) {
        Serial.println(">> Moving to 135° (RIGHT)");
        myServo.write(135);
        sweepState = 3;
      } 
      else {
        Serial.println(">> Moving to 90° (CENTER)");
        myServo.write(90);
        sweepState = 0;
      }
    }
  }
}
