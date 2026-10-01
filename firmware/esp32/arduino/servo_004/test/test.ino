#include <Arduino.h>

constexpr uint8_t SERVO_PIN = 25;

constexpr uint32_t SERVO_FREQ = 50;       // 50 Hz
constexpr uint8_t SERVO_RESOLUTION = 16;  // 16-bit

constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;

uint32_t pulseUsToDuty(uint16_t pulseUs) {
  constexpr uint32_t PERIOD_US = 20000;  // 50 Hz = 20 ms
  constexpr uint32_t MAX_DUTY = 65535;

  return ((uint64_t)pulseUs * MAX_DUTY) / PERIOD_US;
}

void setServoAngle(int angle) {
  angle = constrain(angle, 0, 180);

  uint16_t pulseUs =
      map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);

  uint32_t duty = pulseUsToDuty(pulseUs);

  ledcWrite(SERVO_PIN, duty);

  Serial.print("angle_deg=");
  Serial.print(angle);
  Serial.print(" pulse_us=");
  Serial.println(pulseUs);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 004 - SG90 PWM Bring-up");

  if (!ledcAttach(
          SERVO_PIN,
          SERVO_FREQ,
          SERVO_RESOLUTION)) {

    Serial.println("ERROR: LEDC attach failed");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("Servo PWM initialized");

  setServoAngle(90);
}

void loop() {
  setServoAngle(30);
  delay(1500);

  setServoAngle(90);
  delay(1500);

  setServoAngle(150);
  delay(1500);

  setServoAngle(90);
  delay(1500);
}