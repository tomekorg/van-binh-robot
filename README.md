# tomek

## Wymagania
- [Visual Studio Code](https://code.visualstudio.com/)

Dla macOS/Linux należy zainstalować dodatkowe zależności (dla Windows brak dodatkowych wymagań):

### macOS
```sh
brew install cmake ninja dfu-util ccache
```

### Linux
```sh
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

[Oficjalna dokumentacja w razie potrzeby (tylko Step 1).](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites)

## Instalacja ESP-IDF

1. Zainstaluj rozszerzenie ESP-IDF w VSCode:
```sh
View > Command Palette > Extensions: Install Extensions
```
2. Skonfiguruj je, wybierając opcję "Express" setup mode.
3. Wskaż wersję ESP-IDF v5.3.2.

[Oficjalna dokumentacja w razie potrzeby.](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/installation.html)

## Import i budowa projektu

1. Sklonuj to repozytorium i otwórz w VSCode:
```sh
cd ~/
git clone git@github.com:krolikbrunatny/van-binh-robot.git
```

2. Zbuduj projekt:
```sh
View > Command Palette > ESP-IDF: Build Your Project
```
