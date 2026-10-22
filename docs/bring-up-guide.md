# Exerciser Board Bring-Up Guide

Oct 21, 2026 · Chip McClelland (updated Oct 22, 2026)

This guide takes an assembled exerciser board from first inspection to a blinky sketch uploaded over serial; every step was run on board 1 on 21 Oct 2026 unless marked untested.

## What you need

The board is programmed over UPDI once (bootloader and fuses), then over serial for every sketch after that.

| Item | Detail |
| --- | --- |
| Bench supply | 3.3V, current limit about 25 mA |
| Meter | Series mA range for idle current; a scope helps for the IO pins |
| USB-serial adapter | SparkFun Serial Basic (CH340C), left on its default 3.3V setting |
| UPDI adapter board | Serial Basic header to a 10-pin J1 for the Tag-Connect cable; TX goes through R1 to the UPDI line, RX straight onto it. R1 on board 1 is 5.1k (marked 512) and worked; 470 ohm to 1k is the documented range |
| Cable to U3 | Tag-Connect footprint on the board: UPDI on pin 1, VCC on pin 2, GND on pin 6 |
| Mac tooling | Python venv with pymcuprog: `python3 -m venv ~/updi-env`, then `~/updi-env/bin/pip install pymcuprog` |
| Arduino IDE | Version 1.8.13 with megaTinyCore 2.6.11. Do not use megaTinyCore 2.6.8 or older: its README says early Optiboot builds ignored the entry-condition setting |

The AVR Pocket Programmer will not work: it is an ISP (SPI) tool and the ATtiny3226 only programs over UPDI.

## Step 1: Inspect and apply first power

With a magnifier, check U5 for solder bridges between pins, D1 for its notch at the pin 6 corner (pin 1 is top-right on the layout), D2 for its band on the VCC side (cathode to VCC), Q1 for orientation, and J2 and J3 for seating. With the board unpowered, check that VCC to GND and V_IN to GND are not shorted.

Connector pin maps:

| Pin | J2 (GPIO connector) | J3 (serial connector) |
| --- | --- | --- |
| 1 | GND | GND |
| 2 | not connected | V_IN |
| 3 | DTR | RX |
| 4 | EN | TX |
| 5 | IO1 (PA6) | |
| 6 | IO2 (PA4) | |

J2 pin 6 is IO2, not ground. Use J2 pin 1 or the shield pads for ground.

First power on board 1:

1. Set the bench supply to 3.3V with a 25 mA limit and connect it to J3 pin 2 (V_IN) and pin 1 (GND).
2. Leave EN floating. The board stays off and the supply shows 0 mA, because R3 pulls Q1's gate up. This is the normal off state, not a fault.
3. Tie EN (J2 pin 4) to GND. Q1 turns on and VCC comes up. VDD and GND at U5 pins 1 and 20 measured 3.3V.
4. Write down the idle current. On board 1 it was not recorded at first power; see Step 5.

## Step 2: Make first UPDI contact

Power the board from the Serial Basic through the adapter alone. Switch the bench supply off and leave J3 empty: Q1's body diode lets VCC feed back toward V_IN, and two 3.3V sources must never be tied together.

Find the port with `ls /dev/cu.*`; on board 1 it was `/dev/cu.usbserial-110`. Then ping, which only reads:

```
~/updi-env/bin/pymcuprog ping -d attiny3226 -t uart -u /dev/cu.usbserial-110
```

Expect the device ID `1E9527`. That confirms the adapter, R1, R5 on the board, the cable to U3 and U5's UPDI joint all work. Then read the fuses, still without writing:

```
~/updi-env/bin/pymcuprog read -d attiny3226 -t uart -u /dev/cu.usbserial-110 -m fuses
```

A fresh chip reads `00 00 7E FF FF F6 FF 00 00`. Byte 5 is SYSCFG0, and its bits 3:2 (RSTPINCFG) are `01`, meaning UPDI on PA0.

The bootloader burn in Step 3 writes SYSCFG0 itself, so setting it by hand is optional. If you want to, write `0xFE` to byte 5, which makes PB4 the alternate reset pin while keeping UPDI on PA0, then read back and ping again:

```
~/updi-env/bin/pymcuprog write -d attiny3226 -t uart -u /dev/cu.usbserial-110 -m fuses -o 5 -l 0xFE
```

Never write `0x2` to RSTPINCFG. It disables UPDI, and getting it back needs a high-voltage programmer.

## Step 3: Burn the bootloader

Use Arduino IDE 1.8.13 with the adapter still connected and the board powered from it. Set the Tools menu exactly as below, then choose Burn Bootloader.

