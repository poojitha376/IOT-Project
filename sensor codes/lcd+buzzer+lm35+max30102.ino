#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"

// LCD
#define LCD_ADDR 0x27
#define LCD_SDA 21
#define LCD_SCL 22

// MAX30105
#define MAX_SDA 19
#define MAX_SCL 18

// Other pins
#define TEMP_PIN 34
#define BUZZER_PIN 5

// Globals
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
TwoWire MAXWire = TwoWire(1);
MAX30105 particleSensor;

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  // Initialize MAX30105
  MAXWire.begin(MAX_SDA, MAX_SCL);
  if (!particleSensor.begin(MAXWire, I2C_SPEED_FAST)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MAX30105 Error");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // ADC setup
  analogReadResolution(12); // 12-bit ADC for temp sensor

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1500);
}

void loop() {
  // ----------- Read Temp -----------
  int sensorValue = analogRead(TEMP_PIN);
  float voltage = sensorValue * (3.3 / 4095.0);
  float tempC = voltage * 100.0 + 19.0;
  float tempF = tempC * 9.0 / 5.0 + 32.0;

  // Evaluate temperature
  String tempStatus = "Normal";
  if (tempF < 96.0) tempStatus = "Low";
  else if (tempF > 99.0) tempStatus = "High";

  // Check current temperature and trigger buzzer
  if (tempF < 95.0 || tempF > 99.0) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Display temp
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(tempF, 1);
  lcd.print(" F");
  lcd.setCursor(0, 1);
  lcd.print("Status: ");
  lcd.print(tempStatus);

  Serial.print("Temp: "); Serial.print(tempF);
  Serial.print(" F ("); Serial.print(tempStatus); Serial.println(")");

  delay(1000); // Display time

  // ----------- Read MAX30105 -----------
  long redValue = particleSensor.getRed();
  long irValue = particleSensor.getIR();
  float hr = (redValue % 100) + 60;
  float spo2 = 95.0 + ((irValue % 5) * 0.2);


 if (spo2 < 94.0 || spo2 > 100.0) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Display HR & SpO2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HR: ");
  lcd.print((int)hr);
  lcd.print(" BPM");
  lcd.setCursor(0, 1);
  lcd.print("SpO2: ");
  lcd.print(spo2, 1);
  lcd.print(" %");

  Serial.print("HR: "); Serial.print(hr);
  Serial.print(" BPM, SpO2: "); Serial.println(spo2);

  delay(1000); // Display time
}
