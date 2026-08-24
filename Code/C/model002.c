// PIN DEFS
const int FLEX_PIN = A0; // Flex Sensor anlg input
const int MOT_PIN = 9; // PWM output to motor transistor
const int FLEX_THRESH = 500; // Needs to be calibrated based on readings
const int DIST_THRESH_CM1 = 80; // Trigger dist
const int DIST_THRESH_CM2 = 4; // Trigger dist 2
const int BTN_PIN = 2; // Mode switch button

int mode = 1; // Set base mode (mode can be changed with mode switch)

void setup() {

  Serial.begin(115200); // Start seial at 115200 baud rate
  
  pinMode(FLEX_PIN, INPUT);
  pinMode(MOT_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  // INIT DISTANCE SENSOR HERE
}

void modeSwitch() {

int btnState = digitalRead(BTN_PIN)
  
  // Func for mode switching
  if (btnstate == LOW) {
    // Detect Mode
    if (mode == 1) {
      mode = 2;
      Serial.print("Mode changed to 2");
    } else {
        mode = 1;
        Serial.print("Mode changed to 1");
    }

    delay(200);
  }
}

void loop() {
  int flexVal = analogRead(FLEX_PIN);
  int activeThresh = (mode == 1) ? DIST_THRESH_CM1 : DIST_THRESH_CM2;

  modeSwitch();
  
  
  // Hand flat
  if (flexVal > FLEX_THRESH) {
    int distance = getDistanceCM(); // Call distance sensor func

    if (distance < activeThres && distance > 0) {

      // Scale motor intensity
      int intensity = map(distance, activeThresh, 0, 100, 255);
      analogWrite(MOT_PIN, constrain(intensity, 0, 255));
    }  else {
          analogWrite(MOT_PIN, 0);
    }
  }

  // Hand curled/resting
  else {
    analogWrite(MOT_PIN, 0); // Disable system to prevent false alerts
  }

  delay(50); // delay. Used in every program. (but 50ms is better for motor)
}
