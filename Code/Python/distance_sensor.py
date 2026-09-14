"""
S.E.N.S.E. -- distance_sensor.py

Wraps the VL53L1X Time-of-Flight sensor.

Handles:
  - I2C init
  - Pulling a raw reading in cm
  - Exponential smoothing (so the motor doesn't jitter on noisy readings)

Wiring / setup lives in config.py -- don't duplicate it here.
"""

import time

import board
import busio
import adafruit_vl53l1x

import config


class DistanceSensor:
    def __init__(self):
        i2c = busio.I2C(board.SCL, board.SDA)
        self._sensor = adafruit_vl53l1x.VL53L1X(i2c)

        # "Short" distance mode is plenty for this use case (indoor, < 4m)
        # and is less sensitive to ambient light than "long" mode.
        self._sensor.distance_mode = 1  # 1 = short, 2 = long
        self._sensor.timing_budget = 50  # ms, matches config.SENS_INTERVALS

        self._sensor.start_ranging()

        # -1 means "no valid reading yet" -- mirrors the C++ reference impl
        self._smoothed_cm = -1

    def _read_raw_cm(self):
        """
        Returns a single raw reading in cm, or -1 if the reading isn't
        valid (out of range / sensor not ready yet).
        """
        if not self._sensor.data_ready:
            return -1

        distance_cm = self._sensor.distance  # library reports cm as a float
        self._sensor.clear_interrupt()

        if distance_cm is None or distance_cm <= 0:
            return -1

        return distance_cm

    def read_cm(self):
        """
        Returns the smoothed distance in cm, or -1 if there's no valid
        reading. Call this once per poll loop -- it updates the internal
        filter each time.
        """
        raw = self._read_raw_cm()

        if raw < 0:
            self._smoothed_cm = -1
            return -1

        if self._smoothed_cm < 0:
            # First good reading after a gap -- snap straight to it instead
            # of smoothing from a stale/invalid value.
            self._smoothed_cm = raw
        else:
            alpha = config.DIST_SMOOTHING_ALPHA
            self._smoothed_cm = (alpha * raw) + ((1 - alpha) * self._smoothed_cm)

        return self._smoothed_cm

    def reset_filter(self):
        """Call when the hand curls / system goes idle so old readings
        don't leak into the next active session."""
        self._smoothed_cm = -1

    def stop(self):
        self._sensor.stop_ranging()


if __name__ == "__main__":
    # Quick manual test: run `python3 distance_sensor.py` and wave a hand
    # in front of the sensor.
    sensor = DistanceSensor()
    try:
        while True:
            print(f"Distance: {sensor.read_cm():.1f} cm")
            time.sleep(config.SENS_INTERVALS)
    except KeyboardInterrupt:
        sensor.stop()
