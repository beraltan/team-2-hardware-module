# Laser grid - XIAO ESP32-C3

New Team 2 implementation of the existing `beam_broken` / `button_pressed` protocol. It is not a recovered copy of the friend's original program. Build from the repository root with `scripts/build.ps1 -Module laser`. Wi-Fi, OOCSI host and team channel all come from the one local `config/wifi.json`.

## Hardware settings

The user confirmed **Seeed XIAO ESP32-C3**, one **TinyTronics REDLASERSENSOR**, and a TinyTronics laser emitter. Follow the complete [wiring guide and printable schematic](../../docs/LASER_WIRING.md). Blocked polarity still needs measuring on the actual sensor. Edit `LaserConfig.h` if needed before uploading.

| Default | Connection / behavior |
| --- | --- |
| Beam 1 receiver signal | OUT through 10k/15k divider to D1 / GPIO3; digital, LOW assumed blocked, INPUT |
| Finish button | D4 / GPIO6; bare switch to GND, active LOW |
| Receiver ground | Common ESP GND |
| Receiver VCC and emitter power | USB VBUS / 5V; common GND |
| Receiver signal voltage | Receiver outputs 0/5V; divider required before D1. Measure the loaded levels. |

Pin labels are checked against the [official Seeed C3 pin map](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#pin-map) and the installed core's board variant. D1/D4 define the documented wiring. The sketch reads receivers and the finish button; the emitter stays continuously powered from VBUS. It does not need a GPIO power connection.

For a digital receiver that goes HIGH when blocked, set `brokenLow=false` and use the appropriate input pull mode. For a three-wire active-HIGH finish button, set `FINISH_ACTIVE_LOW=false`. Add one `BEAMS` entry per receiver, with distinct pins and beam IDs (up to four). The compile checks reject overlapping pins/IDs.

For an analog receiver/LDR voltage divider, use `ReceiverKind::Analog` and `INPUT`, then calibrate its 12-bit thresholds from received `input_raw` values. A blocked-low receiver enters at or below `brokenThreshold` and clears at or above the higher `clearThreshold`. For blocked-high, reverse the order. The illustrative 1500/1800 values must be measured on the actual sensor. Use an ADC1-capable pin such as D1 or D2 for analog input; Seeed warns about ADC2 operation on D3. All signals still need to stay within the ESP input voltage range.

## Operation

- Inputs are sampled in a separate task about every 5 ms. Digital/analog beam changes require 20 ms stable input; the finish switch uses 25 ms debounce. A held beam generates one cut; clear then break generates another. A held startup value or a held button across a round change does not create a new event.
- A stable MAC-derived `device_id` identifies this ESP. Each boot has a new random `boot_id`; each edge has a unique `boot-counter` event ID. The beam event includes its `beam_id`; the finish event omits it.
- The ESP subscribes to the team channel and follows the controller's **`laser_round_id`**, not a hardcoded round. Two fresh increasing heartbeats establish the live controller. Conflicts, fault/lost/won states and already-completed Laser stages suppress live input events.
- With only the workshop connected, events use `test-<boot>` as their round. The workshop accepts it; a live heist rejects mismatched round IDs. No ESP controller heartbeat is published.
- Reconnects never block input sampling. The event queue is bounded, edges expire after 750 ms, and round/session changes or disconnection drop old edges. Events during an outage are not replayed on reconnect. Network transport uses the same OOCSI TCP `sendraw`/subscribe framing as the existing camera.
- `online` and per-beam `state` reports appear every two seconds, including when live events are blocked. Blocked-controller diagnostics use a test round. State reports are baselines and do not deduct time. They include raw input values for alignment. The screen applies the five-second penalty and anomaly count; the ESP's finish event only completes Laser.
- Normal operation needs no Serial Monitor. Native USB serial output has zero TX wait and there is no continuous debug printing.

## Check on the actual kit

1. Set the one Wi-Fi config and the real hardware pins/polarity. Build for `esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default`. Identify the board and COM port before uploading.
2. Home -> Test your module -> Laser -> Connect test inputs -> Start alignment mode. Align mirrors using the live detected/missing indicator. The VBUS-powered emitter stays on; local test counters pause. Exit alignment mode to check a held beam counts once, clearing/breaking counts again, and each short finish press counts once. A separate active heist still applies its normal penalties.
3. Use one crew controller for a real round. Verify the ESP echoes its current `laser_round_id`, a cut takes five seconds and one anomaly, and the finish button completes only Laser. The shared clock must continue. Test with Serial Monitor closed and power cycle the ESP.

Compiled and policy/protocol checked in [BUILD_CHECKS.md](../../BUILD_CHECKS.md). No board was flashed here; optical alignment, voltage, actual pins, sensor polarity and physical button delivery remain unverified.
