"""
S.E.N.S.E. -- flex_sensor.py

Reads the flex sensor through the MCP3008 (Pi has no analog inputs of its
own, hence the ADC). Wired over hardware SPI0 -- see config.py for pinout.

Exposes a single 0.0-1.0 "flatness" reading, plus a helper that compares
it against config.FLEX_THRESH so main.py doesn't need to know the raw
ADC scale.
"""

import spidev

import config

MCP3008_MAX_VALUE = 1023  # 10-bit ADC, so readings range 0-1023


class FlexSensor:
    def __init__(self, channel=config.FLEX_CHANNEL, bus=0, device=0):
        if not 0 <= channel <= 7:
            raise ValueError("MCP3008 channel must be 0-7")

        self._channel = channel
        self._spi = spidev.SpiDev()
        self._spi.open(bus, device)
        self._spi.max_speed_hz = 1_350_000  # safe default for the MCP3008

    def _read_raw(self):
        """Single-ended read from the MCP3008, per its standard 3-byte
        command protocol. Returns 0-1023."""
        cmd = [1, (8 + self._channel) << 4, 0]
        reply = self._spi.xfer2(cmd)
        return ((reply[1] & 3) << 8) + reply[2]

    def read_normalized(self):
        """Returns the current flex reading scaled to 0.0-1.0."""
        raw = self._read_raw()
        return raw / MCP3008_MAX_VALUE

    def is_hand_flat(self):
        """True when the hand is extended flat enough to arm the system,
        per config.FLEX_THRESH."""
        return self.read_normalized() >= config.FLEX_THRESH

    def close(self):
        self._spi.close()


if __name__ == "__main__":
    import time

    flex = FlexSensor()
    try:
        while True:
            val = flex.read_normalized()
            print(f"Flex: {val:.2f}  (flat={flex.is_hand_flat()})")
            time.sleep(config.SENS_INTERVALS)
    except KeyboardInterrupt:
        flex.close()
