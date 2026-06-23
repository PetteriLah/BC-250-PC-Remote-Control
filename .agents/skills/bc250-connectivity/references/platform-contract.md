# Platform contract

## Startup

`setup()` initializes pins, reads monitor state, loads Wi-Fi configuration, connects or starts AP mode, mounts LittleFS, initializes Bluepad32, loads PS5 configuration, and starts the web server.

LittleFS is currently mounted more than once. Avoid accidental formatting and make mount failure explicit.

## Persistence

- `/wifi_config.json`: `ssid`, `password`.
- `/ps5_config.json`: `enabled`, `macAddress`, `autoConnect`.
- Empty or all-zero PS5 MAC means no lock.

Use bounded ArduinoJson documents and validate required fields before replacing active configuration.

## Connectivity

- AP SSID: `BC-250-POWER-CONTROL`.
- Provisioning is HTTP-only and must work without internet.
- Station connection attempts are bounded to 20 iterations of 100 ms.
- Bluepad32 accepts a controller only while the PC is off and power state is idle.

## OTA

- `/update` writes firmware with `UPDATE_SIZE_UNKNOWN`.
- `/update-fs` writes the filesystem using `U_SPIFFS`.
- Both restart after completion.
- There is currently no authentication, signature verification, or CSRF protection.
