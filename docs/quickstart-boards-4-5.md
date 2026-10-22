# Quickstart: bootloader and first sketch on a new exerciser board

For boards 4 and 5. Boards 1 to 3 were brought up the same way. The full detail is in `bring-up-guide.md` in this folder; read its "Gotchas" section once.

## What you need
- The exerciser board and its two JST cables (J2 6-pin, J3 4-pin).
- SparkFun Serial Basic (CH340C) set to 3.3 V.
- The UPDI adapter board (Serial Basic header to a 10-pin J1 for the Tag-Connect cable) and the Tag-Connect cable. This is custom; get it from Chip if you don't have one.
- Arduino IDE 1.8.13 with megaTinyCore 2.6.11 installed (not 2.6.8 or older).
- Python 3 with pymcuprog: `python3 -m venv ~/updi-env`, then `~/updi-env/bin/pip install pymcuprog`.
- VS Code with the PlatformIO extension, and this repo cloned.

## Windows differences
The steps below were written on a Mac. On Windows, change these (not yet tested on Windows):
- **Port:** a COM port (for example `COM5`) instead of `/dev/cu.*`. Find it in Device Manager under Ports (COM & LPT), or in the Arduino IDE's Port menu. Windows usually installs the CH340 driver itself; if the Serial Basic doesn't appear, install SparkFun's CH340 driver.
- **pymcuprog:** `py -m venv %USERPROFILE%\updi-env`, then `%USERPROFILE%\updi-env\Scripts\pip install pymcuprog`. Run it as `%USERPROFILE%\updi-env\Scripts\pymcuprog.exe ping -d attiny3226 -t uart -u COM5`, and the same pattern for the fuse read. In pymcuprog, `-b` is a byte count; baud is `-c`.
- **Arduino15 folder:** with the Arduino IDE from arduino.cc it is `C:\Users\<you>\AppData\Local\Arduino15`. Check that `packages\DxCore\tools\avrdude\6.3.0-arduino17or18\bin\avrdude.exe` and `packages\megaTinyCore\hardware\megaavr\2.6.11\avrdude.conf` exist there; if the folder name differs, use what you find. The Microsoft Store version of the IDE keeps these under Documents\ArduinoData instead.
- **`platformio.ini`:** set `upload_port = COM5` (your port) and make `upload_command` use the Windows paths above, with `avrdude.exe`. Quote the paths if your user name has spaces.
- **Keep your path edits out of git:** after editing, run `git update-index --skip-worktree platformio.ini` so they are never committed or overwritten by a pull.

## 1. Look and power
1. With a magnifier, check U5 for solder bridges, D1 notch at the pin 6 corner, Q1 orientation, J2/J3 seating.
2. Unpowered, check VCC to GND and V_IN to GND are not shorted.
3. The board is off until EN (J2 pin 4) is tied to GND. That is normal, not a fault. J2 pin 6 is IO2, not ground; use J2 pin 1.
4. Power from one source only: the Serial Basic through the adapter, or a bench supply on J3 pin 2, never both.

## 2. Contact over UPDI
Find the port (`ls /dev/cu.*` on a Mac), then ping:
```
~/updi-env/bin/pymcuprog ping -d attiny3226 -t uart -u <port>
```
Expect `1E9527`. Then read the fuses; a fresh chip reads `00 00 7E FF FF F6 FF 00 00`:
```
~/updi-env/bin/pymcuprog read -d attiny3226 -t uart -u <port> -m fuses
```
Never write `0x2` to the reset-pin setting. It disables UPDI.

## 3. Burn the bootloader (Arduino IDE 1.8.13)
Tools menu: Board = ATtiny3226/1626/826/426 w/Optiboot; Chip = ATtiny3226; Clock = 16 MHz internal; BOD mode = Disabled/Disabled; UART for bootloader = TX PB2, RX PB3 (default); UPDI/alt-RST pin = UPDI on PA0, RST on PB4; Bootloader entry = Default; Programmer = SerialUPDI - SLOW: 57600 baud; Port = the Serial Basic's port. Leave the other settings at their defaults (millis timer TCB1, startup 8 ms, BOD level 1.8 V). Choose Burn Bootloader and look for "Verify successful".

Read the fuses again. They must be exactly `00 00 01 FF FF FE 04 00 02`.

## 4. Upload the sketch with PlatformIO
1. Unplug the UPDI adapter and Tag-Connect cable. Wire the Serial Basic: GND to J3 pin 1, TXO to J3 pin 3, RXI to J3 pin 4, DTR to J2 pin 3, VCC to J3 pin 2; tie EN (J2 pin 4) to GND.
2. In `platformio.ini`, change `upload_port` and the two paths in `upload_command` (the avrdude binary and `avrdude.conf`) to match your Arduino15 folder. They currently point at Chip's Mac.
3. Open the project in VS Code (use "PlatformIO: New Terminal" if you need `pio`) and run Upload.
4. Success looks like `contains 1398 bytes` (or similar) and `bytes of flash verified`.
5. The LED should show the first pulse about 6 s after power-up, then repeat every 30 s, 10 times.

If the upload fails with "not in sync": check the DTR path (C11, R1), whether RX and TX are swapped, and that EN is tied low.

## 5. Report back to Chip, per board
- The ping response (`1E9527`).
- The fuse line after the burn.
- The avrdude verify line.
- What the LED does after upload.
- Anything that differed from the above, and the board number.
