const int buzzerPin = 9; // Buzzer connected to digital pin 9

void setup() {
    pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output
}

void loop() {
    // Beep sound
    tone(buzzerPin, 1000); // 1000 Hz frequency
    delay(500);            // Beep for 500ms

    noTone(buzzerPin);     // Stop the buzzer
    delay(500);            // Pause for 500ms
}