| Tools menu item | Setting |
| --- | --- |
| Board | ATtiny3226/1626/826/426 w/Optiboot |
| Chip | ATtiny3226 |
| Clock | 16 MHz internal |
| millis()/micros() timer | Enabled (default, TCB1) |
| Startup time | 8ms |
| BOD voltage level | 1.8V (5 MHz or less) |
| BOD mode | Disabled/Disabled |
| UART for bootloader | TX PB2, RX PB3 (UART0 default pins) |
| UPDI/alt-RST pin | UPDI on PA0, RST on PB4 |
| Bootloader entry | Default; the burn loaded `optiboot_tx2z_extr.hex`, the external-reset variant |
| Programmer | SerialUPDI - SLOW: 57600 baud |
| Port | the Serial Basic's port |

The burn log on board 1 showed these fuse writes: `0:0x00 1:0x00 2:0x01 5:0b11111110 6:0x04 7:0x00 8:0x02`. That means 16 MHz (OSCCFG 0x01), PB4 as alternate reset (SYSCFG0 0xFE), 8ms startup (SYSCFG1 0x04) and a 512-byte boot section (BOOTEND 0x02). It then erased the chip, wrote 472 bytes of bootloader and ended with "Verify successful".

After the burn, the fuses should read `00 00 01 FF FF FE 04 00 02`. Boards 1 to 3 all read back exactly this on 22 Oct 2026 (the unused bytes after the nine fuses print as `xx`), so the burn does write the fuses.

Once the bootloader is in, Upload Using Programmer is refused on Optiboot boards. Every later sketch goes over serial.

## Step 4: Upload a sketch over serial

This proves the DTR reset path, the RX and TX wiring and the bootloader entry in one step. Unplug the UPDI adapter and the Tag-Connect cable first.

| Serial Basic pin | Board pin |
| --- | --- |
| GND | J3 pin 1 (or J2 pin 1) |
| TXO | J3 pin 3 (RX) |
| RXI | J3 pin 4 (TX) |
| DTR | J2 pin 3 (DTR) |
| VCC | J3 pin 2 (V_IN), with EN (J2 pin 4) tied to GND |

Power the board from one source only. If you use the bench supply on V_IN instead, leave the Serial Basic's VCC unconnected.

In the Arduino IDE, keep the Step 3 settings, select the Serial Basic's port and press the normal Upload button. The IDE pulses DTR when it opens the port; the pulse resets PB4 and starts the bootloader's 1-second window.

D1's three channels use these pins:

| Color | D1 pin | MCU pin | Resistor |
| --- | --- | --- | --- |
| Red | 1 | PC3 | R4 (620 ohm) |
| Green | 3 | PC2 | R6 or R7 (200 ohm) |
| Blue | 5 | PC1 | R6 or R7 (200 ohm) |

First sketch: pulses red, green, blue in turn (also in `src/main.cpp`). If the colors appear in that order, D1's rotation and pin map are right. Each channel draws about 2 mA when on, so it is pulsed:

```cpp
const uint8_t LED_R = PIN_PC3;
const uint8_t LED_G = PIN_PC2;
const uint8_t LED_B = PIN_PC1;

void pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delay(20);
  digitalWrite(pin, LOW);
}

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void loop() {
  pulse(LED_R); delay(480);
  pulse(LED_G); delay(480);
  pulse(LED_B); delay(480);
}
```

Second sketch: drives both IO pins at the same time, with the LED mirroring them (red follows IO1, green follows IO2). The pins toggle at different rates so they can be told apart on a scope:

```cpp
const uint8_t LED_R = PIN_PC3;
const uint8_t LED_G = PIN_PC2;
const uint8_t IO1   = PIN_PA6;   // J2 pin 5
const uint8_t IO2   = PIN_PA4;   // J2 pin 6, through R8 (1k)

const uint32_t IO1_HALF_MS = 1000;
const uint32_t IO2_HALF_MS = 1500;

bool io1State = false, io2State = false;
uint32_t lastIo1 = 0, lastIo2 = 0;

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(IO1, OUTPUT);
  pinMode(IO2, OUTPUT);
}

void loop() {
  uint32_t now = millis();
  if (now - lastIo1 >= IO1_HALF_MS) {
    lastIo1 = now;
    io1State = !io1State;
    digitalWrite(IO1, io1State);
    digitalWrite(LED_R, io1State);
  }
  if (now - lastIo2 >= IO2_HALF_MS) {
    lastIo2 = now;
    io2State = !io2State;
    digitalWrite(IO2, io2State);
    digitalWrite(LED_G, io2State);
  }
}
```

