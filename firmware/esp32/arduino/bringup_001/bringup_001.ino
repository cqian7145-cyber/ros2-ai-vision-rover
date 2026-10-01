// Task 001: USB serial only. No GPIO or external peripherals.
void setup() {
  Serial.begin(115200);
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 001 ESP32 Bring-up");
}

void loop() {
  delay(1000);
  Serial.print("uptime_ms=");
  Serial.println(millis());
}
