# WLED build validation

## Required order

```text
npm ci
npm run build
npm test
pio run -e <environment>
```

The Node build converts `wled00/data/` assets into generated firmware headers. Never edit or commit generated `html_*.h` and `js_*.h`.

## Common environments

- `esp32dev`: baseline classic ESP32.
- `nodemcuv2`: baseline ESP8266.
- `esp32c3dev`: ESP32-C3.
- `lolin_s2_mini`: ESP32-S2.
- `esp32s3dev_8MB_opi`: ESP32-S3 with PSRAM.
- `usermods`: all usermods, large ESP32 partition.

Use `platformio_override.ini` for local custom builds. Do not commit it. Changes to the main `platformio.ini` require maintainer approval.

## Scope selection

- Core C++ shared by all targets: build ESP32 and ESP8266.
- Architecture-specific code: build every affected MCU family.
- Web-only change: Node build and tests, then one firmware build to verify generated headers integrate.
- Usermod: use the matching `usermods_*` environment and any provided sample override.
