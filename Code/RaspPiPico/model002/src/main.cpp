#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>
#include <EEPROM.h>

//======================
// INPUTS AND VARIABLES
//======================
const int MOT_IO = 17;
const int LED_IO = 18;
const int BTN_IO = 28;
const int FLEX_IO = 26; // ADC0

const int DIST_THRESH = 100;
const int16_t CHANGE_THRESH = 196;

const int FLEX_ON_THRESH  = 250; // at or above = hand extended
const int FLEX_OFF_THRESH = 220; // at or below = hand curled

const int MOT_MIN_PWM = 100;
const int MOT_MAX_PWM = 255;

const unsigned long SENSOR_INTERVAL_MS = 50;   // how often to poll sensors
const unsigned long DEBOUNCE_MS        = 200;  // button debounce window
const float DISTANCE_SMOOTHING_ALPHA   = 0.35; // 0=very smooth/slow, 1=raw/instant
const float FLEX_SMOOTHING_ALPHA       = 0.35;

//======================
//       GLOBALS
//======================
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X();

int mode = 1; // 1 = navigation, 2 = close-range
float smoothedDistanceCM = -1;      // -1 means "no valid reading yet"
float smoothedFlex = -1;
bool handExtended = false;

unsigned long lastSensRead = 0;
unsigned long lastDebounceTime = 0;
int16_t distance = 0;
int16_t lastDistance = -1;

int lastButtonReading = HIGH; // pull=up idle state
int lastButtonState = HIGH; 

//---------------------------------------------------------------
// Haptic feedback patterns
// Kept short/blocking since these only fire on discrete events
// (boot, mode switch), not during normal obstacle sensing.
//---------------------------------------------------------------
void buzz(int durationMs, int intensity = MOT_MAX_PWM) {
  analogWrite(MOT_IO, intensity);
  delay(durationMs);
  analogWrite(MOT_IO, 0);
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

//======================
//     FLEX SENSOR
//======================
void flexLogic() {
  int raw = analogRead(FLEX_IO);

  smoothedFlex = (smoothedFlex < 0)
    ? raw
    : (FLEX_SMOOTHING_ALPHA * raw + (1 - FLEX_SMOOTHING_ALPHA) * smoothedFlex);

  if (smoothedFlex >= FLEX_ON_THRESH) handExtended = true;
  else if (smoothedFlex <= FLEX_OFF_THRESH) handExtended = false;

  Serial.print("FLEX,");
  Serial.print(raw);
  Serial.print(",");
  Serial.print(smoothedFlex, 1);
  Serial.print(",");
  Serial.println(handExtended ? 1 : 0);
}

//======================
//  MODE SWITCH LOGIC
//======================
void modeSwitch() {
  int reading = digitalRead(BTN_IO);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
    lastButtonReading = reading;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS && reading != lastButtonState) {
    lastButtonState = reading;
    if (lastButtonState == LOW) {
      mode = (mode == 1) ? 2 : 1;
      Serial.print("Mode changed to: ");
      Serial.println(mode);
      EEPROM.write(0, mode);
      EEPROM.commit();
      announceMode();
    }
  }
}
//======================
//     INIT VL53L1X
//======================
void initVL53L1X() {
  Serial.println(F("Initializing VL53L1X..."));
  if (!vl53.begin(0x29, &Wire)) {
    Serial.print(F("Error on initializing VL53L1X sensor: "));
    Serial.println(vl53.vl_status);
    while (1) { announceError(); delay(2000); }
  }
  Serial.println(F("VL53L1X OK!"));
  
  if (!vl53.startRanging()) {
    Serial.print(F("Couldn't start ranging: "));
    Serial.println(vl53.vl_status);
    while (1) { announceError(); delay(2000); }
  }
  
  // Timing budget options: 15, 20, 33, 50, 100, 200 ms
  vl53.setTimingBudget(SENSOR_INTERVAL_MS);
}

//======================
//    VL53L1X LOGIC1
//======================
void VL53L1XLogic1() {

  if (distance != -1) {
    if (lastDistance != -1) {
      int16_t difference = abs(distance - lastDistance);
      if (difference > CHANGE_THRESH) {
        buzz(10, 255);
      }
    }
    lastDistance = distance; // Update state for next calculation
  }
}

//======================
//    VL53L1X LOGIC2
//======================
void VL53L1XLogic2() {
  if (distance < 0) return;

  Serial.println(F("RUNNING VL53L1X LOGIC FOR MODE 2"));

  int difference = DIST_THRESH - distance;

  // Closer than threshold
  if (difference > 0) {
    // Clamp difference to a max limit so mapping stays within expected bounds
    int constrainedDiff = constrain(difference, 0, DIST_THRESH);

    // As distance gets closer (difference increases):
    // - count increases (e.g., 1 to 5 pulses)
    // - duration increases (e.g., 50ms to 200ms per pulse)
    // - gap decreases (e.g., 200ms down to 30ms between pulses for higher frequency)
    int count = map(constrainedDiff, 0, DIST_THRESH, 1, 5);
    int duration = map(constrainedDiff, 0, DIST_THRESH, 50, 200);
    int gap = map(constrainedDiff, 0, DIST_THRESH, 200, 30);

    buzzPattern(count, duration, gap);
  }

}

//======================
//        SETUP
//======================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  // Set Pin-Modes
  pinMode(MOT_IO, OUTPUT);
  pinMode(LED_IO, OUTPUT);
  pinMode(BTN_IO, INPUT_PULLUP);
  pinMode(FLEX_IO, INPUT);

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin(); // Initialize I2C communication

  // Initialize DISTSENS
  initVL53L1X();

  // Restore last-used mode so the wearer doesn't have to re-select it
  // every time the device powers on.
  EEPROM.begin(256);
  mode = EEPROM.read(0);
  if (mode != 1 && mode != 2) mode = 1;   // blank flash reads as 255

  announceReady();
}

//======================
//        LOOP
//======================
void loop() {
  // Check Mode
  modeSwitch();

  // Poll flex sensor on the sensor interval
  if (millis() - lastSensRead >= SENSOR_INTERVAL_MS) {
    lastSensRead = millis();
    flexLogic();
  }

  // Poll distance sensor when data is available
  if (vl53.dataReady()) {
    distance = vl53.distance();
    vl53.clearInterrupt();

    // Hand curled, stay silent
    if (!handExtended) {
      lastDistance = -1;   // forget old reading
      return;
    }
    // Mode 1
    if (mode == 1) {
      VL53L1XLogic1();
    // Mode 2
    } else if (mode == 2) {
      VL53L1XLogic2();
    }
  }
}
