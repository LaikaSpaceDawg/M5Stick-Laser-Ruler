#include <Arduino.h>
#include <M5StickCPlus.h>
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X tofSensor;
unsigned long previousMillis = 0;  // To store the last time the battery was updated
unsigned long lastActivityTime = 0; // To store the last time the button was pressed
const long interval = 30000;       // Interval at which to read battery (30000 milliseconds or 30 seconds)
const long inactivityTimeout = 60000; // Inactivity timeout (60000 milliseconds or 60 seconds)
bool screenOn = true; // Track screen state
int BUILTIN_LED = 10;

void BLINK() {
  int count = 0;
  while (count < 3) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
  digitalWrite(BUILTIN_LED, LOW);
}

void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);
  //digitalWrite(BUILTIN_LED, 0);
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  
  Wire.begin(0, 26);  // SDA, SCL pins if needed to be explicitly defined
  tofSensor.init();
  tofSensor.setTimeout(500);
  tofSensor.startContinuous();
  
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Laser Ruler");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.setTextColor(DARKGREY);
  M5.Lcd.println("Press Button to Measure");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  displayBattery();
  lastActivityTime = millis(); // Initialize last activity time
}

void loop() {
  M5.update();
  
  unsigned long currentMillis = millis();

  // Update battery every 30 seconds if the screen is on
  if (currentMillis - previousMillis >= interval && screenOn) {
    previousMillis = currentMillis;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextSize(2);
      M5.Lcd.println("Laser Ruler");
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(10, 30);
      M5.Lcd.setTextColor(DARKGREY);
      M5.Lcd.println("Press Button to Measure");
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(2);
    displayBattery();
  }
  
  // Turn off the screen after 60 seconds of inactivity
  if (currentMillis - lastActivityTime > inactivityTimeout && screenOn) {
    M5.Axp.SetLDO2(false); // Turn off the LCD backlight (LDO2 controls the LCD power)
    screenOn = false;
  }
  
  // Check if the main button is pressed
  if (M5.BtnA.wasPressed()) {
    // Reactivate the screen if it was off
    if (!screenOn) {
      M5.Axp.SetLDO2(true); // Turn on the LCD backlight
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextSize(2);
      M5.Lcd.println("Laser Ruler");
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(10, 30);
      M5.Lcd.setTextColor(DARKGREY);
      M5.Lcd.println("Press Button to Measure");
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(2);
      screenOn = true;
    }

    long distance_mm = tofSensor.readRangeContinuousMillimeters();
    if (tofSensor.timeoutOccurred()) {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(10, 60);
      M5.Lcd.println("Timeout!");
    } else {
      //BLINK();
      float distance_cm = distance_mm / 10.00;  // Convert mm to cm with one decimal place
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(10, 60);
      M5.Lcd.print("Distance: ");
      M5.Lcd.print(distance_cm, 2);  // Print the distance with 1 decimal place
      M5.Lcd.println(" cm");
      displayBattery();
    }
    lastActivityTime = currentMillis; // Update the last activity time
  }

  delay(10);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void displayBattery() {
  float voltage = M5.Axp.GetBatVoltage();  // Get the battery voltage
  // Update map range for your specific battery, mine tops at 4.19V
  int batteryPercentage = mapFloat(voltage, 3.2, 4.19, 0, 100);  // Map voltage from 3.2V (0%) to 4.2V (100%)
  batteryPercentage = constrain(batteryPercentage, 0, 100);  // Constrain percentage to valid range
  if (batteryPercentage <= 33) {
    M5.Lcd.setTextColor(RED);
  } else if (batteryPercentage > 33 && batteryPercentage <= 66) {
    M5.Lcd.setTextColor(YELLOW);
  } else if (batteryPercentage > 66 && batteryPercentage <= 100) {
    M5.Lcd.setTextColor(GREEN);
  }
  M5.Lcd.setCursor(10,100);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Voltage: %.2fV", voltage);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 110);
  M5.Lcd.printf("Battery: %d%%", batteryPercentage);
  M5.Lcd.setTextColor(WHITE);
}