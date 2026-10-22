# WO-004: V1 exerciser sketch: spec constants and LED behavior

**Author:** Claude (architect, in the app), from the GCC architect's spec and answers of 22 Oct 2026
**Executor:** Claude Code (workflow controller), Sonnet-class model. One file, about 60 lines, several exact boundary cases in the LED timing; not Haiku.
**Device operations:** Chip only (upload, scope, oscillator check). Claude Code builds only.
**Governing process:** Chip's AI_DEVELOPMENT_WORKFLOW.md (kept in the Generalized-Core-Counter repo; roles as in WO-002).
**Size budget:** `src/main.cpp` at most 80 lines in total (it is 35 today; the GCC spec asks for about 60 to 80). No other files in `src/`, no libraries, no configuration, no EEPROM, no serial. Report budget vs actual in the closing record.

## Goal (plain language)
The pulse pattern already works (WO-002, WO-003). This WO adds the LED behavior and the named constants from the GCC spec, so that the LED shows where the counter should be in its occupancy session. When the exerciser's session marker stops, the counter's own LED should go off within about 1 to 2 s. That is the visual cross-check.

## What does not change
- IO1 (PA6, J2 pin 5): push-pull, idle LOW, pulse HIGH; IO2 (PA4) untouched, left as an input.
- Absolute schedule from one start time, wrap-safe `millis()` subtraction, no chained delays, no `delay()` in `loop()`.
- 6 s quiet after power-up, 10 pulses of 2000 ms high, 30 000 ms rise to rise, 1 200 000 ms cycle.

## Constants (top of file, one place to change; rename `QUIET_MS` to `STARTUP_QUIET_MS`)
| Name | Value |
| --- | --- |
| STARTUP_QUIET_MS | 6000 |
| PULSE_HIGH_MS | 2000 |
| PULSES_PER_CLUSTER | 10 |
| PULSE_SPACING_MS | 30000 |
| CYCLE_MS | 1200000 |
| DEBOUNCE_MS | 60000, with the comment: "Must equal the counter's sensor.setting1. Used only for the LED session marker and the expected-value comment; does not affect pulse timing." |

LED pins: red `PIN_PC3`, green `PIN_PC2`, blue `PIN_PC1`; HIGH = on (confirmed on board 1). Each channel draws about 2 mA, so the LED is only ever on briefly.

## LED behavior
Let `pos` be milliseconds since the start of the current cycle (0 to CYCLE_MS), `k = pos / PULSE_SPACING_MS` and `r = pos % PULSE_SPACING_MS`. Let `LAST_RISE = (PULSES_PER_CLUSTER - 1) * PULSE_SPACING_MS` (270 000 ms) and `tl = pos - LAST_RISE`. Exactly one colour (or none) is on at a time.

| When | LED |
| --- | --- |
| Power-up | One 200 ms blue blink starting at the beginning of the quiet period ("armed") |
| IO1 high (r < PULSE_HIGH_MS, k < 9) | Solid: green for the first pulse (k = 0), blue for pulses 2 to 9 (k = 1 to 8) |
| Pulse 10 (tl < PULSE_HIGH_MS) | Solid red |
| Between pulses, before the last (k < 9, r from 5000 ms) | 50 ms blink whenever `r % 5000 < 50`, in the colour of pulse k (green for k = 0, blue for k = 1 to 8). That is blinks at 5, 10, 15, 20 and 25 s after each rise |
| After the last rise, while tl < DEBOUNCE_MS | Red 50 ms blink whenever `tl % 5000 < 50` and `tl >= 5000`: blinks at 5, 10, ..., 55 s after the last rise; none at 60 s |
| Gap (pos > LAST_RISE + DEBOUNCE_MS) | Off, with a 20 ms blue heartbeat whenever `pos % PULSE_SPACING_MS < 20` (first at 360 s, last at 1170 s: 28 heartbeats). Heartbeat colour was not specified: blue is an assumption |

Notes:
- "Dim" is only brief: the LED has no brightness control, so the blinks are short on-times.
- Quiet period at power-up: IO1 stays LOW and the LED shows only the armed blink.
- Average LED load is well under 0.1 mA (about 0.04 mA by the numbers above: 20 s of solid LED per 1200 s cycle plus about 3 s of blinks).

## Expected values (put these in the file's header comment)
```
session   = (PULSES-1) x SPACING + DEBOUNCE = 9 x 30 + 60 = 330 s = 5.5 min
per hour  = 3 sessions = 16.5 min
per open day = whole cycles completed x 5.5 min
tolerance = about +/-1 s per session (the counter credits whole seconds)
```

## Verification by Claude Code (no device)
1. Build with `pio run` if PlatformIO is available; report if not. Do not upload.
2. Throwaway check, not committed: simulate the pulse and LED rules over one cycle on the host (any language) and print each transition as seconds from cycle start and colour. Compare with the table above: green at 0 to 2 s, blue at 30 to 32 s and so on through pulse 9, red at 270 to 272 s, red blinks at 275 to 325 s, nothing at 330 s, blue heartbeats from 360 s to 1170 s. Report mismatches; fix before closing.

## Bench acceptance (Chip)
- Scope on J2 pin 5: 6 s quiet after EN goes low; 2.00 s highs; 30.0 s rise to rise; 10 pulses per cluster; 20 min cycle.
- LED: green, blue x8, red, then red blinks for 60 s, then off.
- EN power-cycle mid-cluster restarts the pattern from the quiet period.
- Oscillator: time one hour of cycles against a reference and report the error. Within 0.5% is fine for V1 (0.5% over a 16 h day is about 4.8 min). Beyond that, plan calibration or a crystal.
- Double-wake check from WO-003, if not done yet.

## Not in V1
Serial, configuration from the Boron, hourly alignment, analog outputs, COUNTING patterns.

## Closing record (Claude Code fills in)
Budget vs actual (`src/main.cpp` lines): budget at most 80 total; actual 74 total (was 35). No other files in `src/`.
Build result: `pio run` succeeded (flash 1180 bytes, RAM 8 bytes). Not uploaded.
Host check result: ported `ledPin()` and the pulse rule unchanged (compiled the real `main.cpp` against a stub `Arduino.h`, scratchpad only, not committed) and printed every transition over one cycle at 1 ms steps. Matches the table: green 0 to 2 s; blue 30 to 32 s through pulse 9 (240 s); red 270 to 272 s; red 50 ms blinks at 275, 280, ..., 325 s; none at 330 s; blue 20 ms heartbeats from 360 s to 1170 s, 28 in total; 5 s to 25 s blinks between pulses 1 to 9 in the pulse's colour. No mismatches.
Notes:
- Power-up "armed" blink is a 200 ms blue from `setup()`, shown only while in the quiet period; IO1 stays LOW.
- Boundary case: the table says gap is `pos > LAST_RISE + DEBOUNCE_MS`, but `pos % 30000 < 20` would then blink at 330.001 to 330.019 s, contradicting "first heartbeat at 360 s". I start heartbeats at the first spacing boundary after the session ends (`HEARTBEAT_START_MS` = 360 000 ms), which gives the specified 28 heartbeats.
- Bench acceptance (scope, LED, EN power-cycle, oscillator, double-wake) is Chip's. Not pushed.
