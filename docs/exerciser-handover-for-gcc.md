# Handover: the sensor exerciser (for the Generalized Core Counter thread)

## What it is
A small test device ("exerciser") that plugs into the same JST-SR cable as our sensors and generates interrupt pulses, so we can test the counter's occupancy firmware with a known, repeatable input. ATtiny3226 on a custom board, RGB status LED. Repo: the Exerciser-Board PlatformIO project (docs/bring-up-guide.md has the full detail).

## Status (22 Oct 2026)
- Board 1 of 5 is brought up: powers, bootloader burned, sketches upload over serial, LED and reset work.
- Measured about 2 mA steady at 16 MHz (design budget 2 mA), with unexplained brief peaks to about 8 mA.
- Firmware so far is only a color-order blinky and an IO-toggle test. No exerciser pulse pattern yet.
- PlatformIO upload is set up but not yet tried; the Arduino IDE upload works.

## Interface to the counter / Boron
- J2 (GPIO connector): pin 1 GND, pin 2 not connected, pin 3 DTR, pin 4 EN, pin 5 IO1 (PA6), pin 6 IO2 (PA4, through 1k R8; optional RC filter via JP1 for PWM-as-analog).
- J3 (serial connector): pin 1 GND, pin 2 V_IN (3.3V), pin 3 RX, pin 4 TX.
- EN is active-low: floating = board off, EN tied to GND = board on. So the Boron turns the exerciser on by driving EN low and off by releasing it.
- Both IO pins are 3.3V push-pull outputs from the exerciser.

## Plan
- V1 (MVP): a new sensor type "exerciser" for occupancy. Fixed repeating pattern: a cluster of pulses with intervals under 5 minutes, then about 10 minutes with none. This should drive the counter's start, debounce, timeout and reporting of occupied minutes. The Boron powers it down via EN when it sleeps at the end of the day. Result: a known baseline of occupied minutes by hour and by day; a firmware change that alters accuracy or precision shows up as a shift in that baseline. Cycle length should divide an hour evenly (20 or 30 minutes).
- V2: reconfigurable over serial from the Boron (target occupancy and visitation counts).
- V3: analog outputs (for example simulated soil moisture).

## What I need from the GCC firmware side
1. How is a sensor type defined and registered in the firmware? What would adding "exerciser" as an occupancy sensor type touch?
2. What does the occupancy interrupt input expect electrically: which pin, polarity, minimum pulse width?
3. The occupancy timings: debounce, how long after the last pulse before an occupancy period ends (timeout), and how occupied minutes are accumulated and reported by hour and by day. I need these to choose the pulse intervals and gap.
4. How and when does the Boron sleep and wake, and which pin would it use for EN?
5. Anything in how the counter handles the daily reset or start-up that the exerciser's power-up timing could trip.

## What happens next
Send the answers back and I will write WO-002 for the V1 pulse pattern, with a line budget, in the Exerciser-Board repo's docs/work-orders/. Separately, the GCC thread may want a short note in its own docs on how to use the exerciser as a regression baseline.
