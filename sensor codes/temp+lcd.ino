
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Sensor and LCD config
const int tempPin = 34;         // LM35DZ connected to GPIO34 (ADC1)
#define LCD_ADDR 0x27           // I²C address of LCD
#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 15           // Buzzer on GPIO15

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);  // 16x2 LCD

// Store last 5 readings
float tempHistory[5] = {98};
int historyIndex = 0;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);  // 12-bit ADC resolution (0–4095)

    // Initialize I²C, LCD and buzzer
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    lcd.setCursor(0, 0);
    lcd.print("Temp Monitor");
    delay(2000); // brief welcome screen
    lcd.clear();
}

void loop() {
    int sensorValue = analogRead(tempPin);
    float voltage = sensorValue * (3.3 / 4095.0);
    float tempC = voltage * 100.0 + 21.0;
    float tempF = tempC * 9.0 / 5.0 + 32.0;

    // Save to history buffer
    tempHistory[historyIndex] = tempF;
    historyIndex = (historyIndex + 1) % 5;

    // Determine status for this reading
    String status;
    if (tempF < 96.0) {
        status = "Low";
    } else if (tempF > 99.0) {
        status = "High";
    } else {
        status = "Normal";
    }

    // Count how many of last 5 are out of range
    int outOfRangeCount = 0;
    for (int i = 0; i < 5; i++) {
        if (tempHistory[i] < 95.0 || tempHistory[i] > 99.0) {
            outOfRangeCount++;
        }
    }

    // Buzzer logic
    if (outOfRangeCount >= 3) {
        digitalWrite(BUZZER_PIN, HIGH);  // turn buzzer on
    } else {
        digitalWrite(BUZZER_PIN, LOW);   // turn buzzer off
    }

    // Display on Serial
    Serial.print("Temperature: ");
    Serial.print(tempF);
    Serial.print(" °F -> ");
    Serial.println(status);

    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(tempF, 1);  // 1 decimal place
    lcd.print(" F");

    lcd.setCursor(0, 1);
    lcd.print("Status: ");
    lcd.print(status);

    delay(1000);  // Update every second
}
