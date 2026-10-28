# Radar camera firmware

Build through `scripts/build.ps1 -Module camera` from the repository root. Board: Seeed Studio XIAO ESP32-C3. Toolchain: ESP32 core 3.3.11, ArduinoJson 7.3.2. USB CDC on boot. FQBN: esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default.

The firmware reads LD2450 UART at 256000 8N1 and outputs USB JSON at 5 Hz. This backup uses ONLY the root `config/wifi.json` for Wi-Fi, server and team channel. The build script generates the shared header. Configured builds connect to the configured team automatically; old saved credentials and channel are ignored. USB `wifi_config` is refused with an instruction to edit the shared file and rebuild. USB `wifi_disable` stops networking until reboot.

The original `wifi_setup.h` and `setup_page.h` are retained as historical backup source but are not included or compiled by this sketch. The Wi-Fi setup portal is not active in this shared-config build.

Current directional LED firmware compiled successfully but still needs flashing and optical testing on the assembled device. Earlier versions were tested on USB/Wi-Fi; this does not verify the current guidance patterns. The LD2450_USB_Test sketch is a minimal diagnostic alternative.

See the repository README for wiring, build instructions and the paired software module.
