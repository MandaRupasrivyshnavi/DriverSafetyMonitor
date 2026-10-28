#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =============================
// DRIVER SAFETY MONITORING
// =============================

// Inputs
#define EYE_PIN       34
#define MQ2_PIN       35
#define MOBILE_PIN    32
#define BUCKLE_PIN    27
#define BELT_POS_PIN  33

// Outputs
#define BUZZER_PIN    25
#define RED_LED       26
#define GREEN_LED     14

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// -----------------------------
// Thresholds
// -----------------------------

#define EYE_THRESHOLD    2800
#define SMOKE_THRESHOLD  1800

const unsigned long DROWSY_TIME = 2000;

unsigned long eyeClosedStart = 0;

bool drowsy = false;

// =============================
// SETUP
// =============================

void setup() {

  Serial.begin(115200);

  pinMode(EYE_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);

  pinMode(MOBILE_PIN, INPUT_PULLUP);
  pinMode(BUCKLE_PIN, INPUT_PULLUP);
  pinMode(BELT_POS_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  Wire.begin(21, 22);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C)) {

    Serial.println("OLED failed!");

    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(15, 15);
  display.println("DRIVER SAFETY");

  display.setCursor(25, 30);
  display.println("MONITORING");

  display.display();

  delay(2000);
}

// =============================
// MAIN LOOP
// =============================

void loop() {

  // ---------------------------
  // READ SENSORS
  // ---------------------------

  int eyeValue = analogRead(EYE_PIN);

  int smokeValue = analogRead(MQ2_PIN);

  bool mobileDetected =
      digitalRead(MOBILE_PIN) == LOW;

  bool buckleConnected =
      digitalRead(BUCKLE_PIN) == LOW;

  bool beltAcrossChest =
      digitalRead(BELT_POS_PIN) == LOW;


  // ===========================
  // DROWSINESS DETECTION
  // ===========================

  if (eyeValue > EYE_THRESHOLD) {

    if (eyeClosedStart == 0) {
      eyeClosedStart = millis();
    }

    if (millis() - eyeClosedStart >= DROWSY_TIME) {
      drowsy = true;
    }

  } else {

    eyeClosedStart = 0;
    drowsy = false;
  }


  // ===========================
  // SMOKE DETECTION
  // ===========================

  bool smokeDetected =
      smokeValue > SMOKE_THRESHOLD;


  // ===========================
  // SEAT BELT DETECTION
  // ===========================

  bool beltCorrect =
      buckleConnected && beltAcrossChest;

  bool beltBehindDriver =
      buckleConnected && !beltAcrossChest;


  // ===========================
  // CHECK FOR DANGER
  // ===========================

  bool danger =
      drowsy ||
      smokeDetected ||
      mobileDetected ||
      !beltCorrect;


  // ===========================
  // LED + BUZZER
  // ===========================

  if (danger) {

    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    tone(BUZZER_PIN, 1000);

  } else {

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);

    noTone(BUZZER_PIN);
  }


  // ===========================
  // OLED DISPLAY
  // ===========================

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);

  display.println("DRIVER MONITOR");


  // EYE
  display.setCursor(0, 14);
  display.print("EYE: ");

  if (drowsy) {
    display.println("DROWSY!");
  } else {
    display.println("NORMAL");
  }


  // SMOKE
  display.setCursor(0, 24);
  display.print("SMOKE: ");

  if (smokeDetected) {
    display.println("DETECTED!");
  } else {
    display.println("NORMAL");
  }


  // MOBILE
  display.setCursor(0, 34);
  display.print("MOBILE: ");

  if (mobileDetected) {
    display.println("USING!");
  } else {
    display.println("NO");
  }


  // SEAT BELT
  display.setCursor(0, 44);
  display.print("BELT: ");

  if (beltCorrect) {

    display.println("OK");

  } else if (beltBehindDriver) {

    display.println("WRONG!");

  } else {

    display.println("NOT WORN");
  }


  // Overall condition
  display.setCursor(0, 55);

  if (danger) {
    display.println("!! WARNING !!");
  } else {
    display.println("STATUS: SAFE");
  }

  display.display();


  // ===========================
  // SERIAL MONITOR
  // ===========================

  Serial.print("Eye=");
  Serial.print(eyeValue);

  Serial.print(" Smoke=");
  Serial.print(smokeValue);

  Serial.print(" Mobile=");
  Serial.print(mobileDetected);

  Serial.print(" Buckle=");
  Serial.print(buckleConnected);

  Serial.print(" Chest=");
  Serial.print(beltAcrossChest);

  Serial.print(" Drowsy=");
  Serial.println(drowsy);


  delay(100);
}