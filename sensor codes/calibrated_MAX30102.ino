#include <Wire.h>
#include "MAX30105.h"  // Use SparkFun's MAX30105 library

MAX30105 particleSensor; // Correct object name for the library

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MAX30102...");

  // Initialize I2C (SDA=GPIO19, SCL=GPIO18 for most ESP32 boards)
  Wire.begin(19, 18);

  // Initialize the sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { 
    Serial.println("❌ MAX30102 not found! Check wiring.");
    while (1);  // Stop execution if sensor is not detected
  }

  Serial.println("✅ MAX30102 initialized successfully!");

  // Configure sensor settings
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F); // Adjust LED brightness (6.4mA)
  particleSensor.setPulseAmplitudeIR(0x1F);
}

void loop() {
  // Read raw sensor values
  long redValue = particleSensor.getRed();
  long irValue = particleSensor.getIR();

  // Display meaningful sensor data
  Serial.println("📡 MAX30102 Sensor Readings:");
  Serial.print("🔴 Red LED Absorption (Blood Flow): ");
  Serial.print(redValue);
  Serial.println(" units");

  Serial.print("🌙 Infrared LED Absorption (Oxygen Saturation): ");
  Serial.print(irValue);
  Serial.println(" units");

  // Simulated Heart Rate & SpO2 (for better accuracy, use an algorithm)
  float estimatedHeartRate = -0.000306*redValue+0.001224*irValue+57.09;  // Placeholder formula
  float estimatedSpO2 = -0.0636*(redValue/1000)-(0.1621*irValue)/10000+113.30; // Placeholder formula

  Serial.print("💓 Estimated Heart Rate: ");
  Serial.print(estimatedHeartRate);
  Serial.println(" BPM");

  Serial.print("🩸 Estimated SpO₂ Level: ");
  Serial.print(estimatedSpO2);
  Serial.println(" %");

  Serial.println("--------------------------------------------------");

  delay(4000); // Delay for better readability
}
