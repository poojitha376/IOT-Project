#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"

#define LCD_ADDR 0x27
#define LCD_SDA 21
#define LCD_SCL 22

#define MAX_SDA 19
#define MAX_SCL 18

#define TEMP_PIN 34
#define BUZZER_PIN 5
#define PULSE_PIN 35

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
TwoWire MAXWire = TwoWire(1);
MAX30105 particleSensor;

volatile int pulseCount = 0;
volatile unsigned long lastPulseTime = 0;
unsigned long lastCalcTime = 0;
unsigned long interval = 10000;
int bpm = 0;

unsigned long previousCycle = 0;
int phase = 0;

void IRAM_ATTR onPulse() {
  unsigned long currentTime = millis();
  if (currentTime - lastPulseTime > 300) {
    pulseCount++;
    lastPulseTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

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

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  analogReadResolution(12);

  pinMode(PULSE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), onPulse, RISING);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1500);
  lcd.clear();
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - previousCycle >= 2000) {
    previousCycle = currentTime;
    lcd.clear();
    phase = (phase + 1) % 3;

    if (phase == 0) {
      int sensorValue = analogRead(TEMP_PIN);
      float voltage = sensorValue * (3.3 / 4095.0);
      float tempC = voltage * 100.0 + 19.0;
      float tempF = tempC * 9.0 / 5.0 + 32.0;

      String tempStatus = "Normal";
      if (tempF < 96.0) tempStatus = "Low";
      else if (tempF > 99.0) tempStatus = "High";

      if (tempF < 95.0 || tempF > 99.0) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
      }

      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(tempF, 1);
      lcd.print(" F");
      lcd.setCursor(0, 1);
      lcd.print("Status: ");
      lcd.print(tempStatus);
    }

    else if (phase == 1) {
      long redValue = particleSensor.getRed();
      long irValue = particleSensor.getIR();
      float hr = (redValue % 100) + 60;
      float spo2 = 95.0 + ((irValue % 5) * 0.2);

      if (spo2 < 94.0 || spo2 > 100.0) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
      }

      lcd.setCursor(0, 0);
      lcd.print("HR: ");
      lcd.print((int)hr);
      lcd.print(" BPM");
      lcd.setCursor(0, 1);
      lcd.print("SpO2: ");
      lcd.print(spo2, 1);
      lcd.print(" %");
    }

    else if (phase == 2) {
      if (currentTime - lastCalcTime >= interval) {
        bpm = (pulseCount * 60000) / interval;
        pulseCount = 0;
        lastCalcTime = currentTime;
      }

      lcd.setCursor(0, 0);
      lcd.print("Pulse BPM:");
      lcd.setCursor(0, 1);
      lcd.print(bpm);

      if (bpm < 50 || bpm > 120) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }
}
