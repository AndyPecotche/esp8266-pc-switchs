This project is used to create a remote power and reset button for any PC or server using an ESP32 or ESP8266/01 board.
Connections:
- GND to PC GND
- VCC to PC 5v USB VCC. (if used esp01s, needs an 5v to 3.3v ams1117 module)
- PWR GPIO to PC PWR+
- RST & STS GPIO to PC RST+

How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-espressif8266/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell
# Change directory to example
$ cd platform-espressif8266/examples/arduino-webserver

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e <esp01_1m or esp32doit-devkit-v1>

# Upload firmware for the specific environment
$ pio run -e <esp01_1m or esp32doit-devkit-v1> --target upload

# Clean build files
$ pio run --target clean
```
