#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// IT must be connected to vin, not 3v3


// LCD I²C address (0x27 or 0x3F, depending on your module)
#define LCD_ADDR 0x27  
#define SDA_PIN 21
#define SCL_PIN 22

// Initialize LCD (16 columns, 2 rows)
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    Serial.println("Serial communication initialized!");

    // Initialize I²C and LCD
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();

    // Print initial message
    lcd.setCursor(0, 0);
    lcd.print("ESP32 Timer");
    lcd.setCursor(0, 1);
    lcd.print("Time: 0 sec");
}

void loop() {
    // Get elapsed time in seconds
    unsigned long elapsedTime = millis() / 1000;

    // Display elapsed time on LCD
    lcd.setCursor(6, 1);  // Move cursor to the time section
    lcd.print("      ");   // Clear previous numbers
    lcd.setCursor(6, 1);
    lcd.print(elapsedTime);
    lcd.print(" sec");

    // Debug message in Serial Monitor
    Serial.print("Elapsed Time: ");
    Serial.print(elapsedTime);
    Serial.println(" sec");

    delay(1000);  // Update every second
}
