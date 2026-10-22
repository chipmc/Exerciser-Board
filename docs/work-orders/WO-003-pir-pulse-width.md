# WO-003: Match the real PIR's pulse width (2 s high)

**Author:** Claude (architect, in the app)
**Executor:** Claude Code (workflow controller), Sonnet-class model. One-constant change.
**Device operations:** Chip only (upload, scope).
**Governing process:** Chip's AI_DEVELOPMENT_WORKFLOW.md (kept in the Generalized-Core-Counter repo; roles as in WO-002).
**Size budget:** 1 line changed in `src/main.cpp`, 0 lines added. Report budget vs actual in the closing record.

## Goal (plain language)
The real PIR output goes from 0 to 3.3 V on motion and stays at 3.3 V for 2 s before going low (Chip, 22 Oct 2026). The exerciser should do the same, so the baseline matches the real sensor.

## Change
- In `src/main.cpp` set `PULSE_HIGH_MS` from `200` to `2000`.
- Everything else stays as in WO-002: 10 pulses per cluster, 30 000 ms between pulse starts, 1 200 000 ms cycle, 6000 ms quiet at power-up.
- Update the WO-002 table row "Pulse high time" to read 2000 ms in `docs/work-orders/WO-002-v1-pir-pattern.md`, and note "changed by WO-003".
- Do not build for or upload to a device. Run `pio run` only if PlatformIO is available.

## Why this is safe (checked against the GCC answers doc)
- GCC counts a pulse on the rising edge (ISR sets a flag; `intPin` is RISING). A longer high time after the edge adds no new edges.
- The GCC doc asks for 100 ms to 1 s high; 2 s is outside that range but matches the real sensor. Chip is asking the GCC thread to confirm that debounce and the session timer run from the rising edge, not from the falling edge. If they run from the falling edge, each credited session grows by 2 s per pulse chain end, about 2 s per session.
- 2 s high is well under the 30 s spacing, so pulses never overlap.

## Done when (Claude Code)
- One line changed in `src/main.cpp`; WO-002 table updated; closing record filled in and committed. No push unless Chip says so.

## Verification (Chip, on the bench)
- Scope on J2 pin 5: each pulse is 3.3 V for about 2 s, then low; 30 s between pulse starts; 10 pulses per cluster.
- The red LED stays on for the full 2 s per pulse.

## Closing record (Claude Code fills in)
Budget vs actual (`src/main.cpp` lines): budget 1 changed, 0 added; actual 1 changed (`PULSE_HIGH_MS` 200 -> 2000), 0 added. Also 1 line changed in `docs/work-orders/WO-002-v1-pir-pattern.md` (table row, as specified).
Notes:
- `pio run` (from `~/.platformio/penv/bin/pio`, not on PATH) succeeded: flash 854 bytes, RAM 8 bytes. No upload; bench verification is Chip's.
- The pre-step commit message was reused as instructed, but `platformio.ini`, `src/main.cpp` and `bring-up-guide.md` were already committed in `5a64060`; commit `5b2ee26` therefore contains only the new WO-003 file.
- Not pushed.
