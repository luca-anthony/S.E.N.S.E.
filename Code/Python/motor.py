"""
S.E.N.S.E. -- motor.py

Drives the coin vibration motor via the 2N2222 transistor on config.MOT_PIN
using PWM (gpiozero + pigpio backend, for clean hardware PWM on the Pi).

Vibration "intensity" is a 0.0-1.0 value from the caller. We scale it into
the [MOT_MIN, MOT_MAX] range from config.py so weak motors still reliably
kick on instead of getting a PWM duty cycle too small to feel.
"""

from gpiozero import PWMOutputDevice
from gpiozero.pins.pigpio import PiGPIOFactory

import config


class Motor:
    def __init__(self):
        # pigpio gives smoother/quieter PWM than the software fallback --
        # requires the pigpiod service, which setup.sh already enables.
        factory = PiGPIOFactory()
        self._motor = PWMOutputDevice(config.MOT_PIN, pin_factory=factory)

    def set_intensity(self, intensity):
        """
        intensity: 0.0-1.0, where 0.0 means "off" and 1.0 means max
        vibration. Anything above 0 gets floored to config.MOT_MIN so it's
        actually felt, and everything is capped at config.MOT_MAX.
        """
        intensity = max(0.0, min(1.0, intensity))

        if intensity <= 0:
            self._motor.value = 0
            return

        scaled = config.MOT_MIN + intensity * (config.MOT_MAX - config.MOT_MIN)
        self._motor.value = min(scaled, config.MOT_MAX)

    def off(self):
        self._motor.value = 0

    def close(self):
        self.off()
        self._motor.close()


if __name__ == "__main__":
    import time

    motor = Motor()
    try:
        # Quick ramp test
        for step in range(0, 11):
            motor.set_intensity(step / 10)
            time.sleep(0.2)
        motor.off()
    finally:
        motor.close()
