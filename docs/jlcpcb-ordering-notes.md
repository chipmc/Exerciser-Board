# JLCPCB ordering notes (BOM, CPL, and the Serial Basic adapter)

Written 25 Oct 2026. Covers what worked for the exerciser board (first order) and the Serial Basic adapter board (ordered 25 Oct 2026, 5 boards, about $35 delivered). Items marked "unconfirmed" were not verified.

## Files JLCPCB needs
1. Gerbers.
2. BOM (CSV). Headers: `Comment, Designator, Footprint, LCSC Part #`. Put the part value in Comment (for example `220R`). One row per part; extra columns are not needed. Fusion's export (`Vendor`, `Vendor Part Number`, `Top/Bottom`) uses other names, so rename them or the part numbers may not be picked up.
3. CPL (CSV). Header exactly `Designator,Mid X,Mid Y,Layer,Rotation`. Coordinates as text with `mm` attached (`9.779mm`), Layer as `Top` or `Bottom` (not `TOP`), rotation in degrees. The origin must match the Gerber origin.

## Getting them out of Fusion Electronics
- Give every part the attributes `MANUFACTURER`, `MPN`, `VENDOR`, `VPN`, `DESCRIPTION`. `VPN` is the LCSC C-number. Parts from SnapEDA or Ultra Librarian use other field names and silently drop out of the BOM. After a library edit run Library, Update design from all libraries used in design; instance overrides must be edited directly.
- After any batch attribute edit, re-export the BOM. A batch edit once overwrote MPN and VPN on other resistors.
- CPL: File, Run ULP, `mount-smd-tht.ulp` (includes through-hole parts). The built-in File, Export, Mount SMD (`mnt`/`mnb`) disagreed with the ULP by about 1.2 mm on the JST connectors; the ULP was used. Fusion's built-in pick-and-place can default to mils even in a mm project: check one coordinate against a known part position.
- Reshape the CPL to the format above by hand.
- Leave out of the BOM: board outline, frames, fiducials, Tag-Connect pads (U3), solder-jumper footprints (JP1), and any part you hand-solder.

## Lessons from the two orders
- **Rotation:** both JST SR connectors (BM06B-SRSS-TB and BM04B-SRSS-TBT) came out 180 degrees wrong in JLCPCB's preview with the ULP value of 90. Fixing it in the preview was easy. Expect to add 180 for these parts next time and check the preview before approving.
- **D1, U5, Q1, D2:** check the rotation in the preview too. They came from three different library sources.
- **-TB versus -TBT:** `-TBT` has tape over the top for the pick-and-place nozzle. The untaped part is cheaper or in stock but riskier for machine placement. Check that the C-number, MPN and footprint name all agree. For J2, C160392 is the untaped BM06B (used on the exerciser board); the adapter uses the taped J2 (C495540) and taped J3 (C495539), confirmed against LCSC.
- **PCBA tier:** the exerciser board used Standard PCBA because D1 (RGB LED, MSL 5a) needs the 240 C reflow profile of Standard, not Economic's 255 C. The popup's "package 0201" reason is generic.
- **Through-hole parts:** JLCPCB has a through-hole assembly service, but the supporting tier, the cost and the limits are unconfirmed. A forum user got crooked connectors from it. Hand-soldering short headers (J1 on the adapter) was the choice.
- **Resistor values:** the BOM from Fusion had no value for the adapter's R1 ("Resistor Fixed - Generic"). Always check the LCSC page for the C-number and write the value in Comment. R1 on the adapter is 220 ohm 0603, confirmed against its LCSC part number.

## Serial Basic adapter board (J1 to the exerciser's J3 and J2)
Plugs the SparkFun Serial Basic (CH340C, set to 3.3 V) into the exerciser's two JST SR connectors for serial uploads. A 0.1 inch right-angle male header (J1) is hand-soldered; J2 (BM06B) and J3 (BM04B) and one resistor are assembled.

| J1 pin (Serial Basic label) | Goes to |
| --- | --- |
| 1 DTR | exerciser J2 pin 3 (DTR) |
| 2 RXI | exerciser J3 pin 4 (TX) |
| 3 TXO | exerciser J3 pin 3 (RX) |
| 4 VCC | exerciser J3 pin 2 (V_IN) |
| 5 CTS | not connected |
| 6 GND | J3 pin 1, J2 pin 1, J2 pin 4 (EN tied low = always on), shields |

Silkscreen reads Gnd, CTS, Vcc, TXO, RXI, DTR along the header to match the Serial Basic. Because Dupont-style connectors are not polarized, a 180 degree insertion swaps the pairs (DTR on GND, TXO on VCC). Mark pin 1, outline the programmer body, and keep the "Serial Basic must be 3.3 V" note on the board. A series resistor on the DTR line (R1, 220 ohm) is harmless because DTR is AC-coupled through 100 nF and protects the programmer if the header is flipped.
