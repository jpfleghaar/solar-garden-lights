# Solar Garden Lights

WiFi-controlled, battery-powered garden/outdoor lighting (LED string) built on an ESP32-C6. Dimmed via PWM through an H-bridge driver module — solar charging is the long-term goal, the battery currently charges over USB.

This repo contains **two independent firmware options** for the same hardware. They're alternatives, not add-ons — you flash one or the other, not both at once.

## Two Ways to Run This

| | ESPHome (current) | PlatformIO / Blynk (legacy) |
|---|---|---|
| Control | Home Assistant and/or the built-in local web UI | Blynk app on your phone |
| Needs a cloud account? | No (Home Assistant integration is optional) | Yes — a free Blynk account |
| Firmware source | `esphome/gardenlights.yaml` | `src/main.cpp` |
| Maintained? | Yes, actively | No, kept for reference |

Both drive the same single light channel (GPIO14 → H-bridge → LED string) and read the same battery voltage divider (GPIO0) — see [Wiring](#wiring) below. Full setup steps for each are further down.

## Overview

- **Board:** DFRobot FireBeetle 2 ESP32-C6, with an onboard LiPo charge circuit on the JST battery connector
- **Battery:** 1S LiPo, 5000 mAh, own BMS (overcharge/overdischarge protected)
- **Load:** LED string (pure DC load, not a motor), driven through a 4-channel H-bridge driver, used purely for PWM dimming

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

**Not yet calibrated:** the battery voltage divider (`*2` factor) and the target LED voltage (`LED_TARGET_MV`/`led_target_mv`, currently 3500 mV) haven't been cross-checked with a multimeter yet.

## Setup: ESPHome + Home Assistant (current)

### Prerequisites

- [ESPHome](https://esphome.io/) installed (`pip install esphome`, or use the Home Assistant ESPHome add-on/dashboard)
- The ESP32-C6 connected via USB for the first flash (OTA works after that)

### Steps

1. Copy `esphome/secrets.yaml.example` to `esphome/secrets.yaml` and fill in:
   - `wifi_ssid` / `wifi_password` — your home WiFi
   - `api_encryption_key` — a random 32-byte base64 key, e.g. `python3 -c "import base64,os; print(base64.b64encode(os.urandom(32)).decode())"`
   - `ota_password` / `ap_password` — pick your own passwords
   - `location_latitude` / `location_longitude` — your location, used for the sunset/sunrise automation (look it up on [latlong.net](https://www.latlong.net/))
2. Flash it over USB: `esphome run esphome/gardenlights.yaml`, then pick the USB port when prompted.
3. On first boot without saved WiFi, the device opens a fallback hotspot called "Outdooresp Fallback Hotspot" (password = your `ap_password`). Connect to it and use the captive portal page to hand it your real WiFi credentials.
4. Once it's on your WiFi, reach it either:
   - directly in a browser at `http://<device-ip>` (built-in web server), or
   - through Home Assistant, which auto-discovers it via the native API if you run HA.
5. You'll see one light entity ("Outdoor Light") plus "Battery Voltage" and "Battery Percentage" sensors.
6. To re-flash later or watch logs, run `esphome run esphome/gardenlights.yaml` again — this now happens over WiFi/OTA, no USB cable needed.

## Setup: PlatformIO + Blynk (legacy)

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI, or the VS Code extension)
- A free [Blynk](https://blynk.cloud) account and the Blynk app on your phone
- The ESP32-C6 connected via USB

### Steps

1. In the Blynk console, create a new template/device and note down its **Template Name**, **Template ID**, and the device's **Auth Token**.
2. Copy `include/secrets.h.example` to `include/secrets.h` and fill in those three values, plus a name for the WiFi setup portal (`WIFI_AP_NAME`).
3. Build and flash over USB: `pio run -t upload`
4. Open the serial monitor to watch it boot: `pio device monitor` (115200 baud).
5. On first boot without saved WiFi, it opens a captive portal named after `WIFI_AP_NAME` (e.g. "GardenLights-Setup") — connect to it with your phone/laptop and enter your home WiFi credentials through the portal page.
6. Once connected, it logs into the Blynk cloud using your auth token.
7. In the Blynk app, add widgets bound to these virtual pins:
   - `V0` — switch (on/off)
   - `V1` — slider, **set Min 0 / Max 100** (the firmware reads/sends this as a percentage, not raw 0–255)
   - `V2` — labeled value, battery volts
   - `V3` — labeled value, battery percent
   - `V4` — button, remotely resets the saved WiFi credentials
8. To reset WiFi credentials locally instead of via `V4`: hold the BOOT button for 3 seconds.

`main.cpp` was reconstructed from the compiled object file of the last successful build (function/string/symbol table match verified against it), since the original source was lost before it ended up in version control — see the comment at the top of the file for details.

## Known Open Issues

- Battery health was questionable as of the last check (a single cell measured 2.33V in isolation — below the BMS threshold); confirm battery condition before running further drain tests
- Voltage-divider and target-LED-voltage calibration is still outstanding
- The exact driver model/datasheet hasn't been confirmed

## Serial Monitor

Baud rate for both firmware variants: `115200`
