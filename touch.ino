#define TOUCH_1 25  // OUT1
#define TOUCH_2 26  // OUT2
#define TOUCH_3 27  // OUT3
#define TOUCH_4 14  // OUT4

void setup() {
    Serial.begin(115200);
    pinMode(TOUCH_1, INPUT);
    pinMode(TOUCH_2, INPUT);
    pinMode(TOUCH_3, INPUT);
    pinMode(TOUCH_4, INPUT);
}

void loop() {
    if (digitalRead(TOUCH_1)) {
        Serial.println("Touchpad 1 Pressed!");
    }
    if (digitalRead(TOUCH_2)) {
        Serial.println("Touchpad 2 Pressed!");
    }
    if (digitalRead(TOUCH_3)) {
        Serial.println("Touchpad 3 Pressed!");
    }
    if (digitalRead(TOUCH_4)) {
        Serial.println("Touchpad 4 Pressed!");
    }
    delay(100);
}