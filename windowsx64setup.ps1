# Stop script execution if any command fails
$ErrorActionPreference = "Stop"

Write-Host "Starting Windows x64 Setup Sequence..."

# Install Git
Write-Host "Installing Git..."
winget install --id Git.Git -e --source winget --accept-source-agreements --accept-package-agreements

# Install Python 3 (64-bit)
Write-Host "Installing Python..."
winget install --id Python.Python.3 -e --source winget --accept-source-agreements --accept-package-agreements

# Install VS Code (64-bit System/User Installer)
Write-Host "Installing Visual Studio Code..."
winget install --id Microsoft.VisualStudioCode -e --source winget --accept-source-agreements --accept-package-agreements

# Refresh environment variables so newly installed tools are visible immediately
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Install the VS Code extension
Write-Host "Installing PlatformIO IDE Extension..."
code --install-extension platformio.platformio-ide

# Install PlatformIO Core CLI
Write-Host "Initializing PlatformIO Core CLI..."
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py" -OutFile "get-platformio.py"
python get-platformio.py

# Clone repository into User Home directory
cd $HOME
if (Test-Path "S.E.N.S.E") { Remove-Item -Recurse -Force "S.E.N.S.E" }
Write-Host "Cloning Git repository..."
git clone https://github.com/luca-anthony/S.E.N.S.E.

cd "$HOME/S.E.N.S.E/Code/RaspPiPico/model001"

# Set environment path for the PlatformIO CLI
$env:Path += ";$HOME\.platformio\penv\Scripts"

Write-Host "Initializing PlatformIO project configuration..."
pio project init --board pico

Write-Host "Compiling and Flashing firmware..."
pio run --target upload

Write-Host "Windows x64 setup and flashing sequence complete!"
