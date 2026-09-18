# Stop script execution if any command fails
$ErrorActionPreference = "Stop"

Write-Host "Starting Windows x32 Setup Sequence..."

# Install Git (skip if already installed)
if (winget list --id Git.Git -e --source winget | Select-String -Quiet "Git.Git") {
    Write-Host "Git is already installed, skipping."
} else {
    Write-Host "Installing Git..."
    winget install --id Git.Git -e --architecture x86 --source winget --accept-source-agreements --accept-package-agreements
}

# Install Python 3 (32-bit) (skip if already installed)
if (winget list --id Python.Python.3 -e --source winget | Select-String -Quiet "Python.Python.3") {
    Write-Host "Python is already installed, skipping."
} else {
    Write-Host "Installing Python..."
    winget install --id Python.Python.3 -e --architecture x86 --source winget --accept-source-agreements --accept-package-agreements
}

# Install VS Code (32-bit System/User Installer) (skip if already installed)
if (winget list --id Microsoft.VisualStudioCode -e --source winget | Select-String -Quiet "Microsoft.VisualStudioCode") {
    Write-Host "Visual Studio Code is already installed, skipping."
} else {
    Write-Host "Installing Visual Studio Code..."
    winget install --id Microsoft.VisualStudioCode -e --architecture x86 --source winget --accept-source-agreements --accept-package-agreements
}

# Refresh environment variables so newly installed tools are visible immediately
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Install the VS Code extension (skip if already installed)
if (code --list-extensions | Select-String -Quiet -SimpleMatch "platformio.platformio-ide") {
    Write-Host "PlatformIO IDE extension is already installed, skipping."
} else {
    Write-Host "Installing PlatformIO IDE Extension..."
    code --install-extension platformio.platformio-ide
}

# Set environment path so we can detect an existing PlatformIO Core install
$env:Path += ";$HOME\.platformio\penv\Scripts"

# Install PlatformIO Core CLI (skip if already installed)
if (Get-Command pio -ErrorAction SilentlyContinue) {
    Write-Host "PlatformIO Core CLI is already installed, skipping."
} else {
    Write-Host "Initializing PlatformIO Core CLI..."
    Invoke-WebRequest -Uri "https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py" -OutFile "get-platformio.py"
    python get-platformio.py
}

# Clone repository into User Home directory
cd $HOME
if (Test-Path "S.E.N.S.E") { Remove-Item -Recurse -Force "S.E.N.S.E" }
Write-Host "Cloning Git repository..."
git clone https://github.com/luca-anthony/S.E.N.S.E.

cd "$HOME/S.E.N.S.E/Code/RaspPiPico/model001"

Write-Host "Initializing PlatformIO project configuration..."
pio project init --board pico

Write-Host "Compiling and Flashing firmware..."
pio run --target upload

Write-Host "Windows x32 setup and flashing sequence complete!"
