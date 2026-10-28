# Heist message contract

The maintained [software interface](https://github.com/beraltan/team-2-software-module/blob/main/docs/INTERFACE.md) documents every camera, Skidule, Morse, laser and vault packet. [Mission rules](https://github.com/beraltan/team-2-software-module/blob/main/docs/HEIST.md) describe the final sequence.

Use **OOCSI-things/team-2** everywhere, with one live crew controller. The browser is the game authority. Firmware uses oocsi.id.tue.nl TCP port 4444; the browser uses its WebSocket endpoint.

Laser echoes the current `laser_round_id`; beam breaks carry unique event IDs and the physical finish emits `button_pressed`. In the final software, these affect the heist only during the running laser stage. The hardware-test bench accepts independent test rounds.

Vault automatic release requires all four puzzle flags and a fresh controller in the escape phase. Telemetry describes commanded angles, not measured position. Explicit test commands yield to an active live heist.

For the original configurable camera's USB/Wi-Fi portal, read [its README](../../firmware/Concept1_Camera/README.md). The shared-config camera here instead reads `config/wifi.json` at build time.
