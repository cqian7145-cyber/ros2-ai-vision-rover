#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 21
#define SCL_PIN 22

#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("ROS2 AI Vision Rover");
  Serial.println("Task 003 - OLED Bring-up");

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(
          SSD1306_SWITCHCAPVCC,
          OLED_ADDRESS)) {

    Serial.println("ERROR: OLED initialization failed!");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("OLED initialized");

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ROS2 AI Rover");

  display.setCursor(0, 16);
  display.println("Task 003");

  display.display();
}

void loop() {

  display.clearDisplay();

  display.setCursor(0, 0);
  display.println("ROS2 AI Rover");

  display.setCursor(0, 16);
  display.println("Task 003");

  display.setCursor(0, 32);
  display.print("Uptime: ");
  display.print(millis() / 1000);
  display.println(" s");

  display.display();

  delay(200);
}