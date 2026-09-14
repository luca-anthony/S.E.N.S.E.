// ============================================================================
// S.E.N.S.E. — Spatial Echo Navigation & Sensing Equipment
// model004.cpp — builds on model003, fixes compile bugs + adds UX features
//
// Hardware: Seeed XIAO ESP32-C3, VL53L0X ToF distance sensor, flex sensor,
//           coin vibration motor (via NPN transistor), mode push button
// ============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Preferences.h>   

// ============================================================================
// INPUTS AND VARIABLES - Some need to be calibrated
// ============================================================================
const int FLEX_PIN         = A0;   // Flex sensor analog input
const int MOT_PIN          = 9;    // PWM output to motor transistor
const int BTN_PIN          = 2;    // Mode switch button (INPUT_PULLUP)

const int FLEX_THRESH      = 500;  // Hand-flat threshold — calibrate per user
const int DIST_THRESH_CM1  = 80;   // Mode 1: general navigation range
const int DIST_THRESH_CM2  = 4;    // Mode 2: close-object / grabbing range

const int MOTOR_MIN_PWM    = 100;  // Floor so weak motors reliably kick on
const int MOTOR_MAX_PWM    = 255;  // Max vibration intensity

const unsigned long SENSOR_INTERVAL_MS = 50;   // how often to poll sensors
const unsigned long DEBOUNCE_MS        = 200;  // button debounce window
const float DISTANCE_SMOOTHING_ALPHA   = 0.35; // 0=very smooth/slow, 1=raw/instant

// ============================================================================
// Globals
// ============================================================================
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
Preferences prefs;

int mode = 1;                       // 1 = navigation, 2 = close-range
float smoothedDistanceCM = -1;      // -1 means "no valid reading yet"

unsigned long lastSensorRead = 0;
unsigned long lastDebounceTime = 0;
int lastButtonReading = HIGH;       // pull-up idle state

// ============================================================================
// Haptic feedback patterns
// Kept short/blocking since these only fire on discrete events
// (boot, mode switch), not during normal obstacle sensing.
// ============================================================================
void buzz(int durationMs, int intensity = MOTOR_MAX_PWM) {
  analogWrite(MOT_PIN, intensity);
  delay(durationMs);
  analogWrite(MOT_PIN, 0);
}

void buzzPattern(int count, int durationMs = 120, int gapMs = 120) {
  for (int i = 0; i < count; i++) {
    buzz(durationMs);
    if (i < count - 1) delay(gapMs);
  }
}

// One buzz = mode 1 (navigation), two buzzes = mode 2 (close-range).
void announceMode() {
  buzzPattern(mode);
}

// Distinct longer pattern on boot so the wearer knows the device is alive
// and the distance sensor initialized correctly, without needing Serial.
void announceReady() {
  buzzPattern(3, 80, 80);
}

// A different, more urgent-feeling pattern to signal a hardware problem
// (sensor failed to initialize) so the wearer isn't left guessing why
// nothing is working.
void announceError() {
  buzz(600);
}

// ============================================================================
// Distance sensor read, in cm. Returns -1 if the reading is invalid/out of range.
// ============================================================================
int getDistanceCM() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus == 4) {
    // 4 == "out of range" per the Adafruit VL53L0X library
    return -1;
  }
  return measure.RangeMilliMeter / 10;
}

// ============================================================================
// Non-blocking button handling. Toggles mode on a clean press, debounced
// with millis() instead of delay() so it never freezes the sensing loop.
// ============================================================================
void handleModeButton() {
  int reading = digitalRead(BTN_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    // Button is pressed (active LOW) and this is a fresh state we haven't acted on
    if (reading == LOW && lastButtonReading == HIGH) {
      mode = (mode == 1) ? 2 : 1;
      Serial.print("Mode changed to ");
      Serial.println(mode);

      prefs.putUChar("mode", mode);  // remember it for next boot
      announceMode();                // and, more importantly, let the wearer feel it
    }
  }

  lastButtonReading = reading;
}

// ============================================================================
// Setup
// ============================================================================
void setup() {
  Serial.begin(115200);

  pinMode(FLEX_PIN, INPUT);
  pinMode(MOT_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Restore last-used mode so the wearer doesn't have to re-select it
  // every time the device powers on.
  prefs.begin("sense", false);
  mode = prefs.getUChar("mode", 1);

  Serial.println("Initializing VL53L0X...");
  if (!lox.begin()) {
    Serial.println("Failed to find VL53L0X sensor!");
    announceError();
    while (1) {
      delay(1000);
    }
  }
  Serial.println("VL53L0X Initialized Successfully!");

  announceReady();
}

// ============================================================================
// Main loop — sensing runs on a timed interval (non-blocking), button
// handling runs every pass so mode switches feel instant.
// ============================================================================
void loop() {
  handleModeButton();

  unsigned long now = millis();
  if (now - lastSensorRead < SENSOR_INTERVAL_MS) {
    return;  // keeps loop responsive to the button
  }
  lastSensorRead = now;

  int flexVal = analogRead(FLEX_PIN);
  int activeThresh = (mode == 1) ? DIST_THRESH_CM1 : DIST_THRESH_CM2;

  // Hand curled/resting — system off to prevent false alerts
  if (flexVal <= FLEX_THRESH) {
    analogWrite(MOT_PIN, 0);
    smoothedDistanceCM = -1;  // reset filter so it doesn't carry stale data
    return;
  }

  // Hand flat — read and smooth the distance
  int rawDistance = getDistanceCM();

  if (rawDistance > 0) {
    smoothedDistanceCM = (smoothedDistanceCM < 0)
      ? rawDistance
      : (DISTANCE_SMOOTHING_ALPHA * rawDistance
         + (1 - DISTANCE_SMOOTHING_ALPHA) * smoothedDistanceCM);
  } else {
    smoothedDistanceCM = -1;
  }

  if (smoothedDistanceCM > 0 && smoothedDistanceCM < activeThresh) {
    int intensity = map((int)smoothedDistanceCM, activeThresh, 0, MOTOR_MIN_PWM, MOTOR_MAX_PWM);
    analogWrite(MOT_PIN, constrain(intensity, 0, MOTOR_MAX_PWM));
  } else {
    analogWrite(MOT_PIN, 0);
  }
}
