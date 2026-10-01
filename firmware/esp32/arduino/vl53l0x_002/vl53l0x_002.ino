#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;
const unsigned long MEASUREMENT_INTERVAL_MS = 200;
const uint16_t SENSOR_TIMEOUT_MS = 500;
bool sensorReady = false;
unsigned long lastReadMs = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 002 VL53L0X Bring-up");

  // Use the selected board's default SDA and SCL pins.
  if (!Wire.begin()) {
    Serial.println("I2C_INIT_FAILED");
    return;
  }

  sensor.setTimeout(SENSOR_TIMEOUT_MS);
  if (!sensor.init()) {
    Serial.println("VL53L0X_INIT_FAILED");
    if (sensor.timeoutOccurred()) {
      Serial.println("VL53L0X_TIMEOUT");
    }
    return;
  }

  sensor.startContinuous(MEASUREMENT_INTERVAL_MS);
  sensorReady = true;
  lastReadMs = millis();
  Serial.println("VL53L0X initialized");
}

void loop() {
  delay(1);  // Yield without a long fixed delay.
  if (!sensorReady) {
    return;  // Check wiring, then reset the board to retry initialization.
  }

  const unsigned long now = millis();
  if (now - lastReadMs < MEASUREMENT_INTERVAL_MS) {
    return;
  }
  lastReadMs = now;

  // This library call waits for a measurement, bounded by the sensor timeout.
  const uint16_t distanceMm = sensor.readRangeContinuousMillimeters();
  if (sensor.timeoutOccurred()) {
    Serial.println("VL53L0X_TIMEOUT");
    return;  // Do not print a timeout return value as a valid distance.
  }

  Serial.print("distance_mm=");
  Serial.println(distanceMm);
}
