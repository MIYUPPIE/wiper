/*
  Automatic Rain-Sensing Windshield Wiper
  ----------------------------------------
  An analog rain/water sensor drives a servo-mounted wiper arm.

    - No rain       -> wiper parks at 0 deg
    - Rain detected -> wiper sweeps 0 <-> 180 deg continuously
    - Heavier rain  -> faster sweep (shorter delay per step)

  Hardware:
    - Arduino Uno / Nano
    - Analog rain/water sensor module -> A0  (+5V, GND)
    - Hobby servo (SG90 / MG90 / etc.) -> D9 (+5V, GND)

  Sensor polarity:
    This resistive sensor reads HIGHER as more water bridges its traces,
    so a larger reading means heavier rain. If yours reads LOWER when wet,
    flip the comparison in loop() and swap WIPE_DELAY_MIN/MAX.

  Calibration:
    Open the Serial Monitor at 9600 baud, watch the "Rain:" value dry vs
    wet, then set RAIN_THRESHOLD a little above the dry reading.
*/

#include <Servo.h>

const int SENSOR_PIN     = A0;   // analog rain sensor output
const int SERVO_PIN      = 9;    // servo signal pin

const int RAIN_THRESHOLD = 300;  // above this = rain present (calibrate)
const int WIPE_MIN_ANGLE = 0;    // parked position / start of stroke
const int WIPE_MAX_ANGLE = 180;  // end of stroke

const int WIPE_DELAY_MAX = 15;   // ms/deg in light rain (slow sweep)
const int WIPE_DELAY_MIN = 3;    // ms/deg in heavy rain (fast sweep)

Servo wiper;

void setup() {
  wiper.attach(SERVO_PIN);
  wiper.write(WIPE_MIN_ANGLE);   // start parked
  Serial.begin(9600);
}

void loop() {
  int rain = analogRead(SENSOR_PIN);

  // Heavier rain -> shorter delay -> faster sweep. Clamp to the window.
  int wipeDelay = map(rain, RAIN_THRESHOLD, 1023, WIPE_DELAY_MAX, WIPE_DELAY_MIN);
  wipeDelay = constrain(wipeDelay, WIPE_DELAY_MIN, WIPE_DELAY_MAX);

  Serial.print("Rain: ");
  Serial.print(rain);

  if (rain > RAIN_THRESHOLD) {
    Serial.print(" | Wiping at ");
    Serial.print(wipeDelay);
    Serial.println(" ms/deg");
    sweep(WIPE_MIN_ANGLE, WIPE_MAX_ANGLE, wipeDelay);  // forward stroke
    sweep(WIPE_MAX_ANGLE, WIPE_MIN_ANGLE, wipeDelay);  // return stroke
  } else {
    Serial.println(" | Dry - wiper parked");
    wiper.write(WIPE_MIN_ANGLE);
    delay(200);
  }
}

// Move the wiper between two angles, re-checking rain each step so it can
// stop and park mid-stroke the moment the rain stops.
void sweep(int from, int to, int stepDelay) {
  int step = (from <= to) ? 1 : -1;
  for (int angle = from; angle != to + step; angle += step) {
    if (analogRead(SENSOR_PIN) <= RAIN_THRESHOLD) {  // rain stopped
      wiper.write(WIPE_MIN_ANGLE);
      return;
    }
    wiper.write(angle);
    delay(stepDelay);
  }
}
