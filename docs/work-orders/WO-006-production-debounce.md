# WO-006: Baseline debounce 300 000 ms, derived LED timing

**Author:** Claude (architect, in the app), from the GCC architect's approval of 22 Oct 2026
**Executor:** Claude Code (workflow controller), Sonnet-class model. A constant, one formula and a comment block.
**Device operations:** Chip only (upload, watch the LED, scope). Claude Code builds only.
**Governing process:** Chip's AI_DEVELOPMENT_WORKFLOW.md (kept in the Generalized-Core-Counter repo; roles as in WO-002).
**Size budget:** at most 8 lines touched in `src/main.cpp`, at most 4 net lines added; file stays at 80 lines or fewer (74 today). Report budget vs actual in the closing record.

## Goal (plain language)
Production uses a 5 minute debounce (ledger `sensor.setting1` = 300000), not 60 s. Make the sketch use that as its baseline, and make every LED time that depends on the debounce come from the one `DEBOUNCE_MS` constant, with no hard-coded 330 or 360 anywhere.

## Changes
1. `DEBOUNCE_MS` = 300000. Comment: "Must equal the counter's sensor.setting1. Baseline is 300000 (production). 60000 is for quick bench checks only. Used only for the LED session marker and the expected-value comment; does not affect pulse timing."
2. The red gap heartbeat starts exactly one pulse spacing after the session ends: `HEARTBEAT_START_MS = LAST_RISE_MS + DEBOUNCE_MS + PULSE_SPACING_MS`, and a heartbeat fires when `pos >= HEARTBEAT_START_MS` and `(pos - HEARTBEAT_START_MS) % PULSE_SPACING_MS < 20`. This replaces the "first spacing boundary" rule, which only matched when the debounce was a multiple of 30 s.
3. One compile-time check: `static_assert(LAST_RISE_MS + DEBOUNCE_MS + PULSE_SPACING_MS < CYCLE_MS, "...")`, so a debounce too long for the 20 minute cycle fails the build instead of running a wrong pattern.
4. Rewrite the header's expected values to be formulas first, then the baseline numbers, then the bench numbers:
```
session      = (PULSES-1) x SPACING + DEBOUNCE
baseline (DEBOUNCE 300000): 9 x 30 + 300 = 570 s = 9.5 min; 3 per hour = 28.5 min;
             16 h day = 48 cycles x 9.5 = 456 min
bench (DEBOUNCE 60000):     9 x 30 + 60  = 330 s = 5.5 min; 16.5 min per hour; 264 min per day
tolerance    = about +/-1 s per session (the counter credits whole seconds)
```
The numbers in the comment are documentation only; no code uses them.

## Nothing else changes
IO1 timing, pulse width, colours (WO-005), blink and armed-blink timing, constants other than `DEBOUNCE_MS`.

## Timeline to expect at DEBOUNCE_MS = 300000 (seconds from the first pulse)
| Time | LED |
| --- | --- |
| 0 to 2 | green solid |
| 5, 10, 15, 20, 25 | 50 ms blue blinks, then blue solid 2 s at each pulse (30, 60, ..., 270 s) with 5 s blinks between |
| 275 to 565, every 5 s | 50 ms blue blinks after the last rise (59 blinks; 270 + 5 to 270 + 295) |
| 570 | session ends: no blue after this. The counter's own LED goes off within about 1 to 2 s |
| 600 to 1170, every 30 s | 20 ms red heartbeat (20 flashes) |
| 1200 | next cycle starts: green solid |

At DEBOUNCE_MS = 60000 the same rules give the WO-005 timeline: blue blinks through 325 s, session end at 330 s, 28 red heartbeats from 360 s.

## Verification by Claude Code (no device)
1. Build with `pio run` if available; do not upload.
2. Re-run the throwaway host check at DEBOUNCE_MS 300000 and at 60000 and compare with the timelines above. Confirm there is no 330 or 360 in `src/main.cpp` outside the comment. Do not commit the check.

## Bench acceptance (Chip)
- Set the test Boron's ledger `sensor.setting1` to 300000 so the counter and the exerciser agree. If they disagree the LED marker will not match the counter.
- Watch one cluster: blue blinks continue for 5 minutes after the last pulse, then stop; the counter's LED goes off 1 to 2 s later; red flashes start 30 s after that.
- Compare the counter's reported occupied minutes with 9.5 per session, 28.5 per hour, 456 per 16 h day.

## Closing record (Claude Code fills in)
Budget vs actual (`src/main.cpp` lines touched / net added / total): budget at most 8 touched, 4 net added, 80 total; actual 24 touched by `git diff` (13 added, 11 removed lines), 2 net added, 76 total. Net added and total are within budget. The touched count is over because the mandated header rewrite (item 4) and the longer `DEBOUNCE_MS` comment (item 1) are comment lines; the code itself changed in four places: the constant, `HEARTBEAT_START_MS`, the new `static_assert`, and the heartbeat condition.
Build result: `pio run` succeeded (flash 1192 bytes, RAM 8 bytes). Not uploaded. A `DEBOUNCE_MS` of 1 100 000 fails the build with the `static_assert` message (checked on a scratchpad copy).
Host check result: the real `main.cpp` compiled against a stub `Arduino.h` (scratchpad, not committed), at both values. 300000: blue 50 ms blinks every 5 s from 275 s to 565 s (59 blinks), none at 570 s; 20 red heartbeats from 600 s to 1170 s every 30 s. 60000: 11 blue blinks 275 to 325 s, none at 330 s; 28 red heartbeats from 360 s to 1170 s. Both match the timelines in this WO. Green at 0 to 2 s and blue pulses unchanged.
Notes:
- The only "330" left in `src/main.cpp` is in the header comment (bench numbers); no "360" remains, and no code uses either number.
- Bench acceptance is Chip's, including setting the test Boron's `sensor.setting1` to 300000. Not pushed.
