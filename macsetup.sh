#!/bin/zsh

# Exit immediately if a command exits with a non-zero status
set -e

echo "Starting macOS Setup Sequence..."

# Install Homebrew if it isn't present
if ! command -v brew &> /dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

    # Configure path dynamically for both Apple Silicon (arm64) and Intel (x86_64)
    if [[ "$(uname -m)" == "arm64" ]]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
    else
        eval "$(/usr/local/bin/brew shellenv)"
    fi
else
    echo "Homebrew is already installed. Updating..."
    brew update
fi

# Install Git (skip if already installed via Homebrew)
if brew list git &> /dev/null; then
    echo "Git is already installed via Homebrew, skipping."
else
    echo "Installing Git..."
    brew install git
fi

# Install Python (skip if already installed via Homebrew)
if brew list python &> /dev/null; then
    echo "Python is already installed via Homebrew, skipping."
else
    echo "Installing Python..."
    brew install python
fi

# Install VS Code (Visual Studio Code Cask)
# Check for the actual app first: `brew install --cask` errors out (and, with
# `set -e`, kills this whole script) if VS Code is already there - whether it
# was installed by Homebrew or downloaded manually.
if [ -d "/Applications/Visual Studio Code.app" ]; then
    echo "Visual Studio Code is already installed, skipping."
else
    echo "Installing Visual Studio Code..."
    brew install --cask visual-studio-code
fi

# MacOS does not require udev rules or serial port groups like Linux.
# Drivers for the Raspberry Pi Pico serial interface are built natively into macOS.

# Make sure the `code` CLI command is available. Homebrew's cask adds it to
# PATH automatically, but a manually-installed copy of VS Code may not have
# it, since that normally requires running "Shell Command: Install 'code'
# command in PATH" from inside the app once.
if ! command -v code &> /dev/null; then
    echo "Adding VS Code's CLI tool to PATH for this session..."
    export PATH="/Applications/Visual Studio Code.app/Contents/Resources/app/bin:$PATH"
fi

if ! command -v code &> /dev/null; then
    echo "Could not find the 'code' command. Open VS Code once, run"
    echo "Cmd+Shift+P > 'Shell Command: Install code command in PATH', then re-run this script."
    exit 1
fi

# Install the VS Code extension (skip if already installed)
if code --list-extensions | grep -qi "^platformio.platformio-ide$"; then
    echo "PlatformIO IDE extension is already installed, skipping."
else
    echo "Installing PlatformIO IDE Extension..."
    code --install-extension platformio.platformio-ide
fi

# Set environment path so we can detect an existing PlatformIO Core install
export PATH="$HOME/.platformio/penv/bin:$PATH"

# Trigger PlatformIO to install its Core CLI tools silently (skip if already installed)
if command -v pio &> /dev/null; then
    echo "PlatformIO Core CLI is already installed, skipping."
else
    echo "Initializing PlatformIO Core CLI (this may take a minute)..."
    python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"
fi

# Clone repository into the home directory
echo "Cloning Git repository..."
cd "$HOME"
rm -rf S.E.N.S.E
git clone https://github.com/luca-anthony/S.E.N.S.E.

cd "$HOME/S.E.N.S.E/Code/RaspPiPico/model001"

echo "Initializing PlatformIO project configuration..."
pio project init --board pico

echo "Compiling and Flashing firmware..."
pio run --target upload

echo "macOS setup and flashing sequence complete!"
