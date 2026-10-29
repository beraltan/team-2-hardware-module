# Camera firmware

LD2450 + XIAO ESP32-C3 + RGB LED. [Assembly](../../docs/BUILD_GUIDE.md) · [Wiring](../../docs/PINOUTS.md#camera--xiao-esp32-c3)

## Flash from a Mac

1. Download or clone this repo. Keep the `.ino` and both `.h` files in this folder together.
2. Install [Arduino IDE](https://www.arduino.cc/en/software). Add this Boards Manager URL in Settings: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.
3. Install **esp32 by Espressif Systems 3.3.11** and **ArduinoJson 6.21.5**. Open `Concept1_Camera.ino`, select **XIAO_ESP32C3**, enable **USB CDC On Boot**, choose the USB data port and upload. Ordinary builds use the real sensor.
4. If the port does not connect, hold BOOT, press/release RESET, release BOOT and select the port again. Leave Erase All Flash disabled to retain saved Wi-Fi.
5. Join **team2-radar**, password **aqswdefr**, and open **http://192.168.4.1**. Enter a 2.4 GHz personal Wi-Fi network/hotspot, select the team and enable public OOCSI sharing. Eduroam and browser-login networks are unsupported.
6. Rejoin that network and open the [software module](https://github.com/beraltan/team-2-software-module). Select the same team and camera. Team 2 uses `OOCSI-things/team-2`.

With Arduino CLI installed, the equivalent Mac commands are:

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.11 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install 'ArduinoJson@6.21.5'
arduino-cli board list
bash firmware/Concept1_Camera/flash-mac.sh /dev/cu.usbmodemYOUR_PORT
```

Run from the repo root and replace the port with the one listed. The helper explicitly selects real-sensor mode. Compilation was checked on Windows; Mac upload still needs checking on a Mac.

## Verify the build and troubleshoot

- Walk left/right and near/far; the displayed position should follow. Repeat with the finished cover fitted.
- Send a guidance cue during an active round with valid tracking. The physical LED should light; it is deliberately off while idle.
- No camera: check venue Wi-Fi, sharing enabled and the same team/channel. Keep one live game controller.
- No position: check 5V, common ground and crossed radar TX/RX. At USB serial **115200**, `simulated` must be `false` and `radar_frame` should increase. Radar UART is **256000 8N1**.
- Tracking but no LED: check active play, a fresh controller heartbeat, common cathode and D0/D1/D2 resistors/lead order.

For bench simulation only, compile with `-DTEAM2_SIMULATION=1`; simulated samples are labelled. The Mac helper uses `-DTEAM2_SIMULATION=0`.
