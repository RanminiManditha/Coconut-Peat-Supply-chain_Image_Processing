#include <Arduino.h>

// ---------- Ultrasonic Sensor Pins ----------
#define TRIG_PIN   5    // Trigger pin for HC-SR04
#define ECHO_PIN   18   // Echo pin for HC-SR04
float distanceThreshold = 10.0;  // Object detection range in cm

// ---------- Relay Pin (to power ESP32-CAM) ----------
#define RELAY_PIN  19   // Assume active LOW (LOW = power ON)

// ---------- UART Pins for ESP32-CAM Communication ----------
#define CAM_RX 16       // Normal ESP32 RX2 (connected to ESP32-CAM TX)
#define CAM_TX 17       // Normal ESP32 TX2 (if needed; connected to ESP32-CAM RX)

// ---------- Timeout for receiving result ----------
#define RESULT_TIMEOUT 10000  // Maximum wait time (in ms) for a result

void setup() {
  Serial.begin(115200);
  // Initialize Serial2 for communication with the ESP32-CAM.
  Serial2.begin(115200, SERIAL_8N1, CAM_RX, CAM_TX);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay off initially (camera off)

  Serial.println("Normal ESP32: Setup complete.");
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1;
  return duration / 58.0;
}

void powerOnCamera() {
  Serial.println("Turning relay ON -> powering up ESP32-CAM.");
  digitalWrite(RELAY_PIN, LOW);
  delay(500);  // Wait for the ESP32-CAM to boot.
}

void powerOffCamera() {
  Serial.println("Powering off ESP32-CAM.");
  digitalWrite(RELAY_PIN, HIGH);
}

void loop() {
  float distance = getDistance();
  if (distance < 0) {
    Serial.println("No echo from ultrasonic sensor (timeout).");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  if (distance > 0 && distance < distanceThreshold) {
    Serial.println("Object detected! Powering on ESP32-CAM...");
    powerOnCamera();

    // Wait for the result from the ESP32-CAM.
    unsigned long startTime = millis();
    bool resultReceived = false;
    String resultLine = "";

    Serial.println("Waiting for result from ESP32-CAM...");
    while (millis() - startTime < RESULT_TIMEOUT) {
      if (Serial2.available()) {
        resultLine = Serial2.readStringUntil('\n');
        resultLine.trim();
        // Look for the marker "RESULT:"
        if (resultLine.startsWith("RESULT:")) {
          Serial.print("Received from CAM: ");
          Serial.println(resultLine);
          resultReceived = true;
          break;
        } else {
          // Optionally, print other output from the CAM (boot messages, etc.)
          Serial.print("Ignored: ");
          Serial.println(resultLine);
        }
      }
      delay(20);
    }

    if (!resultReceived) {
      Serial.println("No valid result received within timeout.");
    }

    // Optionally, power off the camera after processing.
    powerOffCamera();
  } else {
    Serial.println("No object detected.");
  }

  delay(500);
}
