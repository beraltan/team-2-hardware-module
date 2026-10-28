# Hardware/software interface

The browser runs the game. Devices exchange messages on **OOCSI-things/team-2** at **oocsi.id.tue.nl** (TCP 4444 for the firmware, WebSocket `/ws` for the browser). Choose the same team/channel everywhere and use one live crew controller. The [full contract](https://github.com/beraltan/team-2-software-module/blob/main/docs/INTERFACE.md) covers all puzzles and the vault.

## Camera

LD2450 UART: **256000 baud, 8N1**. USB: **115200 baud**, newline-delimited JSON at 5 Hz. Position is Cartesian millimetres from the radar origin; +Y points forward. Walk a known point to verify X polarity.

Camera → browser includes `radar_seq`, `radar_boot`, `radar_valid`, `radar_x_mm`, `radar_y_mm` and `guidance_supported`. The original portal firmware also sends `led` and `led_report_kind: commanded`; that describes commanded electrical output, not optical confirmation. The browser can still display “Not reported over OOCSI” because the saved controller does not render that field. USB diagnostics can include all target slots. Reject stale, invalid or ambiguous tracking for game decisions.

Browser → camera heartbeats include `toktik_session`, `toktik_beat`, `toktik_state`, `toktik_guide` and `toktik_camera`. Browser guidance has a one-second hold deadline. Firmware stops applying guidance when the controller heartbeat is two seconds old; it also requires valid, fresh single-target tracking. These are separate deadlines.

| Direction | LED cue |
| --- | --- |
| Left | Blue, one pulse |
| Right | Blue, two pulses |
| Forward | Green, three pulses |
| Back | Green, four pulses |
| Stop | Steady red during valid, controlled play |
| Fault / idle | Blinking amber / off |

## Complete heist

The current software follows **camera → Skidule → Morse → laser → vault release → escape**. A ten-minute shared clock and anomaly count span the run. `heist_navigation_complete` marks entry completion; `toktik_state: won` means the completed return, rather than camera arrival. The automatic vault requires all four completion flags and the escape phase, with a fresh controller lease.

The [heist firmware package](../challenge-2/) includes the exact saved board targets, Wi-Fi build configuration, laser circuit and two vault arrangements. Original camera provisioning remains available in [firmware/Concept1_Camera](../firmware/Concept1_Camera/). The shared-config camera is a separate build and does not run that portal.
