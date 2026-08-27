#!/bin/bash
echo "Hello! This is a script to install everything you will need for S.E.N.S.E."
echo "One moment please..."

echo "Enabling I2C..."
sudo raspi-config nonint do_i2c 0

echo "Enabling SPI..."
sudo raspi-config nonint do_spi 0

echo "Updating... (this may take a bit)"
sudo apt update

echo "Installing I2C tools..."
sudo apt install -y python3-pip python3-smbus i2c-tools

echo "Installing GPIO tools..."
sudo apt install -y python3-gpiozero python3-spidev

echo "Installing VL53L1X library..."
pip3 install adafruit-circuitpython-vl53l1x adafruit-blinka

echo "Installing pigpio..."
sudo apt install -y pigpio python3-pigpio

echo "Enabling pigpio..."
sudo systemctl enable pigpiod

echo "Starting pigpio..."
sudo systemctl start pigpiod

while true; do
  read -p "Reboot? (Very much recommended) [Y/n]: " yn
  yn=${yn:-Y}
  case $yn in
    [Yy]* ) echo "Rebooting..." && sudo reboot; break;;
    [Nn]* ) echo "Exiting..."; exit;;
    * ) echo "Please answer yes or no";;
  esac
done
