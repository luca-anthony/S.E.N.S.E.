#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>
#include <Preferences.h>
#include "SparkFunLSM6DS3.h"

//======================
// INPUTS AND VARIABLES
//======================
const int MOT_IO = 17;
const int LED_IO = 18;
const int BTN_IO = 19;

const int DIST_THRESH_CM1 = 80;
const int DIST_THRESH_CM2 = 4;

const int MOT_MIN_PWM = 100;
const int MOT_MAX_PWM = 255;

const unsigned long SENSOR_INTERVAL_MS = 50;   // how often to poll sensors
const unsigned long DEBOUNCE_MS        = 200;  // button debounce window
const float DISTANCE_SMOOTHING_ALPHA   = 0.35; // 0=very smooth/slow, 1=raw/instant

#define IRQ_PIN -1
#define XSHUT_PIN -1

#undef Wire
arduino::MbedI2C Wire(4, 5);

//======================
//       GLOBALS
//======================
VL53L1X lox;
Preferences prefs;

int mode = 1; // 1 = navigation, 2 = close-range
float smoothedDistanceCM = -1;      // -1 means "no valid reading yet"

unsigned long lastSensRead = 0;
unsigned long lastDebounceTime = 0;
int lastButtonReading = HIGH; // pull=up idle state

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
//      INIT IMUs
//======================
// Instantiate the IMU object. 
// 0x6A is the standard address for most breakout boards.
LSM6DS3 myIMU(I2C_MODE, 0x6A); 
void initIMU1() {
   // Call begin() to initialize the IMU registers
    if (myIMU.begin() != 0) {
        Serial.println("Device error! Check your wiring or I2C address.");
        while (1); // Halt if sensor not found
    }
    
    Serial.println("LSM6DS3TR-C Successfully Connected!");
}

void initIMU2() {
   // Call begin() to initialize the IMU registers
    if (myIMU.begin() != 0) {
        Serial.println("Device error! Check your wiring or I2C address.");
        while (1); // Halt if sensor not found
    }
    
    Serial.println("LSM6DS3TR-C Successfully Connected!");
}

//======================
//  MODE SWITCH LOGIC
//======================
void modeSwitch() {
  if 
}

//======================
//     INIT VL53L1X
//======================
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X(XSHUT_PIN, IRQ_PIN);
void initVL53L1X() {
  Serial.println(F("Initializing VL53L1X..."));
  if (!vl53.begin(0x29, &Wire)) {
    Serial.print(F("Error on initializing VL53L1X sensor: "));
    Serial.println(vl53.vl_status);
    while (1) delay(10);
  }
  Serial.println(F("VL53L1X OK!"));
  
  if (!vl53.startRanging()) {
    Serial.print(F("Couldn't start ranging: "));
    Serial.println(vl53.vl_status);
    while (1) delay(10);
  }
  
  // Timing budget options: 15, 20, 33, 50, 100, 200 ms
  vl53.setTimingBudget(SENSOR_INTERVAL_MS);
}

//======================
//        SETUP
//======================

void setup() {
  Serial.begin(115200);

  // Set Pin-Modes
  pinMode(MOT_IO, OUTPUT);
  pinMode(LED_IO, OUTPUT);
  pinMode(BTN_IO, INPUT_PULLUP);

  // Declare BTNState
  int btnState = digitalRead(BTN_IO);

  Wire.begin(); // Initialize I2C communication

  // Initialize DISTSENS' and IMUs
  initVL53L1X();
  initIMU1();
  initIMU2();

  // Restore last-used mode so the wearer doesn't have to re-select it
  // every time the device powers on.
  prefs.begin("sense", false);
  mode = prefs.getUChar("mode", 1);


}