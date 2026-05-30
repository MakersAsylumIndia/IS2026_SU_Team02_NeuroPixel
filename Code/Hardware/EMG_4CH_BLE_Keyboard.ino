#include <BleKeyboard.h>

BleKeyboard bleKeyboard("NeuroPixel EMG", "NeuroPixel", 100);

#define EMG1_PIN  34    // LEFT
#define EMG2_PIN  35    // RIGHT
#define EMG3_PIN  32    // UP
#define EMG4_PIN  33    // DOWN

#define THRESHOLD_1  1800
#define THRESHOLD_2  1800
#define THRESHOLD_3  1800
#define THRESHOLD_4  1800

#define ALPHA      0.15f
#define SERIAL_MS  200

const uint8_t EMG_PINS[4]    = { EMG1_PIN, EMG2_PIN, EMG3_PIN, EMG4_PIN };
const int     THRESHOLDS[4]  = { THRESHOLD_1, THRESHOLD_2, THRESHOLD_3, THRESHOLD_4 };
const uint8_t KEY_CODES[4]   = { KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_DOWN_ARROW };
const char*   KEY_NAMES[4]   = { "LEFT", "RIGHT", "UP", "DOWN" };

float         filtered[4]    = { 0, 0, 0, 0 };
bool          active[4]      = { false, false, false, false };
bool          prevActive[4]  = { false, false, false, false };

unsigned long lastSerial = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  for (int i = 0; i < 4; i++) {
    filtered[i] = (float)analogRead(EMG_PINS[i]);
  }

  bleKeyboard.begin();
  Serial.println("[BLE] NeuroPixel EMG Keyboard starting...");
  Serial.println("[BLE] Pair with 'NeuroPixel EMG' on your device");
  Serial.println();
}

void loop() {
  unsigned long now = millis();

  for (int i = 0; i < 4; i++) {
    int raw     = analogRead(EMG_PINS[i]);
    filtered[i] = ALPHA * (float)raw + (1.0f - ALPHA) * filtered[i];
    active[i]   = ((int)filtered[i] >= THRESHOLDS[i]);
  }

  if (bleKeyboard.isConnected()) {
    for (int i = 0; i < 4; i++) {
      if (active[i] && !prevActive[i])  bleKeyboard.press(KEY_CODES[i]);
      if (!active[i] && prevActive[i])  bleKeyboard.release(KEY_CODES[i]);
      prevActive[i] = active[i];
    }
  }

  if (now - lastSerial >= SERIAL_MS) {
    lastSerial = now;
    if (!bleKeyboard.isConnected()) {
      Serial.println("[BLE] Waiting for connection...");
    } else {
      Serial.print("[BLE] Connected | ");
      for (int i = 0; i < 4; i++) {
        Serial.print(KEY_NAMES[i]); Serial.print(": ");
        Serial.print((int)filtered[i]);
        Serial.print(active[i] ? " [ON]  " : " [off] ");
      }
      Serial.println();
    }
  }

  delay(10);
}
