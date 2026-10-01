#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 002B - VL53L0X Distance");

  // ESP32: SDA = GPIO21, SCL = GPIO22
  Wire.begin(21, 22);

  sensor.setTimeout(500);

  if (!sensor.init()) {
    Serial.println("ERROR: VL53L0X initialization failed!");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("VL53L0X initialized");

  // 连续测距，每约 50 ms 测一次
  sensor.startContinuous(50);
}

void loop() {
  uint16_t distance_mm = sensor.readRangeContinuousMillimeters();

  if (sensor.timeoutOccurred()) {
    Serial.println("VL53L0X_TIMEOUT");
  } else {
    Serial.print("distance_mm=");
    Serial.println(distance_mm);
  }

  delay(200);
}
