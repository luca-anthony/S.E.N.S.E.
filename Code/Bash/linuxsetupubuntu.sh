# Install Git
sudo apt update
sudo apt install git -y
git --version

# Install Snap
echo("Installing Snap")
sudo apt update
sudo apt install snapd

echo("Installing Visual Studio Code")
wget -O vscode.deb "https://visualstudio.com"
sudo apt update && sudo apt install ./vscode.deb
rm vscode.deb

echo("Instaling PlatformIO")
sudo apt update
sudo apt install -y python3-venv git
# 1. Download and install PlatformIO's official udev rules
curl -fsSL https://githubusercontent.com | sudo tee /etc/udev/rules.d/99-platformio-udev.rules

# 2. Restart the udev service
sudo service udev restart

# 3. Add your user to the dialout and tty groups
sudo usermod -a -G dialout $USER
sudo usermod -a -G tty $USER
code --install-extension platformio.platformio-ide
code .
