# WLED effect contract

## Main locations

- Core effects: `wled00/FX.cpp`, `FX_2Dfcn.cpp`, `FX_fcn.cpp`, `FX.h`.
- Particle system: `wled00/FXparticleSystem.*`.
- Color and palette support: `wled00/colors.*`, `palettes.cpp`.
- External/custom effects: `usermods/user_fx/`.

## Runtime state

Use segment runtime storage for state that survives frames. Treat allocation as fallible and ensure the effect can return a safe frame delay or fallback without corrupting memory.

## Metadata

The effect metadata string controls name, speed/intensity/custom sliders, color selectors, palette support, checkboxes, and defaults exposed in the UI. Keep it synchronized with fields actually consumed by the effect.

## Geometry

Account for segment bounds, grouping, spacing, offset, reverse/mirror, 1D-to-2D mapping, transpose, and virtual dimensions. Do not assume a non-empty segment or a rectangular 2D matrix.

## Portability

ESP8266 has tighter heap, flash, and IRAM limits. Avoid making an effect silently compile only on ESP32 unless guarded and documented.
