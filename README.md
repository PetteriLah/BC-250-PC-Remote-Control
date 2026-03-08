# BC-250-PC-Remote-Control
ESP32-based device for remote control of the BC-250 atx power supply with web interface and PS5 controller support.

Key Features

    Remote Power Control: Turn PC on/off via web interface or physical button
    PS5 Controller Integration: Use DualSense controller to power on PC with PS button
    WiFi Configuration: Web-based setup with network scanning
    MAC Address Locking: Restrict controller access to specific devices
    Over-the-Air Updates: Firmware updates via web interface

Installation

Before installing libraries, you need to add the [ESP32_bluepad32](https://github.com/ricardoquesada/bluepad32) platform to your Arduino IDE.

    Open Arduino IDE.
    Go to File > Preferences (or Arduino IDE > Settings on macOS).
    In the "Additional Boards Manager URLs" field, paste the following URL:
    text
```
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json ,https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json 
```
    (If you already have other URLs there, separate them with a comma).

    Click OK.

    Go to Tools > Board > Boards Manager.
    In the search bar, type "esp32_bluepad32".

installing [LittleFS (for ESP32)](https://github.com/lorol/LITTLEFS) 
The LittleFS library is used to serve the web interface files. The version in the Library Manager is not always compatible with ESP32.

    Go to the GitHub repository for the ESP32 version: https://github.com/lorol/LITTLEFS

    Click the green "Code" button and select "Download ZIP".

    Extract the downloaded ZIP file.

    Rename the extracted folder to LITTLEFS (note the all-caps name).

    Move this LITTLEFS folder into your Arduino libraries folder.

    Find "esp32 by Espressif Systems" and click Install. (Version 2.0.x or higher is recommended).
