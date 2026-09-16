#!/bin/zsh

# Exit immediately if a command exits with a non-zero status
set -e

echo "Starting macOS Setup Sequence..."

# Install Homebrew if it isn't present
if ! command -v brew &> /dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://githubusercontent.com)"
    
    # Configure path dynamically for both Apple Silicon (arm64) and Intel (x86_64)
    if [[ "$(uname -m)" == "arm64" ]]; then
        eval "$(/opt/brew/bin/brew shellenv)"
    else
        eval "$(/usr/local/bin/brew shellenv)"
    fi
else
    echo "Homebrew is already installed. Updating..."
    brew update
fi

# Install Git and Python
echo "Installing core dependencies via Homebrew..."
brew install git python

# Install VS Code (Visual Studio Code Cask)
echo "Installing Visual Studio Code..."
brew install --cask visual-studio-code

# MacOS does not require udev rules or serial port groups like Linux. 
# Drivers for the Raspberry Pi Pico serial interface are built natively into macOS.

# Install the VS Code extension
echo "Installing PlatformIO IDE Extension..."
code --install-extension platformio.platformio-ide

# Trigger PlatformIO to install its Core CLI tools silently
echo "Initializing PlatformIO Core CLI (this may take a minute)..."
python3 -c "$(curl -fsSL https://githubusercontent.com)"

# Clone repository into the home directory
echo "Cloning Git repository..."
cd "$HOME"
rm -rf S.E.N.S.E
git clone https://github.com.

cd "$HOME/S.E.N.S.E"

# Run PlatformIO and Flash
echo "Setting environment path..."
export PATH="$HOME/.platformio/penv/bin:$PATH"

echo "Initializing PlatformIO project configuration..."
pio project init --board pico

echo "Compiling and Flashing firmware..."
pio run --target upload

echo "macOS setup and flashing sequence complete!"
