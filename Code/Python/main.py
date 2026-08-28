"""
S.E.N.S.E. -- main.py

Ties the modules together. Follows the flow in Notes/model.txt:

    [Read Flex Sensor] -- Is Hand Flat?
        NO  -> Motors Off
        YES -> [Read Distance Sensor] -- Is Object < Threshold?
            NO  -> Motors Off
            YES -> Motors On (vibration, scaled by how close the object is)

Threshold and vibration intensity depend on the active mode (nav vs.
close-range), handled by ModeManager. See config.py for pins/thresholds.
"""

import time

import config
from distance_sensor import DistanceSensor
from flex_sensor import FlexSensor
from motor import Motor
from mode_button import ModeManager


def announce_mode(motor, mode):
    """One quick buzz for mode 1, two for mode 2 -- lets the wearer feel
    which mode they're in without needing sight or sound."""
    for i in range(mode):
        motor.set_intensity(1.0)
        time.sleep(0.12)
        motor.off()
        if i < mode - 1:
            time.sleep(0.12)


def distance_to_intensity(distance_cm, threshold_cm):
    """
    Maps a distance reading to a 0.0-1.0 vibration intensity: right at the
    threshold is barely-on, and it ramps up to max as the object gets
    closer (down to 0 cm).
    """
    if distance_cm <= 0 or distance_cm >= threshold_cm:
        return 0.0

    return 1.0 - (distance_cm / threshold_cm)


def main():
    distance_sensor = DistanceSensor()
    flex_sensor = FlexSensor()
    motor = Motor()
    modes = ModeManager(on_change=lambda new_mode: announce_mode(motor, new_mode))

    print(f"S.E.N.S.E. running. Starting mode: {modes.mode}")

    try:
        while True:
            if not flex_sensor.is_hand_flat():
                # Hand curled/resting -- system idle, prevent false alerts
                motor.off()
                distance_sensor.reset_filter()
                time.sleep(config.SENS_INTERVALS)
                continue

            # Hand flat -- check for nearby obstacles
            distance_cm = distance_sensor.read_cm()
            threshold_cm = modes.active_threshold_cm

            if distance_cm > 0 and distance_cm < threshold_cm:
                intensity = distance_to_intensity(distance_cm, threshold_cm)
                motor.set_intensity(intensity)
            else:
                motor.off()

            time.sleep(config.SENS_INTERVALS)

    except KeyboardInterrupt:
        print("\nShutting down S.E.N.S.E....")

    finally:
        motor.close()
        flex_sensor.close()
        distance_sensor.stop()
        modes.close()


if __name__ == "__main__":
    main()
