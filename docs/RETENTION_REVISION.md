# Retention revision — 28 September 2026 (revision 2)

This revision addresses a cover that seated fully but did not stay latched, loose sled/head engagement and loose ESP retention.

## Changes

- Top cover hook engagement increased from 0.35 mm to 0.9 mm. A square retaining shoulder replaces the withdrawal ramp; the entry ramp remains.
- Two bottom catches added, each with 0.8 mm engagement. All three tabs have a 1.4 mm flex section and rounded slot roots. Release all three before removing the cover.
- Dovetail clearance reduced from 0.30 mm to 0.15 mm per face. The projecting tongue's side and front clearances in the cover notch are also 0.15 mm. Clearance through the wall remains open, avoiding a lip that would block assembly.
- LED supports replaced by a continuous U-shaped cradle, with a shared base, side walls and a clear lead trough. The LED flange has 0.3 mm axial clearance to the cradle.
- ESP retaining fingers now hold front and USB-end PCB corners beside the header rows. Vertical retention clearance is 0.08 mm; the front fingers span 1.05 mm and the rear fingers 1.7 mm along the PCB. Header, connector, chip and solder-tail envelopes remain clear. The separate ESP lock is unchanged.

## Further fit adjustments (revision 2)

ESP side clearance is now 0.04 mm per side; front and rear stops leave 0.08 mm at each end. Battery clips are now 1.5 mm thick and 8 mm wide (formerly 1.2 mm and 6 mm), with the battery opening unchanged. Cover catch withdrawal play is now 0.15 mm, with 0.20 mm side clearance per side and 0.15 mm pocket-floor clearance. Print the updated ESP, battery and latch samples first.

## Reprint

Use the updated **Electronics Sled**, **Radar Holder** and **Front Cover** together. The existing **ESP Lock** can be reused. Old covers and holders do not have matching latch seats.

Print these small samples first, in their supplied orientation and the intended production filament/profile:

- [Top latch — cover](../design/fit-checks/Top%20Latch%20-%20Cover.stl) and [holder](../design/fit-checks/Top%20Latch%20-%20Holder.stl).
- [Bottom latch — cover](../design/fit-checks/Bottom%20Latch%20-%20Cover.stl) and [holder](../design/fit-checks/Bottom%20Latch%20-%20Holder.stl).
- [Dovetail — sled](../design/fit-checks/Dovetail%20-%20Sled.stl) and [holder](../design/fit-checks/Dovetail%20-%20Holder.stl).
- [ESP holder](../design/fit-checks/ESP%20Holder%20Fit%20Test.stl) with the existing ESP lock.

The samples are cropped from the actual production meshes. Hold the small holder samples while engaging the latch samples; they omit the full guide rails and therefore test local engagement rather than alignment of the whole cover. Check a positive catch, smooth manual release and repeated operation. Check the ESP with its actual headers and USB cable installed.

Square retaining shoulders introduce small overhangs. Inspect the slice around latch teeth, latch pockets and PCB retaining fingers; add localized support where necessary and remove it without damaging the shoulders. Do not fill the flexible slots with hard-to-remove support. The tighter dovetail needs a physical fit check before a complete print.

## Verification

Onshape: four connected production solids, zero component-envelope collisions, zero seated printed-part intersections, zero sampled rigid-guide/dovetail/ESP-lock travel interference. Cover withdrawal is blocked at 0.6 mm, and the fitted cover prevents sideways withdrawal in both directions. Flexible catch contact during insertion is intentional; no force or fatigue simulation was performed.

Independent Blender checks: all four exported production meshes and all eight new/revised samples are single connected watertight meshes. Sled/holder, sled/cover and holder/cover have zero assembled intersection volume; pulling the unreleased cover 0.6 mm produces approximately 9 mm³ intersection with the holder. [Mesh validation](../design/fit-checks/retention-validation.json).

The print-orientation audit flags ten overhang faces. This revision is geometrically checked but still requires a physical print-and-fit test.
