#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Sync package databases and update system
echo "Updating package databases..."
sudo pacman -Syu --noconfirm

# Install Git
echo "Installing Git..."
sudo pacman -S --needed --noconfirm git
git --version

# Install Snap (Requires building from AUR on Arch)
echo "Installing Snapd prerequisites..."
sudo pacman -S --needed --noconfirm base-devel

echo "Building and installing Snapd from the AUR..."
cd /tmp
rm -rf snapd
git clone https://aur.archlinux.org/snapd.git
cd snapd
makepkg -si --noconfirm

echo "Enabling Snap communication socket..."
sudo systemctl enable --now snapd.socket
sudo ln -s /var/lib/snapd/snap /snap

# Install VSCode (Natively available via Arch community repos as 'code')
echo "Installing Visual Studio Code (OSS Variant)..."
sudo pacman -S --needed --noconfirm code

# Install PlatformIO Dependencies
echo "Installing PlatformIO Dependencies..."
sudo pacman -S --needed --noconfirm python python-virtualenv git curl

# Download and apply PlatformIO Udev Rules
echo "Configuring Udev Rules..."
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules

# Restart udev service
echo "Reloading Udev Rules..."
sudo udevadm control --reload-rules
sudo udevadm trigger

# Add user to hardware groups (Arch uses 'uucp' instead of 'dialout')
sudo usermod -a -G uucp $USER
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
