# Build the hardware

[Wiring and parts](PINOUTS.md) · [Software setup](https://github.com/beraltan/team-2-software-module/blob/main/docs/DIY.md)

## Camera

You need an LD2450, XIAO ESP32-C3, common-cathode RGB LED, three 220 Ω resistors, wire, USB power and cardboard. A USB power bank is optional; the sled fits the measured 96.65 × 24.91 × 22.23 mm case.

1. Print the four [camera parts](../design/cardboard-camera/) in their supplied orientations. We used recycled PLA, three walls and 7% infill. Check the [radar Fit F](../design/fit-checks/LD2450%20Fit%20F.stl), [ESP holder](../design/fit-checks/ESP%20Holder%20Fit%20Test.stl) and optional [battery clip](../design/fit-checks/Battery%20Clip%20Fit%20Test.stl) samples first. Inspect the catches in the slicer and support them if needed.
2. With the head detached, seat the XIAO headers-up and lower the ESP lock vertically. Fit the optional assembled power bank between its retaining clips and stops.
3. Slide the radar holder sideways onto the sled's dovetail. Insert the LD2450 along its PCB channels, sensing face towards the cover. Fit the LED and [wire the components](PINOUTS.md#camera--xiao-esp32-c3), keeping wires clear of the joints.
4. Slide the front cover into its guides until the top and lower catches engage. The cover also retains the head-to-sled joint. To disassemble, release the top and lower tabs, remove the cover, then slide the head off.
5. Make your own shell around the assembly. Leave the LED, USB/charging port and release tabs accessible. Check radar tracking through the finished cover before securing the shell.
6. [Flash and configure the camera](../firmware/Concept1_Camera/README.md), then choose the same team in the browser application.

The production radar holder uses Fit F; older fit variants are omitted. The final meshes were checked for rigid interference, but printed fit and tracking through the 4 mm cover depend on the printer and filament. Use the coupons before printing full parts.

## Other modules

- **Morse:** mount the button through a cardboard panel with backing so it cannot be pushed inside. Connect it to the C6 using the [pin table](PINOUTS.md#morse--xiao-esp32-c6).
- **Skidule:** mount the 10 kΩ rotary pot, slider and confirmation button with backing. [Knob STLs](../design/potentiometer-knobs/) are included. Check full travel and button retention before closing the panel.
- **Laser:** fix the emitter and mirror mounts around the opening so the reflected beam ends at the receiver. Put the finish button inside. Align below eye level with a backstop; use the browser's Laser alignment view, then check cuts and finish presses. [C6 build status and C3 alternative](PINOUTS.md#laser--final-c6-and-archived-c3-alternative).
- **Vault:** reuse a box for the door and mount the two servo horns/linkage. Check direction and travel with the mechanism detached before attaching it. Run Test lock/unlock, then return to Automatic.

[Build and upload the other firmware](../challenge-2/README.md). In the browser, check every module in **Test your module**, then rehearse the complete camera → Skidule → Morse → laser → vault → escape sequence and reset.
