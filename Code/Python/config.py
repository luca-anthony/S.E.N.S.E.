"""
S.E.N.S.E. -- config.py
 
All pin numbers and tunable thresholds live here, and nowhere else.
If you're calibrating the device for a new user or swapping hardware,
this is the only file you should need to touch.
"""

import os

# =================================
#          SETUP COMMANDS 
# =================================
"""
sudo raspi-config nonint do_i2c 0
sudo raspi-config nonint do_spi 0
sudo apt update
sudo apt install -y python3-pip python3-smbus i2c-tools
sudo apt install -y python3-gpiozero python3-spidev
pip3 install adafruit-circuitpython-vl53l1x adafruit-blinka
sudo apt install -y pigpio python3-pigpio
sudo systemctl enable pigpiod
sudo systemctl start pigpiod
sudo reboot
"""

# ==================================
#              WIRING
# ==================================
#
# ----------------------------------
#             VL53L1X
# ----------------------------------
# VCC --> Pin 1/5V/3.3V
# GND --> Pin 9/GND
# SDA --> Pin 3/GPIO2
# SCL --> Pin 5/GPIO3
#
# ----------------------------------
#       FLEX SENSOR & MCP3008
# ----------------------------------
# ----MCP3008
# VDD/Pin 16  --> Pin 1/3.3v
# VREF/Pin 15 --> Pin 1/3.3v
# AGND/Pin 14 --> Pin 6/GND
# DGND/Pin 9  --> Pin 6/GND
# CS/SHDN/Pin 10 --> Pin 24/GPIO 8/CE0
# DIN/MOSI/Pin 11 --> Pin 19/GPIO 10/MOSI
# DOUT/MISO/Pin 12 --> Pin 21/GPIO 9/MISO
# CLK/Pin 13  --> Pin 23/GPIO 11/SCLK
# CH0/Pin 1   --> 10K R & FLEX-SENS GND
#
# ----FLEX SENSOR
# VCC --> Pin 1/3.3v
# GND --> 10K R
# 10K R --> Pin 6/GND
#
# ----------------------------------
#      MOTOR/TRANSISTOR/DIODE
# ----------------------------------
# ----TRANSISTOR
# EMITTER --> Pin 6/GND
# BASE --> 1K R
# 1K R --> Pin 11/GPIO 17
# COLLECTOR --> NEG
#
# ----DIODE
# CATHODE --> 3.3V/Meet With Positive Motor Line
# ANODE --> COLLECTOR
#
# ----MOTOR
# POS --> CATHODE/3.3V
# NEG --> COLLECTOR
#
# ----------------------------------
#          MODE BUTTON
# ----------------------------------
# Not in the original build -- added for mode switching (nav vs close-range).
# One leg --> GPIO27 (Pin 13)
# Other leg --> Pin 6/GND
# gpiozero's Button uses an internal pull-up, so no resistor needed.

# ==================================
#                PINS
# ==================================

MOT_PIN = 17
BTN_PIN = 27
FLEX_CHANNEL = 0

# ==================================
#  THRESHOLDS (calibrate per user)
# ==================================

FLEX_THRESH = 0.4 # Hand flat threshold, 0.0-1.0
DIST_THRESH_CM1 = 80 # Mode 1: General Navigation
DIST_THRESH_CM2 = 4 # Mode 2: close object/grabbing range

MOT_MIN = 0.4 # Floor so weak motors actually kick on (0.0-1.0)
MOT_MAX = 1.0 # Max Vibration Intensity

SENS_INTERVALS = 0.05 # Poll sensors ever 50ms
DEBOUNCES = 0.2 # Button debounce window
DIST_SMOOTHING_ALPHA = 0.35 # 0=very smooth/slow, 1=raw/instant

# Where the current selected mode is across reboots

MODE_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "sense_mode.json")
