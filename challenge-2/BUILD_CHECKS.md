# Backup checks - 18 October 2026

## Public handover checks - 28 October 2026

The firmware sources were copied unchanged from the 27 October presentation handoff. Fresh C++17 syntax/assertion checks passed for the laser policy and both vault policy/parser paths. Five laser protocol tests passed against the paired software repository's rebuilt HTML; that software passed 108 JavaScript tests, including the final report's camera → Skidule → Morse → laser → vault/escape order. No fresh complete board compilation, upload or physical hardware test was performed for this publication.

The browser HTML is maintained in the separate software repository rather than duplicated here. Run `node scripts/test-laser.mjs <path-to-bank-heist.html>` to select it explicitly. The recorded compilations and deployment hashes below describe the earlier recovery snapshot, before the current browser progression change.

## Update - 19 October 2026

The confirmed one-beam TinyTronics wiring now has a two-page rendered/reviewed
PDF, SVG, PNG and step-by-step guide. Receiver VCC uses USB VBUS/5V; OUT uses
the user's 10k/15k divider. D1/GPIO3 is `INPUT`; finish remains D4/GPIO6 to GND.
Blocked polarity and loaded signal levels still require measurement.

The updated laser firmware compiled with configured synthetic Wi-Fi:
**972653 flash bytes (74%), 36996 global RAM bytes (11%)**. No board was
flashed. All 15 C++ policy assertions pass. Receiver state reports are now
available even when live edge delivery is blocked; those diagnostics use a
test round and cannot alter a live heist.

The source project's **106 JavaScript tests pass**, including three new
alignment checks for fresh/missing/stale beam states, local counter pause,
retry suppression on exit and selected-device isolation. Browser testing of
the built page showed detected and missing states, zero outgoing application
messages, paused counters, and normal counting after exit. This was simulated
input, not an optical test. Five private-backup bundle checks also pass.

The new page is published as Data Foundry dataset 27070 **record 36**. Its
SHA-256 matches the tested backup file byte-for-byte:
`B09F48CD2BC72AA987A910A9D243D4651E1345DC56FCAFF023AA82D31CC802DF`.
The previous record 35 page was backed up locally and its previous SHA-256
verified before replacement. An anonymous browser verified the live Laser
alignment controls. No physical receiver state was available during that check.

The emitter stays on through its VBUS power wiring. Alignment is a local test
view, not a remote power switch; a separate live controller still applies
normal penalties. Firmware, voltage, optical alignment and USB operation need
checking on the actual kit.

## Original backup verification

All seven ESP targets compiled through the shared-config build script with ESP32 core 3.3.11 and ArduinoJson 7.3.2 (OOCSI 1.6.0 for the library-based targets). The optional Uno compiled separately with AVR core 1.8.6. No board was uploaded or motor actuated.

| Target | Flash bytes | Global RAM bytes | Result |
| --- | ---: | ---: | --- |
| Camera / XIAO C3 | 979517 | 36972 | Passed |
| Skidule / XIAO C6 | 998102 | 43112 | Passed |
| Morse / XIAO C6 | 989374 | 42848 | Passed |
| Laser / XIAO C3 | 972683 | 36996 | Passed with synthetic configured Wi-Fi |
| Direct vault / S3 | 904178 | 47316 | Passed |
| Uno bridge / S3 | 904031 | 46924 | Passed |
| Radar USB diagnostic / XIAO C3 | 285908 | 13472 | Passed |
| Servo fallback / Uno R3 | 4572 | 439 | Passed |

Except for Laser, these sizes are for compile-check builds with placeholder Wi-Fi, which disables network startup. Actual configured builds can differ in size. Laser also compiled with disabled placeholder Wi-Fi (410650 flash bytes / 21080 global RAM bytes); its full networking build used a separate temporary copy and synthetic credentials, leaving the real local config untouched. The direct vault compile also includes its 13 heist-policy and six remote-command assertions; the bridge includes its 13 policy assertions.

The new laser firmware includes **15 compile-time assertions** against its actual round-lease, debounce, analog hysteresis and queued-event code. They also pass separately with `avr-g++ -std=c++17 -fsyntax-only scripts/check-laser.cpp`. They cover fresh controller confirmation, stale/replayed heartbeats, conflicts/owner replacement, bounce/held inputs, old-round queue rejection, expired/disconnected events, finished/faulted states, millis wrap and both analog polarities.

`node --test scripts/test-laser.mjs` passes **three integration checks** against the actual backed-up hacker-page bundle: five-second deduction and duplicate/held suppression; finish completion without penalty or timer reset, with stale-round rejection; and workshop-only test rounds that do not affect the live heist. No browser or OOCSI connection is used by those checks. The board is confirmed XIAO C3; receiver/button pins, polarity, optical alignment and real network/input delivery remain physical checks.

Additional checks:

- Normal builds reject placeholder credentials before invoking Arduino CLI.
- All seven ESP sources import the one generated `Team2Config.h`. The six network sketches have no per-sketch SSID/password literals.
- Camera startup reads SSID/password/channel from shared config; active NVS and provisioning-portal overrides are absent. Historical portal headers are retained but not included by the sketch.
- Both PowerShell scripts pass syntax parsing. The full build iterates all ESP targets; `uno` is an explicit separate target.
- A normal Morse build also passed using synthetic configured Wi-Fi values containing quotes, a backslash and an accented character: 989748 flash bytes / 42864 global RAM bytes. The generated C++ fields decode back to the exact input UTF-8 strings. No real credentials were used; the local config was then restored to its template.
- Git ignores the actual `config/wifi.json`, generated header and all build outputs. Only the example config is included in the backup.

The included hacker page is the previously verified Data Foundry record 35 snapshot, with four-bar Skidule compatibility, the vault panel and the concurrent hardware test bench. SHA-256 before line-ending normalization: `D9F2EE498EF182ED3E4C81C3777A3B9C7A0D9B441AC7171E24B7E9D14F9913FC`. Its 98 JavaScript checks passed in the source project; no web behavior was changed for this backup.

The original friend's laser source and exact ten-position Skidule source are not available. Laser_Grid is a newly written replacement; this backup does not claim to reproduce those currently flashed binaries. The original camera location and unchanged hardware repository revision were verified again through GitHub: see [CAMERA_LOCATION.md](docs/CAMERA_LOCATION.md). Physical Wi-Fi/OOCSI delivery, real input calibration, camera tracking, servo endpoints and door power remain checks on the actual kit.
