# Team 2 — Hardware module: radar camera

Position-sensing camera prop for the Bank Heist connected experience. An LD2450 radar and XIAO ESP32-C3 send player positions; an RGB LED receives navigator guidance. The visible enclosure is cardboard, with printed internal mounts and a front head.

**Software module:** https://github.com/beraltan/team-2-software-module

The battery and power-bank assembly are optional: you can power the module through the XIAO's USB-C port instead. Skip the battery fit test and installation if using external USB power.

The outer shell is entirely up to your group. Be creative with its shape, materials, colours and decoration; the cardboard camera in the video is one example. Keep the radar's sensing area, LED, cables and power connection clear when designing your shell.

## Contents

- [Cardboard camera STL files](../design/cardboard-camera): Electronics Sled, Radar Holder, ESP Lock and Front Cover.
- Print the [ESP holder fit test](../design/fit-checks/ESP%20Holder%20Fit%20Test.stl) with the [ESP lock](../design/cardboard-camera/ESP%20Lock.stl), and the [battery clip fit test](../design/fit-checks/Battery%20Clip%20Fit%20Test.stl) before the complete sled.
- [Camera firmware](../firmware/Concept1_Camera/README.md): UART sensing, USB JSON, Wi-Fi setup and optional OOCSI.
- firmware/LD2450_USB_Test: minimal radar wiring test.
- [Shared interface](INTERFACE.md).

## Parts and wiring

| Part | Quantity / connection |
| --- | --- |
| HLK-LD2450 | 1; 5V and GND; TX to XIAO D7, RX to D6 |
| Seeed XIAO ESP32-C3 | 1; headers upward in the printed well |
| Common-cathode 5 mm RGB LED | 1; R→D0, G→D1, B→D2, cathode→GND |
| LED resistors | One per colour; select for the purchased LED forward voltage/current at 3.3V |
| Optional USB power-bank case and battery | 96.65 × 24.91 × 22.23 mm, with purchased compatible cell; omit for external USB power |
| Printed joints | Screw-free head-to-sled dovetail and sliding front cover with release detent |
| Shell materials | Your group's choice; cardboard is used in the example |

Power down before wiring; verify the actual LED lead order. Do not connect two independent 5V sources together. Use the assembled power-bank electronics for battery charging; do not connect the bare cell to the XIAO 5V pin.

## Assemble and run

1. Print the fit-check STLs, inspect sliced layers and check the purchased parts.
2. Import the four production STLs into Bambu Studio in millimetres, keeping their supplied print orientations. Use the sled underside, holder backplate, end stop flat face and cover front face on the bed. PLA is suitable for initial indoor fit checks. Slice for your printer and filament.
3. Slide the XIAO into its well with the head detached and headers upward, then lower the ESP lock vertically into its slot. If using a battery, seat the power bank between its retaining clips and end stops. Slide the head onto the transverse dovetail, then insert the radar from the open end of its PCB channels toward the fixed stop. Fit the LED and route insulated leads clear of joints.
4. Slide the front cover onto its two guides until the roof detent seats. The rear notch captures the head-to-sled dovetail. Lift the roof release tab gently before withdrawing the cover. Fit your group's shell; preserve USB access and, if fitted, charging-port access.
5. Install the Arduino dependencies and upload the camera sketch with USB CDC enabled. Close serial monitors before starting the software module.
6. Start the paired software module with the correct serial port. Verify coordinates, LED behaviour and tracking through the finished cover before a game.

## CAD and verification status

Design downloads are STL files only. Use all four production parts together. The head slides sideways onto the sled dovetail; the front cover captures the joint and prevents sideways release. Remove the cover before sliding the head off.

The current revision has four connected, watertight print meshes. Sampled CAD travel checks found no rigid interference for the ESP lock, head dovetail or cover guides; the flexible cover detent is evaluated separately. The print orientation audit found no sampled faces steeper than 45 degrees above the bed. These checks do not establish physical strength, spring fatigue, RF transmission or printed fit. Test the small coupons first, particularly the ESP lock's final seating travel. The 4 mm radar cover needs testing with the actual filament. The group assembled an earlier physical prototype; the revised fits are not yet physically validated.

Team 2: Anouk Kramer, Alp Altaner, Berk Eraltan Sönmezgil, Sebastiaan Schenk.
