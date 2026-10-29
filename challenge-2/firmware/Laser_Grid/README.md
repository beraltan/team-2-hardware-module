# Laser: archived C3 alternative

This is a replacement implementation for **XIAO ESP32-C3**. The final physical module used a C6; its sketch and pin map are not included.

[Wiring and schematic](../../../docs/PINOUTS.md#laser--final-c6-and-archived-c3-alternative) · [Build instructions](../../README.md)

Build with `scripts/build.ps1 -Module laser` from `challenge-2/`. Wi-Fi comes from `config/wifi.json`; receiver/button pins and polarity are in `LaserConfig.h`. The emitter stays powered from 5V.

Check loaded receiver voltage and blocked polarity before connecting the GPIO. In **Test your module → Laser**, align the beam, then verify one cut per interruption and one finish event per press. During a heist, events follow the controller's current laser round.
