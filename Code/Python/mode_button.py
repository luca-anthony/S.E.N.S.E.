"""
S.E.N.S.E. -- mode_button.py

Handles the mode switch button (added on top of the original build --
see config.py wiring notes).

Mode 1 = general navigation (larger threshold, config.DIST_THRESH_CM1)
Mode 2 = close-range / grabbing (smaller threshold, config.DIST_THRESH_CM2)

The current mode is written to config.MODE_FILE so it survives a reboot,
same idea as the `Preferences` use in the ESP32 (model004.cpp) version.
"""

import json
import os

from gpiozero import Button

import config


class ModeManager:
    def __init__(self, on_change=None):
        """
        on_change: optional callback fired with the new mode (1 or 2)
        whenever the button toggles it -- handy for main.py to trigger a
        haptic "mode changed" buzz.
        """
        self._on_change = on_change
        self._mode = self._load_mode()

        self._button = Button(
            config.BTN_PIN,
            pull_up=True,           # matches config.py's note: internal pull-up, no resistor
            bounce_time=config.DEBOUNCES,
        )
        self._button.when_pressed = self._toggle

    def _load_mode(self):
        if os.path.exists(config.MODE_FILE):
            try:
                with open(config.MODE_FILE, "r") as f:
                    saved = json.load(f).get("mode", 1)
                    if saved in (1, 2):
                        return saved
            except (json.JSONDecodeError, OSError):
                pass  # fall back to default below
        return 1

    def _save_mode(self):
        try:
            with open(config.MODE_FILE, "w") as f:
                json.dump({"mode": self._mode}, f)
        except OSError:
            pass  # non-fatal -- worst case, mode resets to 1 on next boot

    def _toggle(self):
        self._mode = 2 if self._mode == 1 else 1
        self._save_mode()

        if self._on_change:
            self._on_change(self._mode)

    @property
    def mode(self):
        return self._mode

    @property
    def active_threshold_cm(self):
        """The distance threshold (cm) for whichever mode is currently active."""
        return config.DIST_THRESH_CM1 if self._mode == 1 else config.DIST_THRESH_CM2

    def close(self):
        self._button.close()


if __name__ == "__main__":
    from signal import pause

    def announce(new_mode):
        print(f"Mode changed to {new_mode}")

    manager = ModeManager(on_change=announce)
    print(f"Starting mode: {manager.mode}")
    pause()
