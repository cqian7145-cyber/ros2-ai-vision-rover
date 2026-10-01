#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ESP32: SDA = GPIO21, SCL = GPIO22
  Wire.begin(21, 22);

  Serial.println();
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 002A - I2C Scanner");
  Serial.println("Scanning I2C bus...");
}

void loop() {
  int deviceCount = 0;

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);

    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");

      if (address < 16) {
        Serial.print("0");
      }

      Serial.println(address, HEX);
      deviceCount++;
    }
  }

  if (deviceCount == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.print("Total devices: ");
    Serial.println(deviceCount);
  }

  Serial.println("--------------------");

  delay(3000);
}