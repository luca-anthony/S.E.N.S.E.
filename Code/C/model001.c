// PIN DEFS
const int FLEX_PIN = A0; // Flex Sensor anlg input
const int MOT_PIN = 9; // PWM output to motor transistor
const int FLEX_THRESH = 500; // Needs to be calibrated based on readings
const int DIST_THRESH_CM1 = 80; // Trigger dist
const int DIST_THRESH_CM2 = 4; // Trigger dist 2
const int BTN_PIN = 1; // Mode switch button

void setup() {

  Serial.begin(115200); // Start seial at 115200 baud rate
  
  pinMode(FLEX_PIN, INPUT);
  pinMode(MOT_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  // INIT DISTANCE SENSOR HERE

  int mode = 1;
}

void loop() {
  int flexVal = analogRead(FLEX_PIN);

  int btnState = digitalRead(BTN_PIN);

  if (btnState == LOW) {
    // Detect Modes
    if (mode == 1) {
      mode = 2;
      Serial.print("Mode changed to 2");
    } else {
        mode = 1;
        Serial.print("Mode Changed to 1");
    }
    
    delay(200);
  }
  
  if (mode == 1) {
    // Hand flat
    if (flfexVal > FLEX_THRESH) {
      int distance = getDistanceCM(); // Call distance sensor func

      if (distance < DIST_THRESH_CM1 && distance > 0) {

        // Scale motor intensity
        int intensity = map(distance, DIST_THRESH_CM1, 10, 100, 255);
        analogWrite(VIBE_PIN, constrain(intensity, 0, 255));
      }  else {
            analogWrite(MOT_PIN, 0);
      }
    }

    // Hand curled/resting
    else {
      analogWrite(MOT_PIN, 0); // Disable system to prevent false alerts
    }

    delay(50); // delay. used in every program. (but 50ms is better for motor)
  }

  if (mode == 2) {
    // Hand Flat
    if (flexVal > FLEX_THRESH) {
      int distance = getDistanceCM(); // Call distance sensor func

      if (distance < DIST_THRESH_CM2 && distance >0) {

        // Scale motor intensity
        int intensity = map(distance, DIST_THRESH_CM2, 10, 100, 255);
        analogWrite(VIBE_PIN, constrain(intensity, 0, 255));
      } else {
        analogWrite(MOT_PIN, 0);
      }
    }

    // Hand curled/resting
    else {
      analogWrite(MOT_PIN, 0); // Disable system to prevent false alerts
    }

    delay(50); // delay
  }
       
}
