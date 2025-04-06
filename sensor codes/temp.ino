const int tempPin = 34; // LM35DZ connected to GPIO34 (ADC1)

//// onlyyyy 34 oin works(16 doesn't, others like 35 n all idk)

void setup() {
    Serial.begin(115200);
    analogReadResolution(12); // Set ADC resolution to 12-bit (0-4095)
}

void loop() {
    int sensorValue = analogRead(tempPin); 

    // Convert ADC value to voltage (3.3V reference)
    float voltage = sensorValue * (3.3 / 4095.0);

    // Convert voltage to temperature
    float temperature = voltage * 100.0;

    // Calibration adjustment for accurate skin temperature
    float adjustedTemp = temperature + 2.0;

    Serial.print("Temperature: ");
    Serial.print(adjustedTemp);
    Serial.println(" °C");

    delay(1000); // 1-second delay for stable readings
}
