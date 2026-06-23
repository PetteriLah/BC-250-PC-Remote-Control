# WLED usermod contract

## Structure

```text
usermods/<name>/
  <name>.cpp
  library.json
  readme.md
  optional headers and platformio_override.ini.sample
```

Include `wled.h`, derive from `Usermod`, instantiate statically, then call `REGISTER_USERMOD(instance)`.

## Lifecycle hooks

- `readFromConfig()` runs before `setup()`.
- `setup()` initializes non-network resources.
- `connected()` runs on Wi-Fi reconnection.
- `loop()` runs once per main-loop iteration.
- `addToJsonState()` and `readFromJsonState()` extend `/json/state`.
- `addToJsonInfo()` extends `/json/info`.
- `addToConfig()` and `readFromConfig()` own persistent settings.
- `appendConfigData()` augments the shared usermod settings UI; its output buffer is constrained.
- Optional hooks include MQTT, UDP, ESP-NOW, button, overlay, OTA begin, and state change.

## IDs and pins

Add a `USERMOD_ID_*` only if another module must find it, it participates in data exchange, it owns pins through `pinManager`, or it needs distinct JSON identification.

## Build integration

Set `custom_usermods = <name>` in a local `platformio_override.ini`. The loader also recognizes `_v2` and `usermod_v2_` naming variants. Local directories with `library.json` are loaded as PlatformIO libraries.
