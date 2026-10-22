# WO-002: V1 exerciser firmware: a repeating PIR-style pulse pattern

**Author:** Claude (architect, in the app)
**Executor:** Claude Code (workflow controller), Sonnet-class model. Small, self-contained firmware change; no hardware access needed.
**Device operations:** Chip only (upload, scope, counter integration). Claude Code builds only.
**Governing process:** Chip's AI_DEVELOPMENT_WORKFLOW.md (kept in the Generalized-Core-Counter repo; this repo has no copy yet). Roles here: Claude = architect writes the WO, Claude Code executes, Chip approves and does all device operations.
**Size budget:** `src/main.cpp` at most 45 lines in total (it is 23 today). No other `src/` files, no libraries. Report budget vs actual in the closing record.

## Goal (plain language)
Make the exerciser look exactly like a PIR sensor to the Generalized Core Counter, repeating the same pulse pattern forever, so occupied minutes by hour and day become a known baseline we can compare firmware versions against.

## Source of the numbers
Everything below comes from the GCC repo's `docs/handover/2026-10-22-exerciser-interface-answers.md` (GCC main `cf0edf7`, v45): PIRSensor.h:41-50, 76-80, 127, 136-166; State_Common.h:318-358; Config.h:34. Do not change the numbers without going back to that doc.

## Hardware facts
- Output pulses on IO1 = PA6 = `PIN_PA6` (J2 pin 5). Active-high, rising-edge. Idle LOW.
- IO2 (PA4) is not used in V1: leave it as an input. Do not drive it.
- The counter powers the exerciser through EN (low = on). Every power-up is a fresh start of the pattern; there is no clock to keep.
- Red LED (`PIN_PC3`) mirrors each pulse so a pulse is visible on the bench. Other LED channels unused.

## Pattern (one 20-minute cycle, repeating forever)
| Item | Value | Why |
| --- | --- | --- |
| Quiet time at start | IO1 LOW for 6000 ms after `setup()` begins, then the first cluster starts | GCC wants at least 5 s after EN goes low. Power-on reset runs the sketch straight away, so 6 s gives 1 s of margin |
| Pulses per cluster | 10 | |
| Pulse high time | 2000 ms (changed by WO-003) | GCC wants 100 ms to 1 s; real PIR holds 2 s |
| Spacing between pulse starts | 30 000 ms | Must be well under the 60 s debounce, and over 500 ms |
| Cluster length | 270 000 ms (9 spacings, first to last pulse start) | |
| Cycle length | 1 200 000 ms (20 min) measured from one cluster start to the next | Divides the hour, so every hour is identical |
| Gap with no pulses | 930 000 ms after the last pulse start (15.5 min) | Long enough for the session to close about 60 s after the last pulse |

Expected result at the counter (GCC's estimate): about 5.5 min credited per session, 3 sessions per hour, about 16.5 min per hour, about 264 min over a 06:00 to 22:00 day.

## Implementation rules
1. Schedule from absolute times: compute each pulse time from the cluster start (`clusterStart + n * 30000`) and the next cluster from `clusterStart + 1200000`. Do not use chained `delay()` calls, which accumulate error.
2. `millis()` wraps after about 49 days; at one day per power-up this does not matter, but use unsigned subtraction (`now - start >= interval`) as in the existing IO test sketch.
3. Name every number above as a `const` at the top of the file so Chip can retune without reading the logic.
4. In `setup()`: set IO1 to output and LOW first, before anything else.
5. Do not add sleep modes, serial, or any V2 feature. Keep it the simplest thing that meets the table.
6. Do not build for or upload to a device. Run `pio run` only if PlatformIO is available; report if not.

## Done when (Claude Code)
- `src/main.cpp` implements the table, within the line budget, and builds if PlatformIO is available.
- Constants block matches the table values exactly.
- Closing record filled in, committed, not pushed unless Chip says so.

## Verification (Chip, on the bench, after upload)
1. With a scope or logic analyzer on J2 pin 5: IO1 stays LOW for about 6 s after EN goes low, then pulses.
2. Each pulse is about 200 ms high; 10 pulses 30 s apart; then silence until the next cluster 20 min after the first.
3. Idle level is LOW, and the red LED flashes with each pulse.
4. Power-cycle through EN: the pattern restarts from the quiet period.
5. Integration: counter configured as PIR (`sensor.type = 1`, default 60 s debounce), exerciser on the sensor cable. Check that sessions credit about 5.5 min each and about 16.5 min per hour, and record the first clean days as the baseline.

## Open items for Chip (not part of this WO)
- Confirm the carrier's sensor connector maps `intPin` (D13) to the exerciser's J2 pin 5 (IO1) and `disableModule` (D12) to J2 pin 4 (EN). See the architect's notes in the thread.
- Check what the carrier drives on the cable pins that land on the exerciser's J2 pin 3 (DTR, AC-coupled to the exerciser's reset) and pin 6 (IO2). A carrier signal toggling on pin 3 could reset the exerciser; a carrier output on pin 6 would fight IO2.
- The exerciser's own draw (about 2 mA at 16 MHz) comes from the carrier's V_IN rail, so it adds to any power-budget measurement of the counter. If that matters, a later WO adds sleep.
- Oscillator drift: the exerciser's cycle runs from the internal oscillator. Over a 16 hour day, 1% drift is about 10 minutes, and a late session could cross the top of the hour if drift passes about 14 minutes. Compare the first cluster and the last cluster of a day against the clock and tune the cycle constant if needed.

## Closing record (Claude Code fills in)
Budget vs actual (`src/main.cpp` lines): 36 of 45 (was 23). No other `src/` files, no libraries.
Build result: NOT BUILT. `pio run` is installed but fails on the toolchain, not on this code: `avr-g++: error: device-specs/specs-attiny3226: No such file or directory`, raised for every file including the megaTinyCore framework sources, so the installed avr-gcc package has no ATtiny3226 device specs. Needs a toolchain fix on Chip's side (or a build from the Arduino IDE) before upload. Nothing was uploaded.
Notes:
- Constants block matches the table: quiet 6000 ms, 10 pulses, 200 ms high, 30000 ms spacing, 1200000 ms cycle. The 270 000 ms cluster length and 930 000 ms gap follow from them and are not separate constants.
- Scheduling is by absolute time: `elapsed = millis() - clusterStart`, pulse n is high while `elapsed / 30000 == n` (n < 10) and `elapsed % 30000 < 200`; the next cluster is `clusterStart += 1200000`, so no drift accumulates from the code. Unsigned subtraction throughout; the quiet time is `(int32_t)elapsed < 0`.
- `setup()` sets IO1 to output and LOW first. IO2 is not touched. The red LED mirrors IO1. No serial, no sleep.
- Not verified on hardware: all of the Verification section is Chip's.
