# Heist electronics

Complete saved firmware from the 27 October handoff. The paired [software module](https://github.com/beraltan/team-2-software-module) runs camera → Skidule → Morse → laser → vault release → escape.

**Final build clarification, 29 October:** the builder confirmed a **XIAO ESP32-C6 for the laser** and an **ESP32-S3 driving the vault servos directly**. The S3 board and both servos were reused from a previous project. The saved `Laser_Grid` program and illustrated laser wiring below are a **C3 replacement implementation**, not the recovered C6 program. The latest `team-2-esp-backup` repository was checked and contains the same C3 archive. The final C6 sketch and its actual receiver/button pin map still need to be recovered; do not describe the C3 pin numbers or build target as the final C6 wiring.

| Module | Source | Build target / board |
| --- | --- | --- |
| Camera, shared Wi-Fi config | [Concept1_Camera](firmware/Concept1_Camera/) | `camera` / XIAO ESP32-C3 |
| Skidule knob, slider and button | [Skidule_Controls](firmware/Skidule_Controls/) | `skidule` / XIAO ESP32-C6 |
| Morse button | [Morse_Button](firmware/Morse_Button/) | `morse` / XIAO ESP32-C6 |
| Laser receiver and finish button, archived replacement | [Laser_Grid](firmware/Laser_Grid/) | `laser` / XIAO ESP32-C3; final C6 program not recovered |
| Two-servo vault, direct | [Vault_ESP32S3_Direct](firmware/Vault_ESP32S3_Direct/) | `vault-direct` / ESP32-S3 |
| Vault alternative: ESP bridge | [Vault_ESP32S3_Bridge](firmware/Vault_ESP32S3_Bridge/) | `vault-bridge` / ESP32-S3 |
| Vault alternative: Uno servos | [Vault_Arduino_Uno](firmware/Vault_Arduino_Uno/) | `uno` / Uno R3 |
| USB radar diagnostic | [LD2450_USB_Test](firmware/LD2450_USB_Test/) | `radar-test` / XIAO ESP32-C3 |

Use either the direct vault or bridge + Uno arrangement. Check the actual board markings before uploading: C3, C6 and S3 firmware are different. The S3 target assumes N16R8 (16 MB flash / OPI PSRAM); change [targets.json](targets.json) if your board differs. Vault signal defaults are GPIO4/GPIO5, with 90° locked and 0° unlocked. Calibrate endpoints to the mechanism. SG90/MG90S labels in the historical notes do not change those commanded PWM defaults.

## Configure and build

Install [Arduino CLI](https://arduino.github.io/arduino-cli/). Open PowerShell in this folder:

```powershell
.\scripts\setup.ps1
Copy-Item config/wifi.example.json config/wifi.json
# Enter venue Wi-Fi and team channel in config/wifi.json locally.
.\scripts\build.ps1 -Module laser
```

Choose a target from the table. Dependencies are pinned to ESP32 core **3.3.11**, OOCSI **1.6.0**, ArduinoJson **7.3.2**; optional Uno uses AVR core **1.8.6**. Every build regenerates the shared `Team2Config.h` library from `config/wifi.json`. Keep that local file, generated headers and build outputs private. The script compiles only.

Identify your board and port with `arduino-cli board list`, then upload only the target you built, for example:

```powershell
arduino-cli upload --port COMx --fqbn esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default --input-dir build/laser firmware/Laser_Grid
```

Replace COMx with the actual port. Other targets use their exact FQBN from `targets.json`. Keep each complete sketch folder and all its headers. Do not use `-AllowPlaceholderWifi` for a working upload: it builds a non-connecting diagnostic.

The shared-config camera in this folder has **no active Wi-Fi portal** and connects to the configured OOCSI channel at startup. For the independently configurable camera with SSID **team2-radar**, password **aqswdefr**, and [Mac upload instructions](../firmware/Concept1_Camera/README.md#flash-from-a-mac), use [the original camera firmware](../firmware/Concept1_Camera/) instead. Its saved Wi-Fi settings do not configure the shared-config sketches.

## Wiring and checks

[Laser wiring](docs/LASER_WIRING.md) · [Printable laser schematic](output/pdf/Team2_Laser_Wiring.pdf) · [Direct vault](docs/VAULT_DIRECT.md) · [Uno alternative](docs/VAULT_BRIDGE.md) · [Software input contracts](https://github.com/beraltan/team-2-software-module/blob/main/docs/INTERFACE.md)

Laser defaults: one 5 V receiver, OUT through a 10 kΩ / 15 kΩ divider into D1/GPIO3, finish switch D4/GPIO6 to GND. The emitter stays powered from VBUS. Confirm the sensor's loaded signal voltage and blocked polarity before connecting the GPIO. See the illustrated guide for exact terminals.

For Skidule, use slider A0, knob A1 and button D4/GPIO22 to GND. Calibrate raw endpoints in its sketch. Morse defaults to a three-wire active-HIGH button: signal D3/GPIO21, power 3V3 and common GND; for a bare switch to ground, set `BUTTON_ACTIVE_LOW = true`.

Run the browser's **Test your module** bench before a full heist. Laser alignment observes the receiver; it does not switch emitter power. Vault telemetry reports commanded angles, with no position sensor. End explicit vault tests in Automatic.

After transport, check the retention and backing of every physical control. Compare the infiltrator's printed numerical Morse reference with the passcode in the actual deployed software. Finish preparation with a complete two-player rehearsal, including penalties, vault release and reset, rather than relying only on separate module checks. Skidule was reconstructed by Team 2 from Group 15's Miro concept after an email request for their implementation went unanswered; Group 7 supplied the Morse/Lampo source and documentation.

[Recorded build checks](BUILD_CHECKS.md) describe the earlier compilations, not a fresh upload. The exact teammate-calibrated ten-position Skidule sketch was not supplied; this is the compatible four-bar replacement. Laser is a replacement implementation, not a readback of the friend's board. Both vault arrangements remain as archived references; the builder has since confirmed the direct ESP32-S3 vault as the final setup.

To run the five offline laser/bundle checks, clone the software repo alongside this one and run `node scripts/test-laser.mjs`, or supply its HTML path as the first argument. [Attribution](THIRD_PARTY.md).
