# Solar Garden Lights

WiFi-controlled, battery-powered garden/outdoor lighting (LED string) built on an ESP32-C6. Dimmed via PWM through an H-bridge driver module, currently controlled by **ESPHome** (local web UI + optional Home Assistant integration) — solar charging is the long-term goal, the battery currently charges over USB.

## Overview

- **Board:** DFRobot FireBeetle 2 ESP32-C6, with an onboard LiPo charge circuit on the JST battery connector
- **Battery:** 1S LiPo, 5000 mAh, own BMS (overcharge/overdischarge protected)
- **Load:** LED string (pure DC load, not a motor), driven through a 4-channel H-bridge driver, used purely for PWM dimming
- **Control:** currently via ESPHome (`esphome/gardenlights.yaml`) — one dimmable light ("Outdoor Light"), battery low-voltage cutoff, sunset/time automation, fallback hotspot on WiFi failure

## Firmware History

The current firmware is **ESPHome** (`esphome/gardenlights.yaml`). Before that there was a PlatformIO/Arduino variant controlled through the Blynk app (`src/main.cpp`) — kept in the repo for reference, but no longer the actively maintained version. Both drive a single light channel. Note: `main.cpp` was reconstructed from the compiled object file of the last successful build (string/symbol table match verified), since the original source was lost before it ended up in version control — see the comment at the top of the file for details.

## Wiring

A full wiring diagram (control signals, driver power supply, ground) is at [`docs/wiring.md`](docs/wiring.md).

Pinout summary (see the diagram for details):

| Signal | Pin | Target |
|---|---|---|
| PWM | GPIO14 | Driver IN4 → LED string |
| Driver enable/standby | GPIO6 | Driver EEP |
| Battery voltage (ADC) | GPIO0 | Voltage divider on the battery |
| WiFi reset button | GPIO9 | BOOT button, to GND |

The driver's power supply (VCC/VM) comes directly from a tap on the raw battery leads — **not** via `VIN` (charging input only) or `3V3` (fixed voltage, would defeat the battery voltage compensation used for dimming).

**Not yet calibrated:** the battery voltage divider (`*2` factor in the ESPHome config) and the target LED voltage (`led_target_mv`, currently 3500 mV) haven't been cross-checked with a multimeter yet.

## Setup (ESPHome, current)

1. Install [ESPHome](https://esphome.io/) (CLI or the Home Assistant add-on)
2. Copy `esphome/secrets.yaml.example` to `esphome/secrets.yaml` and fill in the placeholders (WiFi credentials, API encryption key, OTA password, hotspot password, location coordinates for the sunset automation). This file is listed in `.gitignore` and never gets committed.
3. Flash it: `esphome run esphome/gardenlights.yaml`
4. The device then also runs without Home Assistant via its built-in web server; the native API is only needed for the HA integration.

## Setup (PlatformIO/Blynk, legacy)

1. Copy `include/secrets.h.example` to `include/secrets.h` and fill in your own Blynk template name/ID and auth token (from [blynk.cloud](https://blynk.cloud)). This file is git-ignored and never gets committed.
2. Build/flash with [PlatformIO](https://platformio.org/): `pio run -t upload`
3. Blynk virtual pins: `V0` switch, `V1` dimmer (0–100%), `V2` battery volts, `V3` battery percent, `V4` button to reset saved WiFi credentials.

## Known Open Issues

- Battery health was questionable as of the last check (a single cell measured 2.33V in isolation — below the BMS threshold); confirm battery condition before running further drain tests
- Voltage-divider and target-LED-voltage calibration is still outstanding
- The exact driver model/datasheet hasn't been confirmed

## Build & Tools

- ESPHome config: `esphome run esphome/gardenlights.yaml` (or via the ESPHome dashboard UI)
- Legacy PlatformIO project: `pio run` (build) / `pio run -t upload` (flash)
- Serial monitor baud rate: `115200`
