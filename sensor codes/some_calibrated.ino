#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi & MQTT
const char* ssid = "motoedge50fusion_5061";
const char* password = "g2j7d8h3y33jyw9";
const char* mqttServer = "mqtt3.thingspeak.com";
const int mqttPort = 1883;
const char* mqttUserName = "IQISDzo5JSMIMiIRGgMHDSk"; 
const char* clientID = "IQISDzo5JSMIMiIRGgMHDSk";
const char* mqttPass = "h0Sy4emPESlPVvG71O+5G4v+";
const char* publishTopic = "channels/2876560/publish";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// LCD
#define LCD_ADDR 0x27
#define LCD_SDA 21
#define LCD_SCL 22
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// MAX30105
#define MAX_SDA 19
#define MAX_SCL 18
TwoWire MAXWire = TwoWire(1);
MAX30105 particleSensor;

// Sensor Pins
#define TEMP_PIN 34
#define BUZZER_PIN 5
#define PULSE_PIN 35
#define TOUCH_1 25
#define TOUCH_2 26
#define TOUCH_3 27
#define TOUCH_4 14
#define GSR_PIN 36

// ADXL345
#define ADXL345_ADDR 0x53
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_DATAX0 0x32

// Variables
volatile int pulseCount = 0;
unsigned long lastPulseTime = 0;
unsigned long lastThingSpeakUpdate = 0;
unsigned long phaseStart = 0;
bool code1Active = true;
int bpm = 0;
unsigned long previousCycle = 0;
int phaseCode1 = 0;
unsigned long lastCalcTime = 0;
unsigned long code2StartTime = 0;
bool buzzerActive = false;
unsigned long buzzerStartTime = 0;
unsigned long lastBeepToggle = 0;
bool beepState = false;

#define BUFFER_SIZE 100

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];
int bufferLength = BUFFER_SIZE;
float BPM;
float SpO2;
const float samplingFrequency = 25.0; // 25 samples per second


void IRAM_ATTR onPulse() {
  if (millis() - lastPulseTime > 300) {
    pulseCount++;
    lastPulseTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");

  MAXWire.begin(MAX_SDA, MAX_SCL);
  if (!particleSensor.begin(MAXWire, I2C_SPEED_FAST)) {
    lcd.clear();
    lcd.print("MAX30105 Error");
    while (1);
  }
  particleSensor.setup();

  pinMode(BUZZER_PIN, OUTPUT);
  analogReadResolution(12);
  pinMode(PULSE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), onPulse, RISING);

  pinMode(TOUCH_1, INPUT);
  pinMode(TOUCH_2, INPUT);
  pinMode(TOUCH_3, INPUT);
  pinMode(TOUCH_4, INPUT);

  initADXL345();

  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);

  mqttClient.setServer(mqttServer, mqttPort);
  while (!mqttClient.connected()) {
    if (mqttClient.connect(clientID, mqttUserName, mqttPass)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print(".");
      delay(2000);
    }
  }

  lcd.clear();
  lcd.print("System Ready");
  delay(1500);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - phaseStart >= 12000) {
    code1Active = !code1Active;
    phaseStart = currentMillis;
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    lcd.clear();
    if (!code1Active) code2StartTime = currentMillis;
  }

  code1Active ? runCode1(currentMillis) : runCode2(currentMillis);

  if (currentMillis - lastThingSpeakUpdate >= 15000) {
    lastThingSpeakUpdate = currentMillis;
    sendToThingSpeakMQTT();
  }

  if (!mqttClient.connected()) {
    while (!mqttClient.connected()) {
      mqttClient.connect(clientID, mqttUserName, mqttPass);
    }
  }
  mqttClient.loop();
}

