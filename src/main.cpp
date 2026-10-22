// Reconstructed 2026-10-22 from the string/symbol tables of
// .pio/build/esp32-c6-devkitc-1/src/main.cpp.o (last successful build:
// 2026-09-20, 23:02:04 - see "LED Light Controller v2.0" / build date below).
// The original file was already empty before this session; comments, exact
// formatting, and code order are therefore not guaranteed to be 1:1 with the
// original, but all function/variable names are confirmed from the compiled
// object file. Log messages have been translated to English from the original
// (German) wording.

#include <Arduino.h>
#include <WiFiManager.h>
#include "secrets.h"
#include <BlynkSimpleEsp32.h>

#define PIN_PWM 14
#define PIN_ENABLE 6
#define PIN_BAT 0
#define PIN_RESET_BTN 9

#define PWM_FREQ 20000
#define PWM_RESOLUTION 8

#define MAX_POWER 1.0f
#define LED_TARGET_MV 3500

#define BATTERY_SAMPLES 8
#define BATTERY_EMPTY_MV 3000
#define BATTERY_FULL_MV 4200
#define BATTERY_LOW_MV 3050
#define BATTERY_RECOVER_MV 3150
#define BATTERY_SYNC_MS (5UL * 60UL * 1000UL)

BlynkTimer timer;

int currentBrightness = 0;
int lastBatteryMV = BATTERY_FULL_MV;
int lastReportedPct = -1;
unsigned long lastReportMillis = 0;
bool lowBattery = false;
unsigned long resetBtnPressedSince = 0;

int readBatteryMV() {
  float sum = 0;
  for (int i = 0; i < BATTERY_SAMPLES; i++) {
    int mv = analogReadMilliVolts(PIN_BAT);
    sum += mv;
  }
  return (int)((sum / BATTERY_SAMPLES) * 2.0f);
}

int mvToPct(int mv) {
  int pct = map(mv, BATTERY_EMPTY_MV, BATTERY_FULL_MV, 0, 100);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}

void applyLight() {
  if (lowBattery || currentBrightness == 0) {
    ledcWrite(PIN_PWM, 0);
    digitalWrite(PIN_ENABLE, LOW);
    return;
  }

  digitalWrite(PIN_ENABLE, HIGH);

  float ratio = (float)LED_TARGET_MV / (float)lastBatteryMV;
  if (ratio > 1.0f) ratio = 1.0f;

  int duty = (int)(currentBrightness * MAX_POWER * ratio);
  ledcWrite(PIN_PWM, duty);
}

void checkBattery() {
  int mv = readBatteryMV();
  lastBatteryMV = mv;
  int pct = mvToPct(mv);

  float volts = mv / 1000.0f;

  if (mv <= BATTERY_LOW_MV && !lowBattery) {
    lowBattery = true;
    currentBrightness = 0;
    Serial.printf(" Battery critical (%.2f V) - light off!\n", volts);
  } else if (mv >= BATTERY_RECOVER_MV && lowBattery) {
    lowBattery = false;
    Serial.printf(" Battery recovered (%.2f V) - light available again\n", volts);
  }

  if (lastReportedPct < 0 || abs(pct - lastReportedPct) >= 1) {
    Blynk.virtualWrite(V2, volts);
    Blynk.virtualWrite(V3, pct);
    Serial.printf(" Blynk: %.2f V (%d%%)\n", volts, pct);
    lastReportedPct = pct;
    lastReportMillis = millis();
  } else {
    Serial.printf(" Local: %.2f V (%d%%)\n", volts, pct);
  }

  applyLight();
}

void checkResetButton() {
  if (digitalRead(PIN_RESET_BTN) == LOW) {
    if (resetBtnPressedSince == 0) {
      resetBtnPressedSince = millis();
      Serial.println(" Reset button pressed (hold 3s)...");
    } else if (millis() - resetBtnPressedSince >= 3000) {
      Serial.println(" Reset: Erasing WiFi credentials, restarting...");
      WiFiManager wm;
      wm.resetSettings();
      WiFi.mode(WIFI_OFF);
      delay(500);
      ESP.restart();
    }
  } else {
    resetBtnPressedSince = 0;
  }
}

BLYNK_CONNECTED() {
  Serial.println(" Connected to Blynk!");
  Blynk.virtualWrite(V1, map(currentBrightness, 0, 255, 0, 100));
}

BLYNK_WRITE(V0) {
  int state = param.asInt();
  currentBrightness = state ? 255 : 0;
  Serial.printf(" Switch: %s\n", state ? "ON" : "OFF");
  applyLight();
}

BLYNK_WRITE(V1) {
  int pct = param.asInt();
  currentBrightness = map(pct, 0, 100, 0, 255);
  Serial.printf(" Dimmer: %d%% (%d/255)\n", pct, currentBrightness);
  applyLight();
}

BLYNK_WRITE(V4) {
  if (param.asInt()) {
    Serial.println(" Blynk: WiFi reset requested - erasing credentials, restarting...");
    WiFiManager wm;
    wm.resetSettings();
    WiFi.mode(WIFI_OFF);
    delay(500);
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);

  btStop();
  setCpuFrequencyMhz(80);

  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, LOW);
  ledcAttach(PIN_PWM, PWM_FREQ, PWM_RESOLUTION);
  pinMode(PIN_RESET_BTN, INPUT_PULLUP);

  Blynk.config(BLYNK_AUTH_TOKEN);

  WiFiManager wm;
  wm.setConnectRetries(3);
  wm.setConnectTimeout(20);
  wm.setConfigPortalTimeout(180);
  wm.setWiFiAutoReconnect(true);
  wm.setCaptivePortalEnable(true);

  bool connected = false;
  if (!wm.getWiFiIsSaved()) {
    Serial.println("  No WiFi saved!");
    Serial.println(" Setup portal opening...");
  }
  connected = wm.autoConnect(WIFI_AP_NAME);

  if (connected) {
    Serial.print(" WiFi connected: ");
    Serial.print(WiFi.SSID());
    Serial.print(" (");
    Serial.print(WiFi.localIP());
    Serial.println(")");
    Blynk.connect();
  } else {
    Serial.println(" WiFi error - running offline");
    Serial.println(" Reconnecting in background");
  }

  WiFi.setSleep(WIFI_PS_MIN_MODEM);
  Serial.println(" WiFi Power Save: MIN_MODEM");
  Serial.println("   (Fast response, ~25-30 mA WiFi)");

  Serial.println("  LED Light Controller v2.0");
  Serial.printf("  Built: %s %s\n", __DATE__, __TIME__);
  Serial.println("  Initializing...");
  Serial.println(" Bluetooth: OFF");
  Serial.println(" CPU clock: 80 MHz");

  Serial.println(" Blynk Virtual Pins:");
  Serial.println("   V0 = Switch (0/1)");
  Serial.println("   V1 = Dimmer (0-100%)");
  Serial.println("   V2 = Battery volts (e.g. 4.1V)");
  Serial.println("   V3 = Battery percent (0-100%)");
  Serial.println("   V4 = WiFi reset (button)");
  Serial.println(" Ready!");

  timer.setInterval(BATTERY_SYNC_MS, checkBattery);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  timer.run();
  checkResetButton();
}
