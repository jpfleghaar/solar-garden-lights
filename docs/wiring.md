# Wiring Diagram

Board, battery, H-bridge driver, and LED string in one picture — including the two spots where it's easy to get confused: `VIN` only charges the battery, and the H-bridge draws its own power straight from the battery, not from the board.

## Schematic

![Wiring diagram: USB or solar charges only the battery via VIN. The battery powers both the board over JST and, as a separate tap, the H-bridge directly. The board controls the H-bridge only through the logic lines GPIO14 (PWM) and GPIO6 (EEP). The H-bridge's IN3 is hardwired to GND. Battery, board, and H-bridge share one common ground. The H-bridge switches the raw battery voltage through to the LED string.](wiring-diagram.svg)

**Key takeaway:** only the battery leads power the H-bridge directly (thick lines above). `VIN` exclusively charges the battery and has no connection of its own to the driver stage. Battery, board, and H-bridge still share one common ground — otherwise GPIO14/GPIO6 wouldn't register as valid logic levels at the driver at all.

## Pinout Detail

| From | To | Kind | Note |
|---|---|---|---|
| GPIO14 | IN4 | Logic | PWM, 20 kHz, 8-bit — controls brightness |
| GPIO6 | EEP | Logic | Enable/standby, LOW = driver output stage fully off |
| — | IN3 | GND | Hardwired, no GPIO — there is no `PIN_DIR` in the firmware |
| Battery + | VCC / VM | Power | Raw tap straight off the battery — not via VIN or 3V3 |
| Battery −, board GND, driver GND | common node | GND | Must be the same ground everywhere |
| OUT4 | LED string + | Power | Switched, raw battery voltage (0–100% PWM) |
| OUT3 | LED string − | Power | Return path, since IN3 is hardwired LOW |
| USB-C / 5V / Solar | VIN | Charging input | Charges only the battery via the board's own charge circuit |

## Important to Know

1. **VIN is not the H-bridge's power supply.** VIN only accepts 5V for charging (USB-C, DC, or solar) and has no connection to the driver stage.
2. **3V3 doesn't work either** — a fixed 3.3V regardless of how full the battery is. The firmware specifically compensates for the real, sagging battery voltage (`readBatteryMV()`, `LED_TARGET_MV`), so the raw battery voltage has to actually reach the driver.
3. **At a full battery (4.2V)**, the full battery voltage briefly reaches the LED string, which is only rated for 3.2–3.5V. The firmware now caps this at 3.5V (`LED_TARGET_MV`), but that hasn't been verified with a multimeter on the real hardware yet.
4. **Driver model unconfirmed.** If your module brings out VCC (logic) and VM (output voltage) as separate pins, only VM needs to go to the battery voltage — check the silkscreen/datasheet.
5. **PIN_ENABLE has been on GPIO 6 since 2026-09-20** (previously GPIO 7) — if you're going off an older wiring note, that was the only control pin that changed.

---
*Last updated 2026-09-20 · see also `AGENTS.md` in the project root for the software context (battery compensation, known open items).*
