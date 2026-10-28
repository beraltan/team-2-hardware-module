# One-beam laser module wiring

Confirmed on 19 October 2026: **Seeed XIAO ESP32-C3**, one TinyTronics
**REDLASERSENSOR / SKU 001061**, and a TinyTronics red laser emitter.
The emitter SKU is not confirmed, so both common 5 V variants are covered.

[Printable schematic](../output/pdf/Team2_Laser_Wiring.pdf) |
[Wiring diagram](schematics/laser-wiring.svg) |
[300 dpi image gallery](schematics/README.md) |
[Schematic PNG](schematics/laser-wiring.png) |
[Pin guide PNG](schematics/laser-pin-guide.png)

## Connections

Power the XIAO from a USB-C laptop, charger or power bank. Its **VBUS / 5V**
header supplies the receiver and the 5 V laser module. No separate bench supply
is required for this one-beam circuit. VBUS is a power pin; the GPIOs use
3.3 V logic. Leave the XIAO's **3V3 pin unconnected** in this circuit.

| Part / terminal | Connect to |
| --- | --- |
| Receiver `VCC` | XIAO `VBUS / 5V` |
| Receiver `GND` | XIAO `GND` / common ground rail |
| Receiver `OUT` | One end of R1 = **10 kOhm** |
| Other end of R1 | Junction J1: XIAO **D1 / GPIO3** AND top of R2 |
| Other end of R2 = **15 kOhm** | Common GND |
| Bare normally-open finish switch, first contact | XIAO **D4 / GPIO6** |
| Finish switch, other contact | Common GND |
| 5 V laser emitter positive terminal | XIAO `VBUS / 5V` |
| Laser emitter negative terminal | Common GND |
| C1 = 100 nF ceramic, recommended | Receiver VCC to GND, close to receiver |

Follow the receiver's actual **VCC / OUT / GND** printing. The product photo
shows those three labels; the receiver is not wired like the laser emitter.
If its optical sensor arrives loose, fit it in the orientation shown by the
seller's product photo. Do not infer pin order from wire colour or PCB rotation.

```text
Receiver OUT ---- R1 10k ----+---- D1 / GPIO3
                            |
                         R2 15k
                            |
                           GND

D4 / GPIO6 ------ normally-open finish switch ------ GND
```

Both resistor ends at J1 and the D1 jumper must share the **same breadboard
row**. Resistors have no polarity. Use 1% parts if available (ordinary 1/4 W
parts are sufficient). A 4-leg tactile button has two internally joined pairs:
connect D4 to one pair and GND to the other pair; verify with continuity testing.

## Which laser emitter?

- **LSRDRD, two wires:** positive/red to VBUS, negative/black to GND, after
  confirming the product's polarity. This is a complete 5 V module.
- **LSRMOD, three header pins, SKU 001339:** the seller identifies the pin
  marked **S as positive** and **- as negative**; the **middle pin is unused**.
  Therefore **S -> VBUS**, **- -> GND**, **middle -> leave disconnected**.
  Do not assume its middle pin is VCC, and do not connect S to an ESP GPIO.
- For another SKU, follow its own rated voltage/pinout. A bare laser diode
  needs a suitable driver and is not interchangeable with these modules.

The two listed emitter modules draw roughly 25 mA according to TinyTronics.
This design powers the emitter continuously; the firmware does not switch it.
Attach the XIAO's supplied Wi-Fi antenna. USB power-bank auto-shutoff behaviour
still needs checking with the actual bank.

## Voltage and polarity check before play

1. Wire with USB unplugged. Check VBUS and GND are not shorted. Align the
   beam below eye level into the receiver with a non-reflective backstop;
   keep people out of the reflected path when adjusting mirrors.
2. Initially leave the D1 jumper disconnected. Power the board by USB.
   Check receiver VCC-to-GND is about 5 V.
3. Measure J1-to-GND with the beam received and then blocked. An ideal
   0/5 V OUT becomes **0/3.0 V** because `VJ1 = VOUT * 15 / (10 + 15)`.
   The *loaded* readings must give a LOW below about **0.8 V** and a HIGH
   above about **2.5 V**, while staying at or below **3.3 V**. Use the actual
   measured levels, not the unloaded OUT voltage.
4. Some receiver revisions have a weak/resistive output, so the 10k/15k
   divider can pull the HIGH down. If HIGH is below 2.5 V, try **100k/150k**
   with the same topology and measure again, keeping the divider near the
   ESP. If it still fails the levels, use a suitable 5 V-to-3.3 V buffer;
   do not bypass the divider. Higher values reduce load but pick up more noise.
5. Unplug USB, connect D1 to J1, then power up with a clear aligned beam.
   The seller does not specify blocked polarity. Firmware currently assumes
   **blocked LOW**. In `LaserConfig.h`, use `brokenLow=false` if blocking
   instead makes the input HIGH. Keep the pin mode **INPUT**, not INPUT_PULLUP.

The finish button uses the ESP's internal pull-up: released = HIGH,
pressed = LOW. No external button pull-up is needed. The receiver divider
is different: it must use `INPUT` so the internal pull-up does not change it.

## Alignment mode and demo behaviour

Home -> **Test your module** -> **Laser** -> **Connect test inputs** ->
**Start alignment mode**. The laser stays continuously powered from VBUS.
The page displays the receiver's live detected/missing state and raw reading
while you align mirrors. This is a local receiver check; it does not arm the
heist or send a second controller heartbeat. There is no remote laser-power
switch in this always-on wiring.

Use an isolated workshop session for alignment. A separate live heist
controller can still count real beam breaks. End the live round before
alignment if penalties would be disruptive. Alignment events are excluded
from this page's local cut/finish counters, and its original counters resume
after leaving alignment mode. A 2-second state report is normal.

After alignment, stop alignment mode, test a beam break and a short finish
press, then start one crew controller. A break sends `beam_broken`, deducting
five seconds and one anomaly. The finish press sends `button_pressed` and
completes Laser while the shared heist clock continues. Held beams and
repeated event IDs must not repeat penalties. Nothing here has been flashed
or electrically tested on your board.

## Build and upload

Fill the existing ignored **config/wifi.json** once. From the repo root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Module laser
arduino-cli board list
# Replace COMx with the confirmed XIAO C3 port; this command flashes the board.
arduino-cli upload --fqbn esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default --port COMx --input-dir build/laser firmware/Laser_Grid
```

Keep the entire sketch folder and generated config together. The shared build
script supplies Wi-Fi/OOCSI settings; do not hardcode them into the sketch.

## Verified references

Checked 19 October 2026:

- [TinyTronics REDLASERSENSOR: 5 V supply, 0/5 V output and sensor assembly photo](https://www.tinytronics.nl/nl/sensoren/optisch/licht-en-kleur/rode-laser-sensor-module).
- [Seeed XIAO C3 pin map: VBUS, D1=GPIO3, D4=GPIO6](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#pin-map).
- [Espressif ESP32-C3 datasheet, DC characteristics, section 5.4](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf).
- [TinyTronics LSRMOD: 5 V, S positive, - negative, middle unused](https://www.tinytronics.nl/nl/verlichting/lasers/rode-laser-module-5v-650nm-1).
- [TinyTronics LSRDRD: 5 V two-wire emitter](https://www.tinytronics.nl/nl/verlichting/lasers/rode-laser-5v-650nm-draad).
