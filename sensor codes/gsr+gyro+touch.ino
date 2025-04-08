#include <Wire.h>

#define TOUCH_1 25
#define TOUCH_2 26
#define TOUCH_3 27
#define TOUCH_4 14
#define GSR_PIN 34

#define ADXL345_ADDR 0x53
#define ADXL345_DEVID 0x00
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_DATAX0 0x32
#define ADXL345_DATAY0 0x34
#define ADXL345_DATAZ0 0x36
#define ADXL345_BW_RATE 0x2C
#define SDA_PIN 21
#define SCL_PIN 22

unsigned long lastCycle = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(TOUCH_1, INPUT);
  pinMode(TOUCH_2, INPUT);
  pinMode(TOUCH_3, INPUT);
  pinMode(TOUCH_4, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  initADXL345();
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = (now - lastCycle) % 6000;

  if (elapsed < 2000) {
    if (digitalRead(TOUCH_1)) Serial.println("Touchpad 1 Pressed!");
    if (digitalRead(TOUCH_2)) Serial.println("Touchpad 2 Pressed!");
    if (digitalRead(TOUCH_3)) Serial.println("Touchpad 3 Pressed!");
    if (digitalRead(TOUCH_4)) Serial.println("Touchpad 4 Pressed!");
    delay(100);
  } else if (elapsed < 4000) {
    int rawValue = analogRead(GSR_PIN);
    Serial.print("GSR Raw Value: ");
    Serial.println(rawValue);
    delay(500);
  } else {
    int16_t x, y, z;
    readAcceleration(&x, &y, &z);

    Serial.print("Raw Values - X: "); Serial.print(x);
    Serial.print(", Y: "); Serial.print(y);
    Serial.print(", Z: "); Serial.println(z);

    float xg = x * 0.0078;
    float yg = y * 0.0078;
    float zg = z * 0.0078;

    Serial.print("X: "); Serial.print(xg); Serial.print(" g, ");
    Serial.print("Y: "); Serial.print(yg); Serial.print(" g, ");
    Serial.print("Z: "); Serial.print(zg); Serial.println(" g");
    delay(500);
  }

  if (now - lastCycle > 6000) lastCycle = now;
}

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
  if (Wire.available()) return Wire.read();
  return 0;
}

void readAcceleration(int16_t *x, int16_t *y, int16_t *z) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(ADXL345_DATAX0);
  Wire.endTransmission();
  Wire.requestFrom(ADXL345_ADDR, 6);
  if (Wire.available() < 6) {
    *x = *y = *z = 0;
    return;
  }
  *x = Wire.read(); *x |= Wire.read() << 8;
  *y = Wire.read(); *y |= Wire.read() << 8;
  *z = Wire.read(); *z |= Wire.read() << 8;
}
