#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Install Git
echo "Installing Git..."
sudo apt update
sudo apt install git -y
git --version

# Install Snap
echo "Installing Snap..."
sudo apt update
sudo apt install -y snapd

# Install VSCode
echo "Installing Visual Studio Code..."
wget -O vscode.deb "https://code.visualstudio.com/sha/download?build=stable&os=linux-deb-x64"
sudo apt update 
sudo apt install -y ./vscode.deb
rm vscode.deb

# Install PlatformIO Dependencies
echo "Installing PlatformIO Dependencies..."
sudo apt update
sudo apt install -y python3-venv git curl

# Download and apply PlatformIO Udev Rules
echo "Configuring Udev Rules..."
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules

# Restart udev service
sudo service udev restart

# Add user to hardware groups
sudo usermod -a -G dialout $USER
sudo usermod -a -G tty $USER

# Install the VS Code extension
echo "Installing PlatformIO IDE Extension..."
code --install-extension platformio.platformio-ide

# Trigger PlatformIO to install its Core CLI tools silently in the background
echo "Initializing PlatformIO Core CLI (this may take a minute)..."
python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"

# Clone repository into the home directory
echo "Cloning Git repository..."
cd "$HOME"
rm -rf S.E.N.S.E 
git clone https://github.com/luca-anthony/S.E.N.S.E.

cd "$HOME/S.E.N.S.E/Code/RaspPiPico/model001"

# Run PlatformIO and Flash
echo "Setting environment path..."
export PATH="$HOME/.platformio/penv/bin:$PATH"

echo "Initializing PlatformIO project configuration..."
pio project init --board pico

echo "Compiling and Flashing firmware..."
pio run --target upload

echo "Setup and flashing sequence complete!"
