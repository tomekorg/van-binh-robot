# tomek

## Requirements
- [Visual Studio Code](https://code.visualstudio.com/)

For macOS/Linux additional dependencies must be installed (no additional requirements for Windows):

### macOS
```sh
brew install cmake ninja dfu-util ccache
```

### Linux
```sh
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

[Official documentation for reference (only Step 1).](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites)

## Installing ESP-IDF

1. Install the ESP-IDF extension in VSCode:
```sh
View > Command Palette > Extensions: Install Extensions
```
2. Configure it by selecting the "Express" setup mode.
3. Select version ESP-IDF v5.3.2.

[Official documentation for reference.](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/installation.html)

## Importing and building the project

1. Clone this repository and open it in VSCode:
```sh
cd ~/
git clone https://github.com/krolikbrunatny/van-binh-robot.git
```

2. Build your project:
```sh
View > Command Palette > ESP-IDF: Build Your Project
```
