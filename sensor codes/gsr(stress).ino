#define GSR_PIN 34

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  int rawValue = analogRead(GSR_PIN);

  Serial.print("GSR Raw Value: ");
  Serial.println(rawValue);

  delay(500);
}
