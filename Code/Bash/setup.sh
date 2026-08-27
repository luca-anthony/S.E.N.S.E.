#!/bin/bash
echo "Hello! This is a script to install everything you will need for S.E.N.S.E."
echo "One moment please..."

errors_occurred=0

echo "Enabling I2C..."
sudo raspi-config nonint do_i2c 0 || errors_occurred=1

echo "Enabling SPI..."
sudo raspi-config nonint do_spi 0 || errors_occurred=1

echo "Updating Package List... (this may take a bit)"
sudo apt update || errors_occurred=1

echo "Installing I2C tools..."
sudo apt install -y python3-pip python3-smbus i2c-tools || errors_occurred=1

echo "Installing GPIO tools..."
sudo apt install -y python3-gpiozero python3-spidev || errors_occurred=1

echo "Installing VL53L1X library..."
pip3 install adafruit-circuitpython-vl53l1x adafruit-blinka --break-system-packages || errors_occurred=1

echo "Installing pigpio..."
sudo apt install -y pigpio python3-pigpio || errors_occurred=1

echo "Enabling pigpio..."
sudo systemctl enable pigpiod || errors_occurred=1

echo "Starting pigpio..."
sudo systemctl start pigpiod || errors_occurred=1

if [ $errors_occurred -ne 0 ]; then
  echo ""
  echo "WARNING: Some installations or configurations failed during the process."
  
  while true; do
    read -p "Would you like to run the script again, or just reboot? (run/reboot): " err_choice
    err_choice=$(echo "$err_choice" | tr '[:upper:]' '[:lower:]')
    
    case $err_choice in
      run* )
        echo "Restarting the script..."
        exec "$0" "$@" 
        exit
        ;;
      reboot* )
        while true; do
          read -p "Are you absolutely sure you want to reboot despite the errors? [y/N]: " confirm
          confirm=${confirm:-N}
          case $confirm in
            [Yy]* ) echo "Rebooting..."; sudo reboot; break 2;;
            [Nn]* ) echo "Exiting script without rebooting."; exit;;
            * ) echo "Please answer yes or no.";;
          esac
        done
        ;;
      * )
        echo "Invalid choice. Please type 'run' or 'reboot'."
        ;;
    esac
  done

else
  echo ""
  echo "All installations completed successfully!"
  while true; do
    read -p "Reboot? (Very much recommended) [Y/n]: " yn
    yn=${yn:-Y}
    case $yn in
      [Yy]* ) echo "Rebooting..." && sudo reboot; break;;
      [Nn]* ) echo "Exiting..."; exit;;
      * ) echo "Please answer yes or no.";;
    esac
  done
fi
