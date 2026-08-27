#!/bin/bash

LOG_FILE="/var/log/sense_install.log"
FAILED_COMMANDS=()
ERRORS_OCCURRED=0
SILENT_MODE=0

if [ "$EUID" -ne 0 ]; then
  echo "Error: This script must be run as root or with sudo."
  exit 1
fi

for arg in "$@"; do
  if [ "$arg" == "--silent" ]; then
    SILENT_MODE=1
  fi
done

touch "$LOG_FILE" 2>/dev/null
if [ $? -ne 0 ]; then
  LOG_FILE="./sense_install.log"
  touch "$LOG_FILE"
fi

clean_failed_apt() {
  local package_list="$1"
  if [ $SILENT_MODE -eq 0 ]; then
    echo "Cleaning up partial or failed installations for: $package_list"
  fi
  sudo apt purge -y $package_list >> "$LOG_FILE" 2>&1
  sudo apt autoremove -y >> "$LOG_FILE" 2>&1
}

clean_failed_pip() {
  local package_list="$1"
  if [ $SILENT_MODE -eq 0 ]; then
    echo "Cleaning up partial or failed pip installations for: $package_list"
  fi
  pip3 uninstall -y $package_list >> "$LOG_FILE" 2>&1
}

run_step() {
  local step_name="$1"
  local cmd="$2"
  local cleanup_type="$3"
  local cleanup_packages="$4"
  local attempt=1
  local max_attempts=2

  while [ $attempt -le $max_attempts ]; do
    if [ $SILENT_MODE -eq 0 ]; then
      echo "Running: $step_name (Attempt $attempt/$max_attempts)..."
    fi
    eval "$cmd" >> "$LOG_FILE" 2>&1
    
    if [ $? -eq 0 ]; then
      if [ $SILENT_MODE -eq 0 ]; then
        echo "Success: $step_name completed."
      fi
      return 0
    else
      if [ $SILENT_MODE -eq 0 ]; then
        echo "Warning: $step_name failed on attempt $attempt."
      fi
      
      ((attempt++))
      if [ $attempt -le $max_attempts ]; then
        if [ "$cleanup_type" == "apt" ] && [ -n "$cleanup_packages" ]; then
          clean_failed_apt "$cleanup_packages"
        elif [ "$cleanup_type" == "pip" ] && [ -n "$cleanup_packages" ]; then
          clean_failed_pip "$cleanup_packages"
        fi
        
        if [ $SILENT_MODE -eq 0 ]; then
          echo "Retrying..."
        fi
        sleep 2
      fi
    fi
  done

  if [ $SILENT_MODE -eq 0 ]; then
    echo "Error: $step_name failed after $max_attempts attempts."
  fi
  FAILED_COMMANDS+=("$step_name")
  ERRORS_OCCURRED=1
  return 1
}

verify_hardware_buses() {
  if [ $SILENT_MODE -eq 0 ]; then
    echo "Verifying hardware buses..."
  fi
  
  local i2c_status=0
  local spi_status=0

  if [ -e /dev/i2c-* ]; then
    if [ $SILENT_MODE -eq 0 ]; then
      echo "Success: I2C hardware bus detected."
    fi
  else
    echo "Warning: I2C interface is enabled but no active bus was found in /dev/."
    i2c_status=1
  fi

  if [ -e /dev/spidev* ]; then
    if [ $SILENT_MODE -eq 0 ]; then
      echo "Success: SPI hardware bus detected."
    fi
  else
    echo "Warning: SPI interface is enabled but no active bus was found in /dev/."
    spi_status=1
  fi

  if [ $i2c_status -ne 0 ] || [ $spi_status -ne 0 ]; then
    ERRORS_OCCURRED=1
    if [ $i2c_status -ne 0 ]; then FAILED_COMMANDS+=("I2C Hardware Verification"); fi
    if [ $spi_status -ne 0 ]; then FAILED_COMMANDS+=("SPI Hardware Verification"); fi
  fi
}

verify_python_libraries() {
  if [ $SILENT_MODE -eq 0 ]; then
    echo "Verifying Python environment and libraries..."
  fi

  python3 -c "import adafruit_vl53l1x" >> "$LOG_FILE" 2>&1
  if [ $? -eq 0 ]; then
    if [ $SILENT_MODE -eq 0 ]; then
      echo "Success: VL53L1X library environment check passed."
    fi
  else
    echo "Warning: VL53L1X environment validation failed."
    ERRORS_OCCURRED=1
    FAILED_COMMANDS+=("VL53L1X Python Library Verification")
  fi

  python3 -c "import pigpio" >> "$LOG_FILE" 2>&1
  if [ $? -eq 0 ]; then
    if [ $SILENT_MODE -eq 0 ]; then
      echo "Success: pigpio Python daemon binding check passed."
    fi
  else
    echo "Warning: pigpio Python daemon binding validation failed."
    ERRORS_OCCURRED=1
    FAILED_COMMANDS+=("pigpio Python Library Verification")
  fi
}

if [ $SILENT_MODE -eq 0 ]; then
  echo "Starting installation for S.E.N.S.E."
  echo "Logs are being recorded to: $LOG_FILE"
  echo "One moment please..."
fi

run_step "Enabling I2C" "sudo raspi-config nonint do_i2c 0" "none" ""
run_step "Enabling SPI" "sudo raspi-config nonint do_spi 0" "none" ""
run_step "Updating Package List" "sudo apt update" "none" ""
run_step "Installing I2C tools" "sudo apt install -y python3-pip python3-smbus i2c-tools" "apt" "python3-pip python3-smbus i2c-tools"
run_step "Installing GPIO tools" "sudo apt install -y python3-gpiozero python3-spidev" "apt" "python3-gpiozero python3-spidev"
run_step "Installing VL53L1X library" "pip3 install adafruit-circuitpython-vl53l1x adafruit-blinka --break-system-packages" "pip" "adafruit-circuitpython-vl53l1x adafruit-blinka"
run_step "Installing pigpio" "sudo apt install -y pigpio python3-pigpio" "apt" "pigpio python3-pigpio"
run_step "Enabling pigpio service" "sudo systemctl enable pigpiod" "none" ""
run_step "Starting pigpio service" "sudo systemctl start pigpiod" "none" ""

verify_hardware_buses
verify_python_libraries

if [ $ERRORS_OCCURRED -ne 0 ]; then
  echo ""
  echo "Installation phase finished with errors."
  echo "The following components failed to install or configure:"
  for component in "${FAILED_COMMANDS[@]}"; do
    echo " - $component"
  done
  echo "Please check $LOG_FILE for details."
  echo ""

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
          read -p "Are you sure you want to reboot despite the errors? [y/N]: " confirm
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
  if [ $SILENT_MODE -eq 0 ]; then
    echo ""
    echo "All installations completed successfully."
    while true; do
      read -p "Reboot? (Recommended) [Y/n]: " yn
      yn=${yn:-Y}
      case $yn in
        [Yy]* ) echo "Rebooting..." && sudo reboot; break;;
        [Nn]* ) echo "Exiting..."; exit;;
        * ) echo "Please answer yes or no.";;
      esac
    done
  fi
fi
