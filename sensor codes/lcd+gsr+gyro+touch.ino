#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// I2C Pins and LCD
#define SDA_PIN 21
#define SCL_PIN 22
#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// Capacitive Touch Sensor Pins
#define TOUCH_1 25
#define TOUCH_2 26
#define TOUCH_3 27
#define TOUCH_4 14

// GSR Sensor
#define GSR_PIN 34

// ADXL345 Accelerometer
#define ADXL345_ADDR 0x53
#define ADXL345_DEVID 0x00
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_DATAX0 0x32
#define ADXL345_DATAY0 0x34
#define ADXL345_DATAZ0 0x36
#define ADXL345_BW_RATE 0x2C

unsigned long lastCycle = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Starting...");
  
  pinMode(TOUCH_1, INPUT);
  pinMode(TOUCH_2, INPUT);
  pinMode(TOUCH_3, INPUT);
  pinMode(TOUCH_4, INPUT);

  delay(2000);
  lcd.clear();
  initADXL345();
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long phase = (currentMillis / 2000) % 3;

  if (phase == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Touch Sensors     ");
    if (digitalRead(TOUCH_1)) lcd.setCursor(0, 1), lcd.print("Touchpad 1       ");
    else if (digitalRead(TOUCH_2)) lcd.setCursor(0, 1), lcd.print("Touchpad 2       ");
    else if (digitalRead(TOUCH_3)) lcd.setCursor(0, 1), lcd.print("Touchpad 3       ");
    else if (digitalRead(TOUCH_4)) lcd.setCursor(0, 1), lcd.print("Touchpad 4       ");
    else lcd.setCursor(0, 1), lcd.print("No Touch         ");
  }

  else if (phase == 1) {
    int gsrValue = analogRead(GSR_PIN);
    lcd.setCursor(0, 0);
    lcd.print("GSR Sensor        ");
    lcd.setCursor(0, 1);
    lcd.print("Value: ");
    lcd.print(gsrValue);
    lcd.print("     ");
    Serial.print("GSR Raw Value: ");
    Serial.println(gsrValue);
  }

  else if (phase == 2) {
    int16_t x, y, z;
    readAcceleration(&x, &y, &z);
    float xg = x * 0.0078;
    float yg = y * 0.0078;
    float zg = z * 0.0078;

    lcd.setCursor(0, 0);
    lcd.print("Accel X:");
    lcd.print(xg, 1);
    lcd.setCursor(0, 1);
    lcd.print("Y:");
    lcd.print(yg, 1);
    lcd.print(" Z:");
    lcd.print(zg, 1);
    Serial.printf("X: %.2f g, Y: %.2f g, Z: %.2f g\n", xg, yg, zg);
  }

  delay(200);
}

// ----------- ADXL345 Functions -------------
void initADXL345() {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.endTransmission();
  writeRegister(ADXL345_BW_RATE, 0x0A);
  writeRegister(ADXL345_DATA_FORMAT, 0x09);
  writeRegister(ADXL345_POWER_CTL, 0x08);
}

void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

byte readRegister(byte reg) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(ADXL345_ADDR, 1);
  return Wire.available() ? Wire.read() : 0;
}

void readAcceleration(int16_t *x, int16_t *y, int16_t *z) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(ADXL345_DATAX0);
  Wire.endTransmission();
  Wire.requestFrom(ADXL345_ADDR, 6);
  if (Wire.available() == 6) {
    *x = Wire.read() | (Wire.read() << 8);
    *y = Wire.read() | (Wire.read() << 8);
    *z = Wire.read() | (Wire.read() << 8);
  } else {
    *x = *y = *z = 0;
  }
}