If an upload fails with a "not in sync" error, check the DTR path (C11 and R1), whether RX and TX are swapped, and the 1-second entry window.

## Step 5: Bench checks

Run these with the first sketch loaded. Board 1 passed everything marked confirmed; the rest were not measured.

| Check | Expected | Board 1 |
| --- | --- | --- |
| EN floating | Board off, 0 mA | Confirmed |
| EN tied to GND | Board on, sketch runs | Confirmed |
| LED order | Red, green, blue | Confirmed |
| SW2 pressed | Sketch restarts (PB4 reset) | Confirmed |
| DTR upload | Sketch uploads over serial | Confirmed |
| Idle current, sketch running | Record the number and compare it with the 2 mA design budget | About 2 mA steady (meter), with brief peaks to about 8 mA that are not yet explained |
| IO1 at J2 pin 5, IO test sketch | About 3.3V for 1 s, about 0V for 1 s | Not yet measured |
| IO2 at J2 pin 6, IO test sketch | About 3.3V for 1.5 s, about 0V for 1.5 s | Not yet measured |
| IO2 through the JP1 filter (R8 1k, C2 4.7 uF) with PWM | Smoothed level that follows the duty cycle | Not tested |

For the idle current, use the first sketch (the pulsed LED). The second sketch holds the LED on, which adds about 2 mA.

Boards 2 and 3 (22 Oct 2026): bootloader burned, fuses read back correctly, and a simple sketch uploaded over serial from the Arduino IDE on all three boards 1 to 3. The PlatformIO upload has not been tried yet.

## Step 6: Develop in PlatformIO

VS Code with PlatformIO builds and uploads through the bootloader on board 1 (confirmed 22 Oct 2026: build, upload and verify work, and the V1 pulse pattern runs). Keep Arduino IDE 1.8.13 for the one-time bootloader burn.

PlatformIO's default settings do not work for an Optiboot ATtiny3226. Each setting below is needed; the symptom without it is in brackets:

- **Newer toolchain** (`platformio/toolchain-atmelavr@~3.70300.0`): the default avr-gcc has no device spec for the chip [error `device-specs/specs-attiny3226: No such file`].
- **`-DCLOCK_SOURCE=0`**: tells megaTinyCore to use the internal oscillator [build error `CLOCK_SOURCE not defined`].
- **`-Wl,--section-start=.text=0x200`**, plus `build_unflags` removing the builder's own `.text=0x0`: the program must start just past the 512-byte bootloader [avrdude verify fails at byte 0, and the hex file is 886 bytes instead of about 1398 because it starts at address 0].
- **`-DUSING_OPTIBOOT`**: the Arduino IDE defines it for Optiboot boards. Without it the program sees the zeroed reset flags the bootloader leaves, resets itself, and loops before `setup()` runs [the LED never lights].

```ini
[env:ATtiny3226]
platform = atmelmegaavr
board = ATtiny3226
framework = arduino
board_build.core = megaTinyCore
board_build.f_cpu = 16000000L
board_hardware.oscillator = internal
board_hardware.bod = disabled
build_flags =
    -DCLOCK_SOURCE=0
    -Wl,--section-start=.text=0x200
    -DUSING_OPTIBOOT
build_unflags = -Wl,--section-start=.text=0x0
platform_packages =
    platformio/toolchain-atmelavr@~3.70300.0
upload_protocol = custom
upload_port = /dev/cu.usbserial-110
upload_command = /Users/chipmc/Library/Arduino15/packages/DxCore/tools/avrdude/6.3.0-arduino17or18/bin/avrdude -C/Users/chipmc/Library/Arduino15/packages/megaTinyCore/hardware/megaavr/2.6.11/avrdude.conf -v -pattiny3226 -carduino -D -P$UPLOAD_PORT -b115200 -Uflash:w:$SOURCE:i
```

The upload line is the command the Arduino IDE ran on board 1, with the port and hex path replaced by PlatformIO's variables. The `-D` flag skips the chip erase, which is why the bootloader survives. It depends on the Arduino IDE's megaTinyCore 2.6.11 files, so keep that install. PlatformIO builds from `src/main.cpp`, so a sketch needs `#include <Arduino.h>` at the top and its functions declared before use. A normal Mac terminal does not have `pio` on its path; use "PlatformIO: New Terminal" in VS Code or `~/.platformio/penv/bin/pio`.

Do not upload from PlatformIO over UPDI with pymcuprog on a board that has the bootloader. That path used `--erase` in the forum thread, which is a full chip erase and would remove the bootloader.

## Firmware plan for the exerciser

