// Seeed Studio XIAO ESP32-C3 + HLK-LD2450. No extra libraries.
// Arduino board: XIAO_ESP32C3. USB CDC On Boot: Enabled.
// LD2450 5V -> XIAO 5V, GND -> GND, TX -> D7/GPIO20, RX -> D6/GPIO21.
// Power from the PC's USB port. The radar needs 5V power, not 3V3.
#include <Arduino.h>
#include <string.h>

constexpr int RADAR_RX = 20;
constexpr int RADAR_TX = 21;
constexpr uint32_t RADAR_BAUD = 256000;
uint8_t windowBytes[30] = {};
uint8_t latest[30] = {};
size_t used = 0;
uint32_t frames = 0, bytesReceived = 0, lastFrameMs = 0, lastPrintMs = 0;

uint16_t readLE(const uint8_t *p) {
  return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

int decodeSigned(const uint8_t *p) {
  const uint16_t raw = readLE(p);
  const int magnitude = raw & 0x7FFF;
  // LD2450 uses bit 15 = POSITIVE, not ordinary two's-complement int16.
  return (raw & 0x8000) ? magnitude : -magnitude;
}

void setup() {
  Serial.begin(115200);
  Serial1.setRxBufferSize(1024);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, RADAR_RX, RADAR_TX);
  delay(1500);
  Serial.println("LD2450 position test: X/Y in mm; speed in cm/s.");
  Serial.println("Waiting for radar at 256000 baud on D7/GPIO20...");
}

void loop() {
  while (Serial1.available()) {
    const uint8_t b = uint8_t(Serial1.read());
    ++bytesReceived;
    if (used == sizeof(windowBytes)) {
      memmove(windowBytes, windowBytes + 1, sizeof(windowBytes) - 1);
      --used;
    }
    windowBytes[used++] = b;
    // Sliding window resynchronizes after noise, partial packets or dropped bytes.
    if (used == 30 && windowBytes[0] == 0xAA && windowBytes[1] == 0xFF &&
        windowBytes[2] == 0x03 && windowBytes[3] == 0x00 &&
        windowBytes[28] == 0x55 && windowBytes[29] == 0xCC) {
      memcpy(latest, windowBytes, sizeof(latest));
      ++frames;
      lastFrameMs = millis();
      used = 0;
    }
  }

  const uint32_t now = millis();
  if (now - lastPrintMs < 500) return;
  lastPrintMs = now;
  if (!frames || now - lastFrameMs > 2000) {
    Serial.printf("NO FRESH RADAR DATA | bytes=%lu frames=%lu | check 5V, GND, TX->D7 and baud\n",
                  (unsigned long)bytesReceived, (unsigned long)frames);
    return;
  }

  Serial.printf("t=%lu ms | frame=%lu | ", (unsigned long)now, (unsigned long)frames);
  int count = 0;
  for (int i = 0; i < 3; ++i) {
    const uint8_t *p = latest + 4 + i * 8;
    bool present = false;
    for (int j = 0; j < 8; ++j) present |= p[j] != 0;
    if (!present) continue;
    ++count;
    Serial.printf("T%d: x=%d y=%d speed=%d resolution=%u | ",
                  i + 1, decodeSigned(p), decodeSigned(p + 2),
                  decodeSigned(p + 4), unsigned(readLE(p + 6)));
  }
  if (!count) Serial.print("Connected; no targets");
  Serial.println();
}
