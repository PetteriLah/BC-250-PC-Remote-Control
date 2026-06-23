# Embedded threat model

## Shared ingress points

- HTTP request path, query, body, and upload data.
- JSON and WebSocket payloads.
- Wi-Fi provisioning and stored credentials.
- OTA firmware and filesystem images.
- Bluetooth/controller identity.
- UDP, TCP, MQTT, serial, IR, and ESP-NOW where present.
- Files loaded from LittleFS/WLED_FS.
- CI event fields, dependencies, actions, and secrets.

## BC-250 priorities

- Power and OTA routes are currently unauthenticated.
- MAC filtering is device selection, not strong authentication.
- Wi-Fi password persistence and LittleFS auto-format behavior are sensitive.
- Malformed commands must not create unsafe relay states.
- Firmware and filesystem OTA must remain distinct and recoverable.

## WLED priorities

- Follow WLED's documented LAN/firewall trust model.
- Enforce configured PIN/auth behavior on state-changing paths.
- Validate external lengths, indices, segments, pins, and allocation sizes at ingress.
- Preserve JSON buffer locking.
- Protect `wsec.json` values.
- Review OTA integrity without treating TLS as universally mandatory.
- Check generated web UI for DOM XSS and unsafe dynamic execution.

## Finding quality

Do not report generic best practices without an exploitable or failure path. Include the exact side effect and the smallest practical fix.