- **Version 1, fixed occupancy pattern.** A new sensor type called "exerciser". It repeats a cluster of interrupt pulses with intervals under 5 minutes, then about 10 minutes with none, which drives the counter through start, debounce, timeout and reporting of occupied minutes. The Boron turns the exerciser off through EN when it sleeps at the end of the day. The result is a known baseline of occupied minutes by hour and by day; a firmware change that alters the counter's accuracy or precision shows up as a shift in that baseline.
- **Choose the cycle length to divide an hour evenly** (for example 20 or 30 minutes) so every hourly bucket sees the same pattern.
- **Check the exerciser's clock drift.** It runs from the internal oscillator, not a crystal; compare it against the Boron over several days before trusting the baseline.
- **Match the real sensor's pulse** (width, polarity and which IO pin) when setting the pulse shape, and sleep between pulses to cut current.
- **Version 2, reconfigurable.** A general sketch the Boron configures over the serial connector, for example by sending target occupancy and visitation counts so the exerciser changes its interrupt cadence to match.
- **Version 3, analog outputs.** Simulate sensors such as soil moisture with PWM through the JP1 filter on IO2. Check first whether the real sensors give a voltage, a frequency or a digital signal.

Baseline run conditions (from the GCC firmware thread, 22 Oct 2026): the counter's debounce and session timers run from the rising edge of each pulse, as handled by the firmware. Awake, that is within milliseconds. Asleep, a pulse wakes the device and the wake path can wait 1 to 2 s for USB serial to re-enumerate when the `serialConnected` ledger setting is on. Times are whole RTC seconds. A session runs from the first pulse to the last pulse plus the debounce. For baseline runs, set `serialConnected` off on the test Boron so it behaves like the field.

## Gotchas from board 1

- **A dead-looking board is usually EN floating.** R3 holds Q1 off until EN is pulled low, so 0 mA at first power is normal. The carrier must actively drive EN low before the exerciser answers.
- **J2 pin 6 is IO2, not GND.** Grounding EN through pin 6 leaves it floating.
- **In pymcuprog, `-b` is the number of bytes to read, not baud.** Baud is `-c`. On a fuse read, `-b 57600` fails with a bytes-out-of-range error. The ping ran at the tool's default speed, which worked through the 5.1k R1.
- **Don't read the Optiboot menu names as a promise.** The "Default" bootloader entry resolved to the external-reset variant on board 1; open the submenu and read the options before relying on it.
- **Old megaTinyCore builds have bad bootloaders.** The core's own notes say 2.6.8 and older should not be used. A forum thread said PlatformIO's packaged core was 2.6.7 when it was written; check the version before moving the bootloader work there.
- **Arduino IDE 1.8.13 is the version the core's README recommends.** The same README says 2.0.x and some 2.1 releases do not work correctly.
- **The IDE console may show "Error downloading http://drazzy.com/package_drazzy.com_index.json".** The core was already installed and worked, so it was a failed index refresh; confirm the installed version in Boards Manager.
- **Power from one source at a time.** The adapter passes the Serial Basic's 3.3V to the board, so switch the bench supply off while using it.
- **Never write 0x2 to RSTPINCFG.** It disables UPDI.
- **JLCPCB's first U5 footprint warning was a fault in their part data, not in the board.** U5 uses the SSOP20_SS_MCH-L footprint and works on board 1 (device ID read and bootloader burn both succeeded). For a future spin, Microchip's recommended land pattern for the 20-pin SSOP (drawing C04-2072B) is 0.65 mm pitch, 7.20 mm pad spacing, and 0.45 mm by 1.75 mm pads.

## Not yet done

- Burn the bootloader on boards 4 and 5 and read their fuses back (boards 1 to 3 are done and matched).
- Explain the brief 8 mA peaks: one LED channel should add only about 2 mA, so check for a meter peak-hold catching a reset or the bootloader window, or two channels on at once. The 2 mA steady reading is at 16 MHz and matches the design budget. At 1 MHz the datasheet's active-current numbers suggest roughly 0.3 to 0.6 mA (an estimate from the 5 MHz and 10 MHz table values, not measured); sleeping between pulses should save more than slowing the clock.
- Measure IO1 and IO2 at J2 pins 5 and 6, and test the JP1 filter with a PWM ramp on IO2.
- Write the serial command protocol, the pulse patterns and the `SENT n` reporting.
- Test low-power behavior for the solar and battery runs, including the effect of the brown-out detector, which is currently disabled.
- Try Particle's Asset OTA path from the Boron. This depends on serial uploads being reliable, which they were on board 1.
- ESD protection was left off this first spin on purpose.
- Repeat Steps 1 to 5 on boards 2 to 5 and note any differences from board 1.
