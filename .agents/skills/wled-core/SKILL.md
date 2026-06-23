---
name: wled-core
description: Develop, review, and validate WLED core firmware and PlatformIO configuration. Use for lifecycle, LED buses, networking, persistence, OTA, pin management, feature flags, board environments, pio-scripts, memory or timing optimization, and multi-target ESP32 or ESP8266 builds. Use wled-effects instead for effect and pixel-rendering algorithms.
---

# WLED core firmware

Work within WLED's constrained multi-target firmware architecture and build pipeline.

## Workflow

1. Locate the WLED checkout and read its `AGENTS.md` plus applicable `docs/*.instructions.md`.
2. Read `references/architecture.md` and `references/build-validation.md`.
3. Identify the affected targets, feature flags, hot paths, persistence contracts, and generated artifacts.
4. Prefer a usermod over a core change when the feature does not require core ownership.
5. Keep ESP8266 RAM/flash constraints and ESP32 variant differences explicit.
6. Follow the repository's C++ style and AI-attribution requirements.
7. Build the web assets before compiling firmware.

## Core invariants

- Include `wled.h` as the primary project header.
- Do not use C++ exceptions.
- Avoid heap churn, `String` growth, blocking delays, and floating-point work in render or loop hot paths.
- Validate untrusted input at its first HTTP, JSON, WebSocket, UDP, TCP, serial, MQTT, or ESP-NOW ingress.
- Verify every new `WLED_ENABLE_*` or `WLED_DISABLE_*` spelling against existing definitions.
- Do not edit generated `wled00/html_*.h` or `wled00/js_*.h`.
- Do not modify `platformio.ini` without explicit maintainer approval.

## Verification

- Run `npm ci`, `npm run build`, and `npm test`.
- Build the smallest relevant PlatformIO environment and at least `esp32dev` for shared ESP32 core changes.
- Add `nodemcuv2` when code is shared with ESP8266.
- Add the exact S2, S3, C3, Ethernet, HUB75, or usermod environment affected by the change.
- Report firmware size, linker, feature-flag, and hardware-test gaps separately.
