#include <WiFi.h>
#include <WebSocketsServer.h>

#define EMG1_PIN  34    // LEFT
#define EMG2_PIN  35    // RIGHT
#define EMG3_PIN  32    // UP
#define EMG4_PIN  33    // DOWN

#define THRESHOLD_1  1800
#define THRESHOLD_2  1800
#define THRESHOLD_3  1800
#define THRESHOLD_4  1800

#define ALPHA  0.15f

#define AP_SSID  "NeuroPixel"
#define AP_PASS  "neuropixel123"

#define WS_SEND_MS      20
#define SERIAL_MS      100
#define HEADER_EVERY   30

WebSocketsServer webSocket(81);

const uint8_t  EMG_PINS[4]    = { EMG1_PIN,  EMG2_PIN,  EMG3_PIN,  EMG4_PIN  };
const int      THRESHOLDS[4]  = { THRESHOLD_1, THRESHOLD_2, THRESHOLD_3, THRESHOLD_4 };
const char*    KEY_NAMES[4]   = { "LEFT", "RIGHT", "UP", "DOWN" };
const char*    COL_HEADS[4]   = { "EMG1(L)", "EMG2(R)", "EMG3(U)", "EMG4(D)" };

float   filtered[4]   = { 0, 0, 0, 0 };
bool    active[4]     = { false, false, false, false };

unsigned long lastWs     = 0;
unsigned long lastSerial = 0;
int           lineCount  = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  for (int i = 0; i < 4; i++) {
    filtered[i] = (float)analogRead(EMG_PINS[i]);
  }

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("[WiFi] AP started  SSID: "); Serial.println(AP_SSID);
  Serial.print("[WiFi] IP: "); Serial.println(WiFi.softAPIP());

  webSocket.begin();
  webSocket.onEvent(wsEvent);
  Serial.println("[WS] WebSocket server on port 81");
  Serial.println();
  printHeader();
}

void loop() {
  webSocket.loop();
  unsigned long now = millis();

  for (int i = 0; i < 4; i++) {
    int raw    = analogRead(EMG_PINS[i]);
    filtered[i] = ALPHA * (float)raw + (1.0f - ALPHA) * filtered[i];
    active[i]   = ((int)filtered[i] >= THRESHOLDS[i]);
  }

  if (now - lastWs >= WS_SEND_MS) {
    broadcastState();
    lastWs = now;
  }

  if (now - lastSerial >= SERIAL_MS) {
    printColumns();
    lastSerial = now;
  }
}

void printHeader() {
  Serial.println("EMG1(L)    EMG2(R)    EMG3(U)    EMG4(D)");
  Serial.println("--------   --------   --------   --------");
  lineCount = 0;
}

void printColumns() {
  if (lineCount >= HEADER_EVERY) {
    Serial.println();
    printHeader();
  }
  for (int i = 0; i < 4; i++) {
    int val = (int)filtered[i];
    char buf[12];
    if (active[i]) snprintf(buf, sizeof(buf), "[%4d]     ", val);
    else           snprintf(buf, sizeof(buf), " %4d      ", val);
    Serial.print(buf);
  }
  Serial.println();
  lineCount++;
}

void broadcastState() {
  String msg = "{\"keys\":[";
  bool first = true;
  for (int i = 0; i < 4; i++) {
    if (active[i]) {
      if (!first) msg += ',';
      msg += '"'; msg += KEY_NAMES[i]; msg += '"';
      first = false;
    }
  }
  msg += "],\"vals\":[";
  for (int i = 0; i < 4; i++) {
    msg += (int)filtered[i];
    if (i < 3) msg += ',';
  }
  msg += "]}";
  webSocket.broadcastTXT(msg);
}

void wsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.print("[WS] Client #"); Serial.print(num);
      Serial.print(" connected from ");
      Serial.println(webSocket.remoteIP(num).toString());
      broadcastState();
      break;
    case WStype_DISCONNECTED:
      Serial.print("[WS] Client #"); Serial.print(num);
      Serial.println(" disconnected");
      break;
    default: break;
  }
}
