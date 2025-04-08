#include <Wire.h>

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

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting ADXL345 Test...");
  Wire.begin(SDA_PIN, SCL_PIN);

  byte deviceID = readRegister(ADXL345_DEVID);
  Serial.print("ADXL345 Device ID: 0x");
  Serial.println(deviceID, HEX);

  if (deviceID != 0xE5) {
    Serial.println("ERROR: Device ID mismatch - check connections or I2C address!");
    Serial.println("Expected: 0xE5, Got: 0x" + String(deviceID, HEX));
    Serial.println("Trying to continue anyway...");
  }

  Serial.println("Initializing ADXL345...");
  initADXL345();
  Serial.println("ADXL345 initialization complete");
}

void loop() {
  Wire.begin(SDA_PIN, SCL_PIN); // Reinit I2C to avoid freeze after multiple reads

  int16_t x, y, z;
  readAcceleration(&x, &y, &z);

  Serial.print("Raw Values - X: ");
  Serial.print(x);
  Serial.print(", Y: ");
  Serial.print(y);
  Serial.print(", Z: ");
  Serial.println(z);

  float xg = x * 0.0078;
  float yg = y * 0.0078;
  float zg = z * 0.0078;

  Serial.print("X: ");
  Serial.print(xg);
  Serial.print(" g, Y: ");
  Serial.print(yg);
  Serial.print(" g, Z: ");
  Serial.print(zg);
  Serial.println(" g");

  delay(500);
}

void initADXL345() {
  Wire.beginTransmission(ADXL345_ADDR);
  byte error = Wire.endTransmission();

  if (error != 0) {
    Serial.println("ADXL345 not found! Check your connections.");
    Serial.println("I2C Error code: " + String(error));
    Serial.println("Trying to continue anyway...");
  }

  writeRegister(ADXL345_BW_RATE, 0x0A);
  writeRegister(ADXL345_DATA_FORMAT, 0x09);
  writeRegister(ADXL345_POWER_CTL, 0x08);

  byte rate = readRegister(ADXL345_BW_RATE);
  byte format = readRegister(ADXL345_DATA_FORMAT);
  byte power = readRegister(ADXL345_POWER_CTL);

  Serial.println("Verification - BW_RATE: 0x" + String(rate, HEX) +
                 ", DATA_FORMAT: 0x" + String(format, HEX) +
                 ", POWER_CTL: 0x" + String(power, HEX));
}

void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.write(value);
  byte error = Wire.endTransmission();
  if (error != 0) {
    Serial.println("Error writing to register 0x" + String(reg, HEX) +
                   ": Error code " + String(error));
  }
}

byte readRegister(byte reg) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  byte error = Wire.endTransmission();
  if (error != 0) {
    Serial.println("Error setting register address 0x" + String(reg, HEX) +
                   ": Error code " + String(error));
    return 0;
  }

  Wire.requestFrom(ADXL345_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  } else {
    Serial.println("No data available when reading register 0x" + String(reg, HEX));
    return 0;
  }
}

void readAcceleration(int16_t *x, int16_t *y, int16_t *z) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(ADXL345_DATAX0);
  byte error = Wire.endTransmission();
  if (error != 0) {
    Serial.println("Error setting data address: Error code " + String(error));
    *x = 0;
    *y = 0;
    *z = 0;
    return;
  }

  Wire.requestFrom(ADXL345_ADDR, 6);
  if (Wire.available() < 6) {
    Serial.println("Not enough data available. Expected 6 bytes, got " + String(Wire.available()));
    *x = 0;
    *y = 0;
    *z = 0;
    return;
  }

  *x = Wire.read();
  *x |= Wire.read() << 8;

  *y = Wire.read();
  *y |= Wire.read() << 8;

  *z = Wire.read();
  *z |= Wire.read() << 8;
}
