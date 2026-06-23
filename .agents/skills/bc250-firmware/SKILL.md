---
name: bc250-firmware
description: Develop and review the BC-250 ESP32 power-control firmware. Use for GPIO assignments, relay behavior, button handling, PC-state monitoring, debounce, timing, power state-machine transitions, restart behavior, and changes to ota_pc_remote.ino, pc_control.h, or pins.h that affect physical power sequencing.
---

# BC-250 firmware

Implement power-control changes without violating electrical and timing assumptions.

## Workflow

1. Read `references/power-contract.md`.
2. Trace the relevant trigger through `PowerState`, GPIO writes, monitor feedback, and return to `POWER_IDLE`.
3. State assumed active levels and pulse durations before editing.
4. Keep `loop()` responsive; prefer `millis()` transitions over `delay()`.
5. Preserve rejection of concurrent power operations.
6. Review boot and restart states of both relay outputs.
7. Verify every changed transition.

## Safety invariants

- Use `pins.h` as the authority; comments and logs may contain older GPIO numbers.
- Never leave `EXTRA_PIN` asserted after a completed pulse.
- Do not silently invert relay, monitor, or button polarity.
- Keep unsigned timing comparisons in subtraction form for `millis()` wraparound.
- Avoid blocking HTTP, Bluetooth, monitor sampling, or button debounce.
- Require physical confirmation before claiming hardware validation.

## Verification

- Check idle, already-on/off, normal shutdown, forced shutdown, failed startup, monitor bounce, and reboot paths.
- Check that no transition starts while `powerState != POWER_IDLE`.
- Compare diagnostic pin labels with `pins.h`.
- Compile with the target Bluepad32 ESP32 board package when available.
- Report untested electrical assumptions.