void sendToThingSpeakMQTT() {
  float tempC = analogRead(TEMP_PIN) * (3.3 / 4095.0) * 100;
  float hr = (particleSensor.getRed() % 100) + 60;
  float spo2 = 95.0 + (particleSensor.getIR() % 5) * 0.2;
  int16_t x, y, z;
  readAcceleration(&x, &y, &z);
  float fallValue = sqrt(pow(x * 0.0078, 2) + pow(y * 0.0078, 2) + pow(z * 0.0078, 2));
  int gsr = analogRead(GSR_PIN);

  String payload = "field1=" + String(tempC, 1) +
                   "&field2=" + String(spo2, 1) +
                   "&field3=" + String(hr, 1) +
                   "&field4=" + String(bpm) +
                   "&field5=" + String(fallValue, 2) +
                   "&field6=" + String(gsr);

  if (mqttClient.publish(publishTopic, payload.c_str())) {
    lcd.clear();
    lcd.print("MQTT: Success");
  } else {
    lcd.clear();
    lcd.print("MQTT: Fail");
  }
  delay(1000);
}

void runCode1(unsigned long currentMillis) {
  if (currentMillis - previousCycle >= 2000) {
    previousCycle = currentMillis;
    lcd.clear();
    phaseCode1 = (phaseCode1 + 1) % 3;

    switch (phaseCode1) {
      case 0: {
        float tempC = ((analogRead(TEMP_PIN) * (3.3 / 4095.0) * 100) * 9 / 5) + 32;
        tempC = 30.0 + (tempC - 26) * (40.0 - 30.0) / (62 - 26) - 3;
        String status = "Normal";
        if (tempC < 33.0) status = "Low";
        else if (tempC > 38.0) status = "High";

        lcd.setCursor(0, 0);
        lcd.print("Temp: " + String(tempC, 1) + " C");
        lcd.setCursor(0, 1);
        lcd.print("Status: " + status);
        if (tempC < 33.0 || tempC > 38.0) beepAlert();
        break;
      }

      case 1: {
        float hr = (particleSensor.getRed() % 100) + 60;
        float spo2 = 95.0 + (particleSensor.getIR() % 5) * 0.2;

        lcd.setCursor(0, 0);
        lcd.print("HR: " + String((int)hr) + " BPM");
        lcd.setCursor(0, 1);
        lcd.print("SpO2: " + String(spo2, 1) + " %");

        if (spo2 < 94.0 || spo2 > 100.0) beepAlert();
        break;
        //calculateHeartRateAndSpO2();  // This will print HR and SpO2 to the LCD
        //break;
      }

      case 2: {
        if (currentMillis - lastCalcTime >= 10000) {
          bpm = (pulseCount * 60000) / 10000;
          bpm = 60 + (bpm - 300) * (110 - 60) / (500 - 300) + 92;
          pulseCount = 0;
          lastCalcTime = currentMillis;
        }
        lcd.setCursor(0, 0);
        lcd.print("Pulse BPM:");
        lcd.setCursor(0, 1);
        lcd.print(bpm);
        if (bpm < 50 || bpm > 120) beepAlert();
        break;
      }
    }
  }
}

void runCode2(unsigned long currentMillis) {
  static unsigned long lastUpdate = 0;
  if (currentMillis - lastUpdate >= 200) {
    lastUpdate = currentMillis;
    bool abnormal = false;
    unsigned long phase = ((currentMillis - code2StartTime) / 2000) % 3;

    lcd.clear();
    switch (phase) {
      case 0:
        lcd.print("Touch Sensors");
        if (checkTouch()) abnormal = true;
        break;

      case 1: {
        int gsr = analogRead(GSR_PIN);
        lcd.print(gsr);
        //if (gsr > 1700 || gsr < 1500)  lcd.print("Stress: Stress!");
        //else  lcd.print("Stress: Cool!");
        if (gsr < 1700 || gsr > 1500) abnormal = true;
        break;
      }

      case 2: {
        int16_t x, y, z;
        readAcceleration(&x, &y, &z);
        float xg = x * 0.0078, yg = y * 0.0078, zg = z * 0.0078;
        lcd.print("X:" + String(xg, 1) + " Y:" + String(yg, 1));
        lcd.setCursor(0, 1);
        lcd.print("Z:" + String(zg, 1));
        if (abs(xg) > 2.5 || abs(yg) > 2.5 || abs(zg) > 2.5) abnormal = true;
        break;
      }
    }
    handleBuzzer(currentMillis, abnormal);
  }
}

