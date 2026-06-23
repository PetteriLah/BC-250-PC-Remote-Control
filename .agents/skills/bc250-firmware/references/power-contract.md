# Power contract

Read current values from `pins.h`. At repository version 1.6.1:

- `OPTO_PIN`: GPIO 16, relay 1 / PSU control.
- `EXTRA_PIN`: GPIO 17, relay 2 / BC-250 power-button pulse.
- `PC_MONITOR_PIN`: GPIO 4, TPMS1 pin 9 monitor input.
- `BUTTON_PIN`: GPIO 22, active-low physical button.
- `POWER_LED_PIN`: GPIO 23.
- `STATUS_LED_PIN`: GPIO 2.

Some serial labels mention older GPIO numbers. Do not use them as configuration.

## Current sequences

- Power on: assert `OPTO_PIN` and `EXTRA_PIN`; release `EXTRA_PIN` after 1 second; wait 8 seconds for monitor confirmation; release `OPTO_PIN` on failure.
- Normal shutdown: assert `EXTRA_PIN` for 500 ms; wait for the monitor to remain low before releasing PSU control.
- Forced shutdown: hold `OPTO_PIN` high for 5 seconds, then set it low and wait for monitor confirmation.
- Physical button: short press while on requests normal shutdown; at least 5 seconds requests forced shutdown; while off requests power on.

## Hazards

- `updatePcState()` can restart the ESP32 after a confirmed transition to off.
- Bluetooth starts and stops according to PC state.
- Blocking delays exist around restart paths; do not spread that pattern.
- `shutdownRequested` exists but is not the state-machine authority.
