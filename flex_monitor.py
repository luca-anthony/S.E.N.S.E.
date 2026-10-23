"""
S.E.N.S.E. -- flex_monitor.py

Live view and calibration for the flex sensor, read from the Pico over USB.
Needs pyserial (pip install pyserial).

    python flex_monitor.py              live view
    python flex_monitor.py --calibrate  prints FLEX_ON_THRESH / FLEX_OFF_THRESH
    python flex_monitor.py --raw        prints everything the Pico sends
    python flex_monitor.py COM5         pick the port yourself
"""

import sys
import time

import serial
from serial.tools import list_ports

BAUD = 115200
CAL_SECONDS = 3
BAR_WIDTH = 40
PICO_VIDS = (0x2E8A, 0x2341)  # Raspberry Pi, Arduino (mbed core)


def find_port():
    for arg in sys.argv[1:]:
        if not arg.startswith("--"):
            return arg

    ports = list_ports.comports()
    picos = [p for p in ports if p.vid in PICO_VIDS]
    found = picos or ports
    if len(found) != 1:
        sys.exit("Couldn't pick a port. Run again with it, e.g. python flex_monitor.py COM5")
    return found[0].device


def read_flex(ser):
    """Returns (raw, smoothed, extended) from one FLEX line, or None."""
    line = ser.readline().decode(errors="ignore").strip()
    if not line.startswith("FLEX,"):
        return None
    try:
        _, raw, smoothed, extended = line.split(",")
        return int(raw), float(smoothed), extended == "1"
    except ValueError:
        return None


def raw(ser):
    while True:
        line = ser.readline().decode(errors="ignore").strip()
        if line:
            print(line)


def live(ser):
    lo, hi = 1023, 0
    while True:
        reading = read_flex(ser)
        if not reading:
            continue

        raw, smoothed, extended = reading
        lo, hi = min(lo, raw), max(hi, raw)
        pos = (smoothed - lo) / (hi - lo) if hi > lo else 0
        bar = "#" * round(max(0, min(1, pos)) * BAR_WIDTH)
        state = "EXTENDED" if extended else "curled  "

        print(f"\rraw {raw:4d}  smooth {smoothed:6.1f}  min {lo:4d}  max {hi:4d}  "
              f"{state} |{bar:<{BAR_WIDTH}}|", end="", flush=True)


def sample(ser, label):
    input(f"Hold your finger {label}, then press Enter...")
    ser.reset_input_buffer()

    values = []
    end = time.time() + CAL_SECONDS
    while time.time() < end:
        reading = read_flex(ser)
        if reading:
            values.append(reading[1])

    if not values:
        sys.exit("No FLEX data received.")
    return sum(values) / len(values)


def calibrate(ser):
    extended = sample(ser, "extended")
    curled = sample(ser, "curled")

    if extended <= curled:
        sys.exit("Extended reads lower than curled; the sensor is reversed or wired backwards.")

    mid = (extended + curled) / 2
    margin = (extended - curled) * 0.1

    print(f"\nextended {extended:.0f}, curled {curled:.0f}\n")
    print(f"const int FLEX_ON_THRESH  = {round(mid + margin)};")
    print(f"const int FLEX_OFF_THRESH = {round(mid - margin)};")


def main():
    ser = serial.Serial(find_port(), BAUD, timeout=1)
    try:
        if "--raw" in sys.argv:
            raw(ser)
        elif "--calibrate" in sys.argv:
            calibrate(ser)
        else:
            live(ser)
    except KeyboardInterrupt:
        print()
    finally:
        ser.close()


if __name__ == "__main__":
    main()