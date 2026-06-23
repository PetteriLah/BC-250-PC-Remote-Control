---
name: bc250-wled-integration
description: Analyze, adapt, and integrate features or engineering patterns between BC-250-PC-Remote-Control and WLED. Use when borrowing WLED architecture for BC-250, implementing BC-250 power control as a WLED usermod, sharing HTTP or JSON concepts, comparing OTA, Wi-Fi, filesystem, GPIO, state-machine, UI, testing, or security approaches, or copying and transforming code across the two repositories.
---

# BC-250 and WLED integration

Port behavior deliberately rather than copying code across incompatible architectures.

## Workflow

1. Read `references/compatibility-map.md` and `references/provenance.md`.
2. Locate both current checkouts and read their `AGENTS.md` files.
3. Define the behavior to preserve independently of either implementation.
4. Choose the destination extension point:
   - BC-250 state machine, platform service, or HTTP/UI.
   - WLED usermod, JSON extension, web source, or core change.
5. Map lifecycle, GPIO ownership, persistence, auth, memory, timing, and build differences.
6. Reimplement the smallest compatible behavior; avoid mechanical copy-paste.
7. Validate each repository with its native build and test workflow.

## Integration rules

- Prefer a WLED usermod for BC-250-specific functionality.
- Use WLED `pinManager` instead of direct uncoordinated GPIO ownership.
- Preserve BC-250 relay safety and explicit `PowerState` transitions.
- Do not make WLED's render loop or BC-250's web loop block on power sequencing.
- Keep persistence schemas namespaced and migration-safe.
- Do not assume WLED authentication, OTA, filesystem, JSON, or async server behavior exists in BC-250.
- Review licensing and attribution before reusing non-trivial WLED code.

## Verification

- Create a behavior and failure-mode matrix before implementation.
- Test boot defaults, invalid config, concurrent commands, timeout, restart, and interrupted OTA or filesystem operations.
- Build WLED assets and relevant PlatformIO targets.
- Compile BC-250 with its Bluepad32 ESP32 toolchain.
- Hardware-test relay polarity and pulse timing before deployment.
