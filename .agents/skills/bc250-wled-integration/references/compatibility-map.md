# Compatibility map

| Concern | BC-250 | WLED | Porting requirement |
|---|---|---|---|
| Main loop | Single Arduino loop with manual intervals | Large cooperative loop plus optional tasks | Keep extension non-blocking |
| HTTP | Synchronous `WebServer` | `ESPAsyncWebServer` | Rewrite handler and upload lifecycles |
| JSON | ArduinoJson documents in handlers | Shared locked JSON buffer and state serializers | Preserve locking and field validation |
| Filesystem | LittleFS config and static files | `WLED_FS`, split public/secret config, embedded generated UI | Namespace data and avoid secret exposure |
| GPIO | Direct writes from power state machine | `pinManager` ownership plus bus subsystem | Allocate and release pins through WLED |
| Extension | Header-coupled modules | `Usermod` lifecycle and linker registration | Prefer a usermod |
| UI | Files served directly from LittleFS | Source assets compiled into compressed headers | Use each native asset pipeline |
| OTA | Raw Update handlers | Auth/PIN-aware OTA subsystem and feature flags | Do not copy handlers directly |
| Build | Arduino/Bluepad32 setup | PlatformIO matrix plus Node asset build | Validate separately |

## Recommended BC-250-on-WLED shape

- Target ESP32 first; the original hardware and GPIO set do not justify claiming ESP8266 support.
- One usermod owns relay pins and monitor/button inputs.
- `readFromConfig()` loads pin numbers, polarities, pulse durations, and enable state.
- `setup()` allocates all required pins atomically through `pinManager` and initializes safe output levels.
- `loop()` advances the non-blocking power state machine.
- `addToJsonState()` and `readFromJsonState()` expose commands and status.
- `addToJsonInfo()` exposes diagnostic state.
- `onUpdateBegin()` cancels pulses, rejects new commands, and places outputs into the documented safe state without shutting down a running PC solely because OTA started.

Use an explicit `Fault` state when required pins conflict or runtime reconfiguration cannot roll back cleanly. Never continue with partially allocated relay control.
