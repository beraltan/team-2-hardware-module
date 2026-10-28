# Camera source located and verified

The camera firmware is in the existing public hardware repository:

- [Original Concept1_Camera sketch](https://github.com/beraltan/team-2-hardware-module/blob/9fe64f78d69a8fde7d2dc4722408f90bfac4742c/firmware/Concept1_Camera/Concept1_Camera.ino)
- [Original complete sketch folder](https://github.com/beraltan/team-2-hardware-module/tree/9fe64f78d69a8fde7d2dc4722408f90bfac4742c/firmware/Concept1_Camera)
- [Shared-config backup sketch](../firmware/Concept1_Camera/Concept1_Camera.ino)
- [Minimal USB radar diagnostic](../firmware/LD2450_USB_Test/LD2450_USB_Test.ino)

Verified against GitHub on 18 October 2026: hardware `main` was commit `9fe64f78d69a8fde7d2dc4722408f90bfac4742c`; the original camera Git blob was `11838851d5e7bcb9ffc95a72ac5f98e76e82e8a6`. This matches the source snapshot already used for the private backup.

The original board is a **XIAO ESP32-C3**, connected to an HLK-LD2450 radar, with an RGB guidance LED. This is the cardboard camera/radar module, not a video-streaming ESP32-CAM. Radar TX goes to ESP D7/GPIO20, radar RX to ESP D6/GPIO21, with 5V/GND power. RGB signal pins are GPIO2/3/4.

The backup preserves radar and LED behavior and replaces per-device saved Wi-Fi provisioning with the requested shared `config/wifi.json` build configuration. Build with `scripts/build.ps1 -Module camera`. It compiled successfully in the preceding backup check. The original portal version remains available at the pinned source links above. No camera was flashed or physically retested during this source lookup.
