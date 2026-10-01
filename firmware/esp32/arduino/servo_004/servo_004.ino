#include <Arduino.h>

const uint8_t SERVO_PIN = 25;
const uint32_t PWM_FREQUENCY_HZ = 50;
const uint8_t PWM_RESOLUTION_BITS = 16;
const uint32_t PWM_PERIOD_US = 1000000UL / PWM_FREQUENCY_HZ;
const uint32_t PWM_TICKS = 1UL << PWM_RESOLUTION_BITS;
const uint32_t MIN_PULSE_US = 1000;
const uint32_t MAX_PULSE_US = 2000;
bool servoReady = false;

uint32_t pulseUsToDuty(uint32_t pulseUs) {
  // 65536 timer ticks per period; round to the nearest tick.
  const uint64_t duty = (uint64_t(pulseUs) * PWM_TICKS + PWM_PERIOD_US / 2) / PWM_PERIOD_US;
  return duty >= PWM_TICKS ? PWM_TICKS - 1 : uint32_t(duty);
}

bool setServoAngle(int angleDeg) {
  if (angleDeg < 0 || angleDeg > 180) {
    Serial.println("ERROR: angle must be 0-180");
    return false;
  }
  if (!servoReady) {
    Serial.println("ERROR: PWM not ready");
    return false;
  }
  const uint32_t pulseUs = MIN_PULSE_US +
      (uint32_t(angleDeg) * (MAX_PULSE_US - MIN_PULSE_US) + 90) / 180;
  // Core 3.x uses the GPIO pin, not the channel number.
  if (!ledcWrite(SERVO_PIN, pulseUsToDuty(pulseUs))) {
    Serial.println("ERROR: ledcWrite failed");
    servoReady = false;
    return false;
  }
  // Report the command, not a measured physical angle.
  Serial.printf("angle_deg=%d pulse_us=%lu\n", angleDeg, (unsigned long)pulseUs);
  return true;
}

void initServo() {
  if (!ledcAttach(SERVO_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS)) {
    Serial.println("ERROR: ledcAttach failed");
    return;
  }
  servoReady = true;
  setServoAngle(90);
}

const int TEST_ANGLES[] = {30, 90, 150};
const unsigned long MOVE_INTERVAL_MS = 2000;
unsigned long lastMoveMs = 0;
uint8_t nextAngle = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 004 SG90 PWM Bring-up");
  initServo();
  lastMoveMs = millis();
}

void loop() {
  delay(1);
  if (!servoReady) return;
  const unsigned long now = millis();
  if (now - lastMoveMs < MOVE_INTERVAL_MS) return;
  lastMoveMs = now;
  if (setServoAngle(TEST_ANGLES[nextAngle])) {
    nextAngle = (nextAngle + 1) % 3;
  }
}
