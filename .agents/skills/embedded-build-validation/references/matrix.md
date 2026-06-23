# Validation matrix

## WLED

| Change | Required checks |
|---|---|
| Web source | `npm ci`, `npm run build`, `npm test`, one firmware build |
| Shared core | web build/tests, `pio run -e esp32dev`, `pio run -e nodemcuv2` |
| ESP32 variant | baseline ESP32 plus exact C3/S2/S3/Ethernet/HUB75 environment |
| Usermod | matching `usermods_*` environments and custom override sample |
| Build scripts/CI | affected scripts locally where possible plus workflow review |

Do not edit or commit WLED generated HTML/JS headers or local `platformio_override.ini`.

## BC-250

The repository does not currently include a reproducible PlatformIO manifest. Verify:

- Arduino/ESP32 Bluepad32 board package availability.
- ArduinoJson and LittleFS compatibility.
- Firmware compilation.
- LittleFS asset packaging/upload separately from firmware.
- UI endpoint and JSON contract consistency.
- Physical relay polarity and timing only on authorized hardware.

The missing pinned build manifest is a release reproducibility risk; report it rather than inventing versions.
