# WLED architecture snapshot

Analyzed from `wled/WLED` commit `67fa2ba096e1b9628151ddfda5f432f7186c1d8d` dated June 22, 2026. Re-read the checkout because WLED evolves rapidly.

## Main areas

- `wled00/wled_main.cpp`, `wled.cpp`, `wled.h`: application lifecycle and shared state.
- `wled00/FX*.cpp`, `FX.h`: 1D/2D effects and render pipeline.
- `wled00/bus_manager*`, `bus_wrapper.h`, `led.cpp`: LED output and hardware buses.
- `wled00/network.cpp`, `mqtt.cpp`, `udp.cpp`, `ws.cpp`, `e131.cpp`: connectivity and protocols.
- `wled00/json.cpp`, `cfg.cpp`, `set.cpp`, `presets.cpp`: state and persistence.
- `wled00/wled_server.cpp`, `ota_update.cpp`: HTTP server and OTA.
- `wled00/pin_manager*`: pin ownership and conflict prevention.
- `platformio.ini`, `pio-scripts/`: targets, feature flags, usermods, UI build, binary output.

## Resource model

- ESP8266 builds are particularly flash, heap, and IRAM constrained.
- Effects and LED output paths are timing-sensitive.
- PSRAM must be detected at runtime; availability cannot be inferred only from a build flag.
- Classic ESP32 PSRAM is not generally DMA-capable.
- Configuration writes can stall rendering and wear flash.

## Security context

Use the checkout's `docs/hardening.instructions.md` and `docs/securecode.instructions.md`. WLED assumes LAN/firewall isolation and does not require TLS as a universal baseline, but still requires input validation, configured auth enforcement, secret protection, OTA integrity review, and safe failure behavior.
