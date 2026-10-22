# Solar Garden Lights

WLAN-gesteuerte, akkubetriebene Garten-/Außenbeleuchtung (LED-Lichterkette) auf Basis eines ESP32-C6. Gedimmt wird per PWM über ein H-Brücken-Treibermodul, gesteuert aktuell über **ESPHome** (lokale Web-UI + optionale Home-Assistant-Integration) — Solarladung ist das langfristige Ziel, aktuell lädt der Akku per USB.

## Überblick

- **Board:** DFRobot FireBeetle 2 ESP32-C6, mit Onboard-LiPo-Laderegler am JST-Akkuanschluss
- **Akku:** 1S-LiPo, 5000 mAh, eigenes BMS (Überladung/Tiefentladung geschützt)
- **Last:** LED-Lichterkette (reine DC-Last, kein Motor), angesteuert über einen 4-Kanal-H-Brücken-Treiber, rein zum PWM-Dimmen genutzt
- **Steuerung:** aktuell über ESPHome (`esphome/gardenlights.yaml`) — zwei dimmbare Lichter ("Ambient Light", "Outdoor Light"), Akku-Tiefentladeschutz, Sonnenstand-/Zeit-Automatik, Fallback-Hotspot bei WLAN-Ausfall
- Es gibt zwei getrennte PWM-Kanäle (Ambient + Outdoor), nicht nur einen wie in der ursprünglichen Version

## Firmware-Historie

Die aktuelle Firmware ist **ESPHome** (`esphome/gardenlights.yaml`). Davor gab es eine PlatformIO/Arduino-Variante mit Steuerung über die Blynk-App (ein Lichtkanal statt zwei) — dieser Code wurde aus dem Repo entfernt.

## Verkabelung

Ein vollständiges Verkabelungsdiagramm (Steuersignale, Treiber-Stromversorgung, Masse) liegt unter [`docs/verkabelung.html`](docs/verkabelung.html).

> GitHub zeigt den Inhalt von `.html`-Dateien nicht interaktiv in der Dateivorschau an. Zum Ansehen die Datei herunterladen und lokal im Browser öffnen, oder GitHub Pages für dieses Repo aktivieren (Settings → Pages → Branch `main`, Ordner `/docs`) — dann ist sie unter `https://<user>.github.io/<repo>/verkabelung.html` erreichbar.

Kurzfassung der Pinbelegung (Details siehe Diagramm):

| Signal | Pin | Ziel |
|---|---|---|
| PWM Ambient | GPIO14 | Treiber IN4 → Ambient-LED-Kette |
| PWM Outdoor | GPIO15 | Treiber IN2 → Outdoor-LED-Kette |
| Treiber Enable/Standby | GPIO6 | Treiber EEP |
| Akku-Spannung (ADC) | GPIO0 | Spannungsteiler am Akku |
| WLAN-Reset-Taste | GPIO9 | BOOT-Taste, gegen GND |

Die Treiber-Stromversorgung (VCC/VM) kommt direkt von einer Abzweigung an den rohen Akku-Leitungen — **nicht** über `VIN` (nur Ladeeingang) oder `3V3` (feste Spannung, würde die Akku-Spannungskompensation im Dimmen unterlaufen).

**Noch nicht kalibriert:** Der Akku-Spannungsteiler (`*2`-Faktor in der ESPHome-Config) und die Ziel-LED-Spannung (`led_target_mv`, aktuell 3500 mV) sind noch nicht mit einem Multimeter gegengeprüft.

## Setup (ESPHome)

1. [ESPHome](https://esphome.io/) installieren (CLI oder Home-Assistant-Add-on)
2. `esphome/secrets.yaml.example` nach `esphome/secrets.yaml` kopieren und die Platzhalter ausfüllen (WLAN-Zugangsdaten, API-Verschlüsselungskey, OTA-Passwort, Hotspot-Passwort, Standort-Koordinaten für die Sonnenstand-Automatik). Diese Datei ist in `.gitignore` eingetragen und wird nie committed.
3. Flashen: `esphome run esphome/gardenlights.yaml`
4. Danach läuft das Gerät auch ohne Home Assistant über seinen eingebauten Web-Server; die native API ist optional für die HA-Integration.

## Bekannte offene Punkte

- Akku-Gesundheit war zuletzt fraglich (Einzelzelle maß isoliert 2,33 V — unter dem BMS-Schwellwert); vor weiteren Drain-Tests erst Akku-Zustand prüfen
- Spannungsteiler- und LED-Zielspannungs-Kalibrierung steht noch aus
- Treiber-Modell/Datenblatt ist nicht eindeutig bestätigt

## Build & Tools

- ESPHome-Config: `esphome run esphome/gardenlights.yaml` (bzw. über die ESPHome-Dashboard-UI)
- Serial-Monitor-Baudrate: `115200`
