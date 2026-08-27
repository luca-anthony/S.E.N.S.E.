#==================================
#             COMMANDS
#==================================

"""
sudo raspi-config nonint do_i2c 0
sudo raspi-config nonint do_spi 0
sudo apt update
sudo apt install python3-pip python3-smbus i2c-tools
pip3 install adafruit-circuitpython-vl53l1x
sudo apt install python3-gpiozero
sudo apt install python3-gpiozero python3-spidev
sudo reboot
"""

#==================================
#              WIRING
#==================================

#----------------------------------
#             VL53L1X
#----------------------------------

# VCC --> Pin 1/5V/3.3V
# GND --> Pin 9/GND
# SDA --> Pin 3/GPIO2
# SCL --> Pin 5/GPIO3

#----------------------------------
#       FLEX SENSOR & MCP3008
#----------------------------------

#----MCP3008
# VDD/Pin 16 --> Pin 1/3.3v
# VREF/Pin 15 --> Pin 1/3.3v
# AGND/Pin 14 --> Pin 6/GND
# DGND/Pin 9 --> Pin 6/GND
# CS/SHDN/Pin 10 --> Pin 24/GPIO 8/CE0
# DIN/MOSI/Pin 11 --> Pin 19/GPIO 10/MOSI
# DOUT/MISO/Pin 12 --> Pin 21/GPIO 9/MISO
# CLK/Pin 13 --> Pin 23/GPIO 11/SCLK
# CH0/Pin 1 --> 10K R & FLEX-SENS GND

#----FLEX SENSOR
# VCC --> Pin 1/3.3v
# GND --> 10K R
# 10K R --> Pin 6/GND

#----------------------------------
#      MOTOR/TRANSISTOR/DIODE
#----------------------------------

#----TRANSISTOR
# EMITTER --> Pin 6/GND
# BASE --> 1K R
# 1K R --> Pin 11/GPIO 17
# COLLECTOR --> NEG

#----DIODE
# CATHODE --> 3.3V/Meet With Positive Motor Line
# ANODE --> COLLECTOR

#----MOTOR
# POS --> CATHODE/3.3V
# NEG --> COLLECTOR

