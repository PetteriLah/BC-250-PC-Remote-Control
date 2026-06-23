---
name: embedded-build-validation
description: Select and execute reproducible build, test, and release validation for BC-250-PC-Remote-Control and WLED. Use for requests to compile, test, prepare a release, change dependencies, CI, board support, web assets, firmware or filesystem images, PlatformIO environments, Arduino Bluepad32 toolchains, versioning, or verification matrices across either repository.
---

# Embedded build validation

Choose checks from the changed surfaces instead of treating one successful build as complete validation.

## Workflow

1. Locate both repositories involved and read their `AGENTS.md`.
2. Classify changes as firmware, web, filesystem, dependency, board-specific, CI, release, or hardware.
3. Read `references/matrix.md`.
4. Run deterministic static and host-side checks first.
5. Build generated assets before firmware.
6. Select the smallest representative target matrix that covers every changed architecture.
7. Report static, unit, compile, browser, device, and hardware checks separately.

## Safety

- Do not upload firmware, format filesystems, erase flash, change credentials, or operate relays without explicit authorization.
- Do not claim hardware validation from compilation or static analysis.
- Preserve generated/source boundaries and avoid committing local override files.
- Record skipped checks and their prerequisites.

## Baseline

- Both: `git diff --check`.
- WLED: Node asset build and tests, then relevant PlatformIO targets.
- BC-250: endpoint-contract checks and compile with the ESP32 Bluepad32 board package when available.
- Release work: verify version metadata, binary/filesystem pairing, and documented upgrade constraints.
