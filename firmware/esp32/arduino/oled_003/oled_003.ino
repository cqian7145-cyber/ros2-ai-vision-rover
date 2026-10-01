#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Confirm these settings against the actual OLED module.
const uint8_t OLED_ADDRESS = 0x3C;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;  // Change to 32 for a 128x32 SSD1306.
const unsigned long REFRESH_MS = 200;
// No reset GPIO; keep the shared bus at 100 kHz during and after display writes.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 100000, 100000);
bool oledReady = false;
unsigned long lastRefreshMs = 0;

bool devicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void scanI2C() {
  Serial.println("I2C scan:");
  unsigned int count = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    if (devicePresent(address)) {
      Serial.printf("I2C_FOUND=0x%02X\n", address);
      ++count;
    }
    delay(1);
  }
  if (count == 0) Serial.println("I2C_NO_DEVICES");
}

void initOLED() {
  // begin() alone is not proof that a physical display acknowledged I2C.
  if (!devicePresent(OLED_ADDRESS)) {
    Serial.println("OLED_NOT_FOUND");
    return;
  }
  // Wire is already initialized: periphBegin=false avoids restarting it.
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS, false, false)) {
    Serial.println("OLED_INIT_FAILED");
    return;
  }
  oledReady = true;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  Serial.println("OLED initialized; verify visible text");
}

void drawUptime() {
  if (!oledReady) return;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("ROS2 AI Rover");
  display.println("Task 003");
  display.print("Uptime: ");
  display.print(millis() / 1000);
  display.println(" s");
  display.display();
}

void setup() {
  Serial.begin(115200);
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 003 OLED Bring-up");
  if (!Wire.begin(21, 22)) {
    Serial.println("I2C_INIT_FAILED");
    return;
  }
  Wire.setClock(100000);
  Wire.setTimeOut(50);
  scanI2C();
  initOLED();
  drawUptime();
}

void loop() {
  delay(1);
  if (!oledReady) return;
  const unsigned long now = millis();
  if (now - lastRefreshMs < REFRESH_MS) return;
  lastRefreshMs = now;
  drawUptime();
}
