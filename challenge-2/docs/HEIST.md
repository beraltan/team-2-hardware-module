# Mission sequence

**Camera maze → Skidule → Morse → laser → vault release → escape.** The browser owns one ten-minute clock and the shared anomaly count. Follow the maintained [heist rules](https://github.com/beraltan/team-2-software-module/blob/main/docs/HEIST.md) and [message contract](INTERFACE.md).

Camera entry completion advances to Skidule. Skidule provides access to the physical numerical Morse clue; verification then advances to Laser. The final software accepts beam cuts and finish presses only while running in the laser phase. The automatic vault still requires all four completion flags, a current round and a fresh controller in escape. Returning to the entrance ends the clock.

The saved firmware sources retain the handoff’s board targets and pin mappings. The older handoff browser put Laser before Skidule; use the current software release for the corrected sequence. Clone only this hardware package to compile firmware, and use the separate software repo for the standalone browser.
