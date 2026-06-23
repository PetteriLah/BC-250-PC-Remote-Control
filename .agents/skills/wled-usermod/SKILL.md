---
name: wled-usermod
description: Create, modify, review, and build WLED usermods and custom effects. Use for features under usermods/, Usermod lifecycle hooks, persistent usermod configuration, pin allocation, JSON state or info extensions, MQTT or ESP-NOW hooks, inter-usermod data exchange, effect registration, library.json, and custom_usermods PlatformIO configuration.
---

# WLED usermod

Extend WLED without modifying core firmware unless a core hook is genuinely required.

## Workflow

1. Read the target checkout's `AGENTS.md`, `usermods/EXAMPLE/`, and `references/usermod-contract.md`.
2. Copy the example pattern into a new directory; never edit the example itself.
3. Define lifecycle, configuration, pin, JSON, network, and memory requirements.
4. Add `library.json` and use `platformio_override.ini.sample` only when custom build settings are necessary.
5. Register one static instance with `REGISTER_USERMOD(instance)`.
6. Add a unique ID only for lookup/data exchange, pin ownership, or JSON identification.
7. Build on every supported MCU family.

## Runtime rules

- Return quickly from `loop()` when disabled or when work is not due.
- Do not use long delays; use `millis()` scheduling.
- Expect loop frequency to vary dramatically under LED, filesystem, and network load.
- Use `pinManager` for owned GPIOs and release pins when disabled or reconfigured.
- Validate JSON, MQTT, UDP, and ESP-NOW input before using lengths, indices, pins, or allocations.
- Avoid repeated filesystem writes and never call `serializeConfig()` from network callbacks.
- Store repeated strings in PROGMEM.

## Verification

- Test missing and partial config, runtime reconfiguration, disabled mode, reconnect, OTA begin, and invalid external input.
- Run Node asset build/tests if UI or JSON presentation changes.
- Build the matching `usermods_esp32`, `usermods_esp32c3`, `usermods_esp32s2`, and `usermods_esp32s3` environments when portability is claimed.
- Run any custom environment from the usermod's override sample.
- Report actual hardware and sensor testing separately.
