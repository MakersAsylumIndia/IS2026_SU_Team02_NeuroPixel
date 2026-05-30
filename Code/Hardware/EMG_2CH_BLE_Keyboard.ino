#include <BleKeyboard.h>

BleKeyboard bleKeyboard("EMG Controller", "Jatin", 100);

const int EMG_PIN_UP   = 34;
const int EMG_PIN_DOWN = 35;

void setup() {
  Serial.begin(115200);
  bleKeyboard.begin();
  Serial.println("EMG BLE Keyboard initializing...");
}

void loop() {
  if (bleKeyboard.isConnected()) {
    int emgUp   = analogRead(EMG_PIN_UP);
    int emgDown = analogRead(EMG_PIN_DOWN);

    Serial.print("UP: "); Serial.print(emgUp);
    Serial.print("  |  DOWN: "); Serial.println(emgDown);

    if (emgUp >= 1800) {
      bleKeyboard.press(KEY_UP_ARROW);
      delay(100);
      bleKeyboard.release(KEY_UP_ARROW);
      delay(200);
    }

    if (emgDown >= 1800) {
      bleKeyboard.press(KEY_DOWN_ARROW);
      delay(100);
      bleKeyboard.release(KEY_DOWN_ARROW);
      delay(200);
    }
  }
}
