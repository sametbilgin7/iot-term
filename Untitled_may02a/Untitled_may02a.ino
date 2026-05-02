#include "arduino_secrets.h"
#include "thingProperties.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include <ESP32Servo.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define LDR_LEFT   34
#define LDR_RIGHT  35
#define SERVO_PIN  18

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
Adafruit_INA219 ina219;
Servo solarServo;

int servoPos = 90;

const int minAngle = 0;
const int maxAngle = 180;

const int threshold = 700;
const int stepAmount = 1;

void setup() {
  Serial.begin(115200);
  delay(1500);

  Wire.begin(SDA_PIN, SCL_PIN);

  solarServo.setPeriodHertz(50);
  solarServo.attach(SERVO_PIN, 500, 2400);
  solarServo.write(servoPos);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  if (!ina219.begin()) {
    Serial.println("INA219 bulunamadi!");
  }

  initProperties();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  delay(1000);
}

void loop() {
  ArduinoCloud.update();

  int leftVal = analogRead(LDR_LEFT);
  int rightVal = analogRead(LDR_RIGHT);

  int diff = leftVal - rightVal;
  String direction = "STABLE";

  if (diff > threshold) {
    servoPos += stepAmount;
    direction = "LEFT";
  } 
  else if (diff < -threshold) {
    servoPos -= stepAmount;
    direction = "RIGHT";
  }

  servoPos = constrain(servoPos, minAngle, maxAngle);
  solarServo.write(servoPos);

  float busVoltage = ina219.getBusVoltage_V();
  float shuntVoltage = ina219.getShuntVoltage_mV() / 1000.0;
  float loadVoltage = busVoltage + shuntVoltage;

  float current = ina219.getCurrent_mA();

  if (current < 0) {
    current = 0;
  }

  float displayCurrent = current * 3.0;

  ldrLeft = leftVal;
  ldrRight = rightVal;
  servoAngle = servoPos;
  panelVoltage = loadVoltage;
  panelCurrent = displayCurrent;
  trackingStatus = direction;
  isStable = (direction == "STABLE");

  Serial.print("L:");
  Serial.print(leftVal);
  Serial.print(" R:");
  Serial.print(rightVal);
  Serial.print(" Diff:");
  Serial.print(diff);
  Serial.print(" Dir:");
  Serial.print(direction);
  Serial.print(" Angle:");
  Serial.print(servoPos);

  Serial.print(" V:");
  Serial.print(loadVoltage, 3);
  Serial.print(" I:");
  Serial.println(displayCurrent, 3);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("SOLAR TRACKING");

  display.setCursor(0, 12);
  display.print("L:");
  display.print(leftVal);
  display.print(" R:");
  display.println(rightVal);

  display.setCursor(0, 24);
  display.print("Diff:");
  display.print(diff);

  display.setCursor(0, 36);
  display.print("Angle:");
  display.print(servoPos);

  display.setCursor(0, 48);
  display.print("V:");
  display.print(loadVoltage, 3);

  display.setCursor(70, 48);
  display.print("I:");
  display.print(displayCurrent, 1);

  display.display();

  delay(300);
}

void onLdrLeftChange() {}
void onLdrRightChange() {}

void onServoAngleChange() {
  servoPos = constrain(servoAngle, minAngle, maxAngle);
  solarServo.write(servoPos);
}

void onPanelVoltageChange() {}
void onPanelCurrentChange() {}
void onTrackingStatusChange() {}
void onIsStableChange() {}