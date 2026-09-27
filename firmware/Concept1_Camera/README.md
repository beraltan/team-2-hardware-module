# Radar camera firmware

Open Concept1_Camera.ino with both headers in this folder. Board: Seeed Studio XIAO ESP32-C3. Tested toolchain: Espressif Arduino core 3.3.11, ArduinoJson 6.21.5. Enable USB CDC on boot. CLI FQBN: esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default.

The firmware reads LD2450 UART at 256000 8N1 and outputs USB JSON at 5 Hz. Wi-Fi credentials are entered through the local setup portal and stored on the device, not in source. The protected setup AP is named Team2-Camera-<device suffix>; obtain its generated password through the software facilitator's authenticated setup-info action over USB. Join the AP, then open http://192.168.4.1. OOCSI remains a separate opt-in setting.

Current directional LED firmware compiled successfully but still needs flashing and optical testing on the assembled device. Earlier versions were tested on USB/Wi-Fi; this does not verify the current guidance patterns. The LD2450_USB_Test sketch is a minimal diagnostic alternative.

See the repository README for wiring, build instructions and the paired software module.
