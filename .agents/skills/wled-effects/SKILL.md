---
name: wled-effects
description: Create, port, optimize, and review WLED 1D or 2D LED effects, palettes, transitions, segment rendering, particle systems, custom user effects, and effect metadata. Use for FX.cpp, FX.h, FX_fcn.cpp, FX_2Dfcn.cpp, FXparticleSystem files, palettes.cpp, colors files, overlay rendering, or usermods/user_fx.
---

# WLED effects and rendering

Keep visual behavior correct under strict frame-time, heap, geometry, and cross-target constraints.

## Workflow

1. Read the checkout's `AGENTS.md`, C++ instructions, and `references/effect-contract.md`.
2. Decide whether the effect belongs in core or `usermods/user_fx`.
3. Define 1D/2D support, segment geometry, mapping behavior, runtime state, controls, and fallback.
4. Allocate persistent effect state through the segment environment and handle allocation failure.
5. Precompute invariants and keep frame-path operations bounded.
6. Add a correct metadata string and register the effect.
7. Build on ESP32 and ESP8266 unless the effect is explicitly target-limited.

## Hot-path rules

- Never use `delay()` in an effect.
- Avoid repeated allocation, `String`, filesystem, networking, or logging in frame paths.
- Cache frequently used segment values locally.
- Guard empty, one-pixel, short, and non-rectangular geometry.
- Use WLED's integer/approximate math helpers where suitable.
- Keep size and offset arithmetic overflow-safe.
- Reset or rebuild effect data when dimensions or controls invalidate it.

## Verification

- Test minimum, typical, and maximum segment sizes.
- Test reverse, mirror, transpose, grouping, spacing, offset, and virtual-strip behavior.
- Test allocation failure and effect reset.
- Verify every metadata control against the implementation.
- Check behavior across time wraparound.
- Build `esp32dev` and `nodemcuv2`; add 2D/HUB75 or usermod targets when applicable.
