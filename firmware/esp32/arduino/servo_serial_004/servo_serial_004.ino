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

// One command per line: 1-3 decimal digits, no spaces or sign.
char inputLine[4];
uint8_t inputLength = 0;
bool invalidLine = false;
bool previousWasCR = false;

void processLine() {
  if (invalidLine || inputLength == 0) {
    Serial.println("ERROR: enter 1-3 digits (0-180), then newline");
  } else {
    int angle = 0;
    for (uint8_t i = 0; i < inputLength; ++i) {
      angle = angle * 10 + (inputLine[i] - '0');
    }
    setServoAngle(angle);
  }
  inputLength = 0;
  invalidLine = false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 004 SG90 Serial Control");
  initServo();
  Serial.println("Enter 0-180; select Newline or Both NL & CR in Serial Monitor.");
}

void loop() {
  delay(1);
  while (Serial.available() > 0) {
    const char c = char(Serial.read());
    if (c == '\n' && previousWasCR) {
      previousWasCR = false;  // Treat CRLF as one line ending.
      continue;
    }
    previousWasCR = (c == '\r');
    if (c == '\r' || c == '\n') {
      processLine();
    } else if (c < '0' || c > '9' || inputLength >= sizeof(inputLine) - 1) {
      invalidLine = true;  // Reject the entire bad/overlong line until newline.
    } else if (!invalidLine) {
      inputLine[inputLength++] = c;
    }
  }
}
