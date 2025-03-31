#include <Wire.h>
#include "MAX30105.h"  // Use SparkFun's MAX30105 library

MAX30105 particleSensor; // Correct object name for the library

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MAX30102...");

  // Initialize I2C (SDA=21, SCL=22 for most ESP32 boards)
  Wire.begin(19, 18); 

  // Initialize the sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { 
    Serial.println("MAX30102 not found. Check wiring!");
    while (1);  // Stop execution if sensor is not detected
  }

  Serial.println("MAX30102 initialized successfully!");

  // Configure sensor settings
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F); // Adjust LED brightness (6.4mA)
  particleSensor.setPulseAmplitudeIR(0x1F);
}

void loop() {
  Serial.print("RED: ");
  Serial.print(particleSensor.getRed());
  Serial.print("\t IR: ");
  Serial.println(particleSensor.getIR());

  delay(2000); // Small delay for stable readings
}
