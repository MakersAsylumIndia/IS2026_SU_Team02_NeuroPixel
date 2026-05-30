const int NUM_SENSORS = 6;
const int emgPins[NUM_SENSORS] = {36, 39, 32, 33, 34, 35};
const char* sensorNames[NUM_SENSORS] = {"Sensor 1 (GPIO 36)", "Sensor 2 (GPIO 39)", "Sensor 3 (GPIO 32)", "Sensor 4 (GPIO 33)", "Sensor 5 (GPIO 34)", "Sensor 6 (GPIO 35)"};

const int CALIBRATE_BUTTON_PIN = 12;

int sensorValues[NUM_SENSORS] = {0};
int thresholds[NUM_SENSORS] = {500, 500, 500, 500, 500, 500};
int maxSignals[NUM_SENSORS] = {0};
int minSignals[NUM_SENSORS] = {4095};

bool isCalibrating = false;
unsigned long calibrationEndTime = 0;
const unsigned long CALIBRATION_DURATION = 5000;

unsigned long lastSerialPrintTime = 0;
const unsigned long SERIAL_PRINT_INTERVAL = 200;

void startCalibrationRoutine() {
  if (!isCalibrating) {
    isCalibrating = true;
    calibrationEndTime = millis() + CALIBRATION_DURATION;
    for(int i = 0; i < NUM_SENSORS; i++) {
      maxSignals[i] = 0;
      minSignals[i] = 4095;
    }
    Serial.println("\n==========================================");
    Serial.println("  [CALIBRATION STARTED] - FLEX MUSCLES NOW! ");
    Serial.println("==========================================");
  }
}

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < NUM_SENSORS; i++) {
    pinMode(emgPins[i], INPUT);
  }
  pinMode(CALIBRATE_BUTTON_PIN, INPUT_PULLUP);
  analogSetAttenuation(ADC_11db);
  Serial.println("\n--- ESP32 Offline 6-Channel EMG Initialized ---");
  Serial.printf("Press the button on GPIO 12 to run a %d-second calibration.\n\n", CALIBRATION_DURATION / 1000);
}

void loop() {
  if (digitalRead(CALIBRATE_BUTTON_PIN) == LOW) {
    startCalibrationRoutine();
    delay(200);
  }

  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorValues[i] = analogRead(emgPins[i]);
    if (isCalibrating) {
      if (sensorValues[i] > maxSignals[i]) maxSignals[i] = sensorValues[i];
      if (sensorValues[i] < minSignals[i]) minSignals[i] = sensorValues[i];
    }
  }

  if (isCalibrating && (millis() >= calibrationEndTime)) {
    isCalibrating = false;
    Serial.println("\n==========================================");
    Serial.println("          CALIBRATION COMPLETED           ");
    Serial.println("==========================================");
    for (int i = 0; i < NUM_SENSORS; i++) {
      if (maxSignals[i] > minSignals[i]) {
        thresholds[i] = minSignals[i] + ((maxSignals[i] - minSignals[i]) * 0.60);
      }
      Serial.printf("%s -> Min: %d | Max: %d | New Threshold: %d\n", sensorNames[i], minSignals[i], maxSignals[i], thresholds[i]);
    }
    Serial.println("==========================================\n");
  }

  if (!isCalibrating && (millis() - lastSerialPrintTime >= SERIAL_PRINT_INTERVAL)) {
    lastSerialPrintTime = millis();
    Serial.println("--- Live Offline EMG Telemetry ---");
    for (int i = 0; i < NUM_SENSORS; i++) {
      bool isTriggered = (sensorValues[i] >= thresholds[i]);
      Serial.printf("CH %d (GPIO %d) | Val: %4d | Thresh: %4d | Status: [%s]\n",
                    i + 1, emgPins[i], sensorValues[i], thresholds[i],
                    isTriggered ? "TRIGGERED" : "  REST   ");
    }
    Serial.println();
  }

  delay(10);
}
