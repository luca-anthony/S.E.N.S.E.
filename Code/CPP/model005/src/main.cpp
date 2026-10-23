#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>

// Inputs and Variables
const int motPin = 4;
const int btnPin = 3;
const int flexPin = 1; // ADC1
const int ledPin = 8;

const int sdaPin = 6;
const int sclPin = 7;

// NOTE: VL53L1X distance() returns millimetres, so these are in mm
const int distThresh = 100;       // 100 mm = 10 cm
const int16_t changeThresh = 196; // mm

const int motMinPWM = 100;
const int motMaxPWM = 255;

const unsigned long sensorIntervalMS = 50; // how often to poll sensors
const unsigned long debounceMS = 200; // button debounce window
const float distSmoothingA = 0.35; // 0=very smooth/slow, 1=raw/instant
const float flexSmoothingA = 0.35;

const int flexTolerance = 150;
const int calibSamples = 30; // number of readings averaged during calibration

// Globals
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X();

int mode = 1; // 1 = navigation, 2 = close-range

float smoothedDistanceCM = -1; // -1 means "no valid reading yet" (currently unused)
float smoothedFlex = -1;
bool handExtended = false;

unsigned long lastSensRead = 0;
unsigned long lastDebounceTime = 0;

int16_t distance = 0;
int16_t lastDistance = -1;

int lastButtonReading = HIGH; // pull-up idle state
int lastButtonState = HIGH;

int extensionValue = 0;

// Haptic Feedback Patterns

void buzz(int durationMS, int intens = motMaxPWM) {
  analogWrite(motPin, intens);
  delay(durationMS);
  analogWrite(motPin, 0);
}

void buzzPattern(int count, int durationMS = 120, int gapMS = 120) {
  for (int i = 0; i < count; i++) {
    buzz(durationMS);
    if (i < count - 1) delay(gapMS);
  }
}

// One buzz = mode 1, two buzzes = mode 2
void announceMode() {
  buzzPattern(mode);
}

// Buzz pattern on boot to indicate everything is working and ready
void bootBuzz() {
  buzzPattern(3, 100, 100);
}

// Announce error with a long buzz pattern
void announceError() {
  buzz(600);
}

// Mode Switch Logic
void modeSwitch() {
  int reading = digitalRead(btnPin);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
    lastButtonReading = reading;
  }

  if ((millis() - lastDebounceTime) > debounceMS && reading != lastButtonState) {
    lastButtonState = reading;
    if (lastButtonState == LOW) {
      mode = (mode == 1) ? 2 : 1;
      lastDistance = -1; // reset so a mode change doesn't trigger a false "change" buzz
      Serial.print("Mode changed to: ");
      Serial.println(mode);
      announceMode();
    }
  }
}

// Flex Sensor
void flexLogic() {
  int flexValue = analogRead(flexPin);
  Serial.print("Flex: ");
  Serial.println(flexValue);

  // Smooth the flex sensor reading
  smoothedFlex = (smoothedFlex < 0)
    ? flexValue
    : (flexSmoothingA * flexValue + (1 - flexSmoothingA) * smoothedFlex);

  // Determine if hand is extended (within tolerance of the calibrated value)
  handExtended = (abs(smoothedFlex - extensionValue) <= flexTolerance);
}

// Calibration
void flexCalibrate() {
  Serial.println("Calibrating flex sensor. Please extend your hand fully and hold for 3 seconds...");
  digitalWrite(ledPin, HIGH);
  buzzPattern(3, 100, 1000); // 3 buzzes with 1s gap
  delay(3000); // Wait for 3 seconds

  // Average several readings so one noisy sample doesn't ruin the calibration
  long sum = 0;
  for (int i = 0; i < calibSamples; i++) {
    sum += analogRead(flexPin);
    delay(10);
  }
  extensionValue = sum / calibSamples;

  smoothedFlex = extensionValue; // start the smoothing filter from the calibrated value

  Serial.print("Extension value: ");
  Serial.println(extensionValue);
  Serial.println("Extension Value Saved!");
  Serial.println("Calibration Complete!");
  digitalWrite(ledPin, LOW); // calibration finished, turn LED off
  buzzPattern(3, 30, 50);
  buzz(400, 255);
}

// VL53L1X

// Initialize the VL53L1X sensor
void initVL53L1X() {
  Serial.println("Initializing VL53L1X...");
  if (!vl53.begin()) {
    Serial.println("Failed to initialize VL53L1X sensor!");
    while (1) { announceError(); delay(2000); }
  }
  Serial.println("VL53L1X initialized successfully.");
  if (!vl53.startRanging()) {
    Serial.println("Failed to start ranging!");
    while (1) { announceError(); delay(2000); }
  }
}

// Mode One
void tofLogic1() {
  if (distance != -1) {
    if (lastDistance != -1) {
      int16_t difference = abs(distance - lastDistance);
      if (difference > changeThresh) {
        buzz(10, 255);
      }
    }
    lastDistance = distance;
  }
}

// Mode Two
void tofLogic2() {
  if (distance < 0) return;

  int difference = distThresh - distance;

  // Closer than thresh
  if (difference > 0) {
    // Max difference
    int constrainedDiff = constrain(difference, 0, distThresh);

    // As distance gets closer (difference increases):
    // - count increases (e.g., 1 to 5 pulses)
    // - duration increases (e.g., 50ms to 200ms per pulse)
    // - gap decreases (e.g., 200ms down to 30ms between pulses for higher frequency)
    int count = map(constrainedDiff, 0, distThresh, 1, 5);
    int duration = map(constrainedDiff, 0, distThresh, 50, 200);
    int gap = map(constrainedDiff, 0, distThresh, 200, 30);

    buzzPattern(count, duration, gap);
  }
}

// Setup
void setup() {
  Serial.begin(115200);

  // Pin Modes
  pinMode(motPin, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  analogReadResolution(12); // 12-bit ADC

  // Initialize I2C
  Wire.begin(sdaPin, sclPin);

  // Initialize VL53L1X
  initVL53L1X();

  bootBuzz(); // Indicate system is ready

  // Calibrate Flex Sensor
  flexCalibrate();
  Serial.println("Setup complete. Entering main loop.");
}

// Loop
void loop() {
  // Always monitor mode button
  modeSwitch();

  // Non-blocking sensor polling interval
  if (millis() - lastSensRead >= sensorIntervalMS) {
    lastSensRead = millis();

    // Poll flex sensor
    flexLogic();

    // Poll distance sensor when data is available
    if (vl53.dataReady()) {
      distance = vl53.distance();
      vl53.clearInterrupt();

      // Only execute haptic feedback logic if hand is extended
      if (handExtended) {
        if (mode == 1) {
          tofLogic1();
        } else if (mode == 2) {
          tofLogic2();
        }
      } else {
        lastDistance = distance;
      }
    }
  }
}