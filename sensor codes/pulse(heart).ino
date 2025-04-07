const int pulsePin = 34;
volatile int pulseCount = 0;
volatile unsigned long lastPulseTime = 0;

unsigned long lastCalcTime = 0;
unsigned long interval = 10000; // Check every 10 seconds for smoother BPM
int bpm = 0;

void IRAM_ATTR onPulse() {
  unsigned long currentTime = millis();

  // Debounce: only count if pulse detected after 300ms
  if (currentTime - lastPulseTime > 300) {
    pulseCount++;
    lastPulseTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pulsePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsePin), onPulse, RISING);
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastCalcTime >= interval) {
    bpm = (pulseCount * 60000) / interval; // BPM formula: (pulses / time) * 60,000 ms
    pulseCount = 0;
    lastCalcTime = currentTime;

    // Display or filter
    if (bpm >= 40 && bpm <= 180) {
      Serial.print("Heart Rate (BPM): ");
      Serial.println(bpm);
    } else {
      Serial.println("Unrealistic BPM - Check sensor or connection");
    }
  }
}
