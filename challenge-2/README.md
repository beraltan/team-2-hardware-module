# Heist firmware

The camera's [Wi-Fi portal firmware](../firmware/Concept1_Camera/) is maintained separately. This folder contains the other devices.

| Build name | Sketch | Board |
| --- | --- | --- |
| `morse` | [Morse_Button](firmware/Morse_Button/) | XIAO ESP32-C6 |
| `skidule` | [Skidule_Controls](firmware/Skidule_Controls/) | XIAO ESP32-C6 |
| `vault-direct` | [Vault_ESP32S3_Direct](firmware/Vault_ESP32S3_Direct/) | ESP32-S3 |
| `laser` | [Laser_Grid](firmware/Laser_Grid/) | C3 alternative; final C6 source missing |

[Assembly](../docs/BUILD_GUIDE.md#other-modules) · [Pinouts](../docs/PINOUTS.md)

## Build and upload

Install Arduino CLI, then run PowerShell from this folder:

```powershell
.\scripts\setup.ps1
Copy-Item config/wifi.example.json config/wifi.json
# Enter Wi-Fi and the team channel in config/wifi.json.
.\scripts\build.ps1 -Module morse
arduino-cli board list
arduino-cli upload --port COMx --fqbn esp32:esp32:XIAO_ESP32C6:CDCOnBoot=cdc --input-dir build/morse firmware/Morse_Button
```

Replace COMx and use the matching build name, sketch and FQBN from [targets.json](targets.json). Keep complete sketch folders and headers. Dependencies: ESP32 core **3.3.11**, OOCSI **1.6.0**, ArduinoJson **7.3.2**. For the S3, confirm flash/PSRAM markings against the target before building.

The build script generates `Team2Config.h` from the local Wi-Fi file. Keep `config/wifi.json` and `build/` private. It compiles only; upload explicitly after selecting the correct board.

Open **Test your module** in [Bank Heist](https://github.com/beraltan/team-2-software-module), check each input and vault movement, then return the vault to Automatic. Rehearse the full sequence and reset before play.
