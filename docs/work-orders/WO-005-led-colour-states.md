# WO-005: LED colours show the session state (green / blue / red)

**Author:** Claude (architect, in the app), at Chip's request (22 Oct 2026)
**Executor:** Claude Code (workflow controller), Sonnet-class model. A colour remap of about 4 lines.
**Device operations:** Chip only (upload, watch the LED). Claude Code builds only.
**Governing process:** Chip's AI_DEVELOPMENT_WORKFLOW.md (kept in the Generalized-Core-Counter repo; roles as in WO-002).
**Size budget:** at most 4 lines changed in `src/main.cpp` (lines 34 to 43 today), 0 lines added; the file stays at 74 lines or fewer. Report budget vs actual in the closing record.

## Goal (plain language)
Make the colour tell you the counter's state at a glance:
- **Green** = a new occupancy session is starting (the first pulse).
- **Blue** = the rest of the occupied period.
- **Red** = short flashes between occupied sessions (the gap).

This replaces the colour scheme from WO-004 (first pulse green, pulses 2 to 9 blue, last pulse red, red blinks after it, blue heartbeat in the gap). The GCC spec asked for the last pulse to be red; Chip has chosen this simpler scheme instead. Timing is unchanged.

## Change (in `ledPin()`)
| When | Was (WO-004) | Now |
| --- | --- | --- |
| First pulse, solid 2 s (k = 0, r < PULSE_HIGH_MS) | green | green (unchanged) |
| Blinks after the first pulse (k = 0, every 5 s) | green | blue |
| Pulses 2 to 9, solid and blinks | blue | blue (unchanged) |
| Pulse 10, solid 2 s (tl < PULSE_HIGH_MS) | red | blue |
| Red blinks after the last rise, tl < DEBOUNCE_MS | red | blue |
| Gap heartbeat, 20 ms every 30 s from HEARTBEAT_START_MS | blue | red |
| 200 ms "armed" blink at power-up | blue | blue (unchanged) |

Nothing else changes: IO1 timing, pulse widths, blink and heartbeat timing, constants.

## Result to expect
- Green solid 2 s at the start; then blue for the rest of the session: pulse 1's blinks, pulses 2 to 10 solid, and blue blinks every 5 s until 60 s after the last rise.
- The blue blinks stop at 330 s from the first pulse (the session end). The first red heartbeat comes at 360 s, then every 30 s until the next cluster.
- Sessions start at 0, 20 and 40 min each hour.

## Verification by Claude Code (no device)
1. Build with `pio run` if available; do not upload.
2. Re-run the throwaway host check from WO-004 with the new colours; confirm only the colour column changed. Do not commit it.

## Bench acceptance (Chip)
- Watch one cluster: green solid first, then only blue until the blinks stop, then red flashes every 30 s.
- Scope on J2 pin 5 is unchanged from WO-004 (2.00 s highs, 30.0 s rise to rise).

## Closing record (Claude Code fills in)
Budget vs actual (`src/main.cpp` lines changed / total): budget at most 4 changed, 0 added, 74 or fewer total; actual 4 changed, 0 added, 74 total.
Build result: `pio run` succeeded (flash 1176 bytes, RAM 8 bytes). Not uploaded.
Notes:
- The four changed lines in `ledPin()`: the colour choice for pulses 1 to 9 (green only for the first pulse's solid 2 s, blue otherwise, so its 5 s blinks are now blue), pulse 10 solid (blue), post-session blinks (blue), gap heartbeat (red).
- Host check (WO-004 harness, scratchpad, not committed) re-run over one cycle: all transition times and IO1 states are identical to WO-004; only colours changed. Green 0 to 2 s; blue everywhere else in the session, including pulse 10 and blinks through 325 s; nothing at 330 s; 28 red 20 ms heartbeats from 360 s to 1170 s.
- Bench acceptance is Chip's. Not pushed.
