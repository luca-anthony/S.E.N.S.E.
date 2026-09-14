// PIN DEFS
const int FLEX_PIN = A0; // Flex Sensor analog input
const int MOT_PIN = 9; // PWM output to motor transistor
const int FLEX_THRESH = 500; // Would need to be calibrated based on fully open hand reading
const int DIST_THRESH_CM = 80; // Trigger distance in cm (convert to in with math if you want to americans)

void setup() {
  pinMode(FLEX_PIN, INPUT);
  pinMode(MOT_PIN, OUTPUT);
  // INIT DISTANCE SENSOR HERE
}

void loop() {
  int flexVal = analogRead(FLEX_PIN);

  // Hand flat
  if (flexVal > FLEX_THRESH) {
    int distance = getDistanceCM(); // Call distance sensor func

    if (distance < DIST_THRESH_CM && distance > 0) {

      // Scale motor intensity (closer object is = stronger pulse)
      int intensity = map(distance, DIST_THRESH_CM, 10, 100, 255);
      analogWrite(VIBE_PIN, constrain(intensity, 0, 255));
    } else {
      analogWrite(MOT_PIN, 0);
    }
  }

  // Hand curled/resting
  else {
    analogWrite(MOT_PIN, 0); // Disable system to prevent false alerts
  }

  delay(50); // delay. used in every program. (but 50ms is better for motor)
}