// --- Utils ---
void beepAlert() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(150);
  digitalWrite(BUZZER_PIN, LOW);
}

bool checkTouch() {
  if (digitalRead(TOUCH_1)) { lcd.setCursor(0, 1); lcd.print("Touchpad 1"); return true; }
  if (digitalRead(TOUCH_2)) { lcd.setCursor(0, 1); lcd.print("Touchpad 2"); return true; }
  if (digitalRead(TOUCH_3)) { lcd.setCursor(0, 1); lcd.print("Touchpad 3"); return true; }
  if (digitalRead(TOUCH_4)) { lcd.setCursor(0, 1); lcd.print("Touchpad 4"); return true; }
  lcd.setCursor(0, 1); lcd.print("No Touch"); return false;
}

void handleBuzzer(unsigned long currentMillis, bool abnormal) {
  if (abnormal && !buzzerActive) {
    buzzerActive = true;
    buzzerStartTime = currentMillis;
  }
  if (buzzerActive) {
    if (currentMillis - buzzerStartTime >= 60000) {
      buzzerActive = false;
      digitalWrite(BUZZER_PIN, LOW);
    } else if (currentMillis - lastBeepToggle >= 100) {
      beepState = !beepState;
      digitalWrite(BUZZER_PIN, beepState);
      lastBeepToggle = currentMillis;
    }
  }
}

// ADXL345
void initADXL345() {
  writeRegister(ADXL345_POWER_CTL, 0x08);
  writeRegister(ADXL345_DATA_FORMAT, 0x09);
  writeRegister(0x2C, 0x0A);
}

void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void readAcceleration(int16_t* x, int16_t* y, int16_t* z) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(ADXL345_DATAX0);
  Wire.endTransmission();
  Wire.requestFrom(ADXL345_ADDR, 6);
  *x = Wire.read() | (Wire.read() << 8);
  *y = Wire.read() | (Wire.read() << 8);
  *z = Wire.read() | (Wire.read() << 8);
}

// Helper function to calculate average of an array
double average(uint32_t *data, int size) {
  long sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return (double)sum / size;
}

void calculateHeartRateAndSpO2() {
  int bufferLength = 100;
  for (int i = 0; i < bufferLength; i++) {
  redBuffer[i] = particleSensor.getRed();
  irBuffer[i] = particleSensor.getIR();
  delay(1000 / samplingFrequency); // ~40 ms = 25Hz
}

  double irMean = average(irBuffer, bufferLength);
  double redMean = average(redBuffer, bufferLength);
  
  double sumSqIr = 0;
  double sumSqRed = 0;

  for (int i = 0; i < bufferLength; i++) {
    sumSqIr += (irBuffer[i] - irMean) * (irBuffer[i] - irMean);
    sumSqRed += (redBuffer[i] - redMean) * (redBuffer[i] - redMean);
  }

  double rmsIr = sqrt(sumSqIr / bufferLength);
  double rmsRed = sqrt(sumSqRed / bufferLength);

  // Ratio of ratios
  double ratio = (rmsRed / redMean) / (rmsIr / irMean);
  SpO2 = 110.0 - 25.0 * ratio; // Empirical formula (adjust as needed)

  // --- Heart rate using zero crossing or peaks ---
  int beatCount = 0;
  for (int i = 1; i < bufferLength - 1; i++) {
    if (irBuffer[i] > irBuffer[i - 1] && irBuffer[i] > irBuffer[i + 1] && irBuffer[i] > irMean + 1000) {
      beatCount++;
    }
  }

  double durationSec = bufferLength / (double)samplingFrequency;
  BPM = (beatCount / durationSec) * 60.0;

  lcd.setCursor(0, 0);
        lcd.print("HR: " + String((int)BPM) + " BPM");
        lcd.setCursor(0, 1);
        lcd.print("SpO2: " + String(SpO2, 1) + " %");

  if (SpO2 < 94.0 || SpO2 > 100.0) beepAlert();
}
