# Radar camera firmware

Open Concept1_Camera.ino with both headers in this folder. Board: Seeed Studio XIAO ESP32-C3. Tested toolchain: Espressif Arduino core 3.3.11, ArduinoJson 6.21.5. Enable USB CDC on boot. CLI FQBN: esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default.

The firmware reads LD2450 UART at 256000 8N1 and outputs USB JSON at 5 Hz. Venue Wi-Fi credentials are entered through the local setup portal and stored on the device, not in source. The setup AP is named `team2-radar`, with password `aqswdefr`. Join it, then open http://192.168.4.1. Choose Team 1-20 (or Custom channel) on that page and tick Enable public OOCSI camera sharing to appear in the matching game. Sharing is opt-in and starts only after the new venue Wi-Fi connection succeeds. Failed credentials are not saved.

Production firmware was compiled, flashed and flash-verified on a XIAO ESP32-C3 on 1 October 2026. That check confirmed simulation off and Wi-Fi/OOCSI but received no radar frames. On 28 October the builder confirmed working camera tracking and RGB feedback after replacing the radar damaged by a fall on presentation day. This is user-reported functional evidence, not a new measurement of latency or position accuracy. A USB-only diagnostic is available in [the heist package](../../challenge-2/firmware/LD2450_USB_Test/).

See the [assembly guide](../../docs/BUILD_GUIDE.md) for wiring and the [paired software module](https://github.com/beraltan/team-2-software-module) for the controller. This portal firmware remains separate from the [shared-config heist snapshot](../../challenge-2/).

## Flash from a Mac

1. Download this repository using **Code > Download ZIP** and unzip it, or clone it with Git. Keep the whole `firmware/Concept1_Camera` folder together: the `.ino` and both `.h` files are required.
2. Install [Arduino IDE 2 for macOS](https://www.arduino.cc/en/software). In **Arduino IDE > Settings**, add this Additional Boards Manager URL:

   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`

3. In **Boards Manager**, install **esp32 by Espressif Systems**, version **3.3.11**. In **Library Manager**, install **ArduinoJson by Benoit Blanchon**, version **6.21.5**. WiFi, Preferences, WebServer and DNSServer are included with the ESP32 core; no OOCSI library is needed.
4. Open [`Concept1_Camera.ino`](Concept1_Camera.ino). Select **XIAO_ESP32C3** under the ESP32 boards. Enable **USB CDC On Boot**. Keep **Erase All Flash Before Sketch Upload** disabled to retain saved Wi-Fi settings.
5. Connect the XIAO using a USB data cable. Choose its port, normally `/dev/cu.usbmodem...`, and click **Upload**. Close any serial monitor or simulator that already has the port open. If uploading cannot connect, hold **BOOT**, press and release **RESET**, release **BOOT**, then select the port again and retry. See [Seeed's board guide](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/).
6. Join **team2-radar**, password **aqswdefr**, from a phone or computer. Open **http://192.168.4.1**, enter the venue's **2.4 GHz** Wi-Fi details, select your **Team 1-20** or **Custom channel**, and tick **Enable public OOCSI camera sharing**. Connect and save. Credentials are stored on the board, never in this repository. Enterprise Wi-Fi such as eduroam and browser-login networks are not supported by this setup form.
7. Rejoin your normal Wi-Fi. Open [Bank Heist](https://github.com/beraltan/team-2-software-module), choose the same team and select `team2_camera_...`. Keep only one game controller tab connected. Open Serial Monitor at **115200** if checking diagnostics: `simulated` must be `false`, and `radar_frame` should increase when the LD2450 is connected.

Normal Arduino IDE uploads use **real hardware mode**. No source edits or Wi-Fi passwords in code are necessary. The LED is off while idle; during play it displays guidance only with valid tracking and a fresh game heartbeat.

### Optional Arduino CLI on macOS

With [Arduino CLI](https://arduino.github.io/arduino-cli/latest/installation/) installed, run from the repository root:

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.11 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install 'ArduinoJson@6.21.5'
arduino-cli board list
bash firmware/Concept1_Camera/flash-mac.sh /dev/cu.usbmodemYOUR_PORT
```

Replace the example port with the one listed for your board. This helper explicitly builds real sensor mode. Mac instructions are based on the official tools; compilation/flashing was tested on Windows, not on a Mac.

## Verify the build and troubleshoot

1. **Check tracking.** Open the paired controller, choose the camera's team/channel and select the discovered camera. Walk to known positions and confirm that the displayed dot follows left/right and near/far movement. Repeat with the finished front cover and cardboard shell in place. This is a functional check, not a measurement of position accuracy.
2. **Check the physical LED.** Start an active round with valid tracking, then send a guidance cue and watch the actual LED. The LED is deliberately off while idle or without fresh control/tracking; darkness in those states is not itself a fault. A reported LED command describes the firmware's commanded output and does not prove that the LED is physically illuminated.
3. **If the camera is not discovered,** check its 2.4 GHz venue Wi-Fi connection, public OOCSI sharing and the selected team. Camera and browser must use exactly the same channel; Team 2 uses `OOCSI-things/team-2`. Keep one game controller tab connected and active.
4. **If the camera is discovered but no position appears,** power down and inspect the radar's 5V supply, common ground and crossed UART connections: radar **TX → XIAO D7/GPIO20 (RX)** and radar **RX → XIAO D6/GPIO21 (TX)**. Reconnect power and inspect USB diagnostics at **115200 baud**: `simulated` must be `false`, and `radar_frame` should increase. The radar UART itself runs at **256000 baud**. If frames remain absent, use the [USB-only radar diagnostic](../../challenge-2/firmware/LD2450_USB_Test/).
5. **If tracking works but a guidance cue does not light the LED,** confirm that the round is active, tracking is valid and the browser is still sending a fresh heartbeat. Power down before checking the common cathode to GND and the three colour leads, each through its own resistor, to D0/D1/D2. Identify the purchased LED's lead order rather than assuming it from appearance.

## Simulator without attached components

Compile with `--build-property compiler.cpp.extra_flags=-DTEAM2_SIMULATION=1` to emulate a camera without the LD2450 or RGB LED. Ordinary builds default to real sensor mode. The simulator advertises as `team2_camera_SIMULATED_<boot ID>` and marks every sample `simulated: true`. It starts at X=-1100, Y=1800 mm and responds to game guidance at 350 mm/s while the round is running. Left increases X, right decreases X, forward decreases Y, and back increases Y, matching the HTML's player-facing-camera convention. Stop or a stale controller heartbeat stops movement. Reset to idle returns to the starting position. This tests communications and game logic, not physical sensing or LED operation.

Configure venue Wi-Fi using the existing USB `wifi_config` command or setup portal. Credentials are stored only on the ESP. After Wi-Fi has been saved, USB command `{"command":"oocsi_config","channel":"OOCSI-things/team-2","enabled":true}` enables public OOCSI without supplying the password again. Only enable it with the device owner's consent. The simulator uses the same channel and discovery fields as the production module.

USB telemetry is queued as complete lines only when sufficient buffer space is available, with a nonblocking transmit timeout, so an unread USB connection does not stall camera updates.

Production builds explicitly use `-DTEAM2_SIMULATION=0`. OOCSI includes `led` and `led_report_kind: commanded`; this reports the output commanded by firmware, not optical confirmation that a physical LED is wired. The existing hosted HTML needs a separate display update to show that field.
