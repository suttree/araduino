# Ara prototype PCB

The editable board and project are in `../schematic/ara.kicad_pcb` and `../schematic/ara.kicad_pro`. The quotation package is in `../manufacturing/rev-a/`.

The board is 65 × 45 mm with two copper layers and 1.6 mm thickness. All 45 fitted components are on the front. Four 3.2 mm mounting holes are centred 3.5 mm from the board edges. Copper keepouts around the holes have a 6.4 mm diameter. The USB-C receptacle faces the left edge. The horizontal speaker connector faces towards the lower edge inside the board outline. Leave room to insert and remove both cables when designing the enclosure.

The processor remains an ATmega328P at 16 MHz and 5 V. The pin-9 PWM signal feeds a two-stage RC filter, fixed quiet-volume attenuation and a PAM8302A amplifier. The speaker is separate and plugs into a JST-PH socket. Neither speaker lead is ground. USB-C also carries CH340C serial programming data. The six-pin ISP header provides initial programming and recovery.

## Files and checks

- `parts.json` assigns an exact JLCPCB part number to every fitted component.
- `sourcing.json` retains public component specifications, stock and USD price breaks checked on 25 November 2026. It also records rejected low-stock candidates. The data is not a stock reservation or assembly quote.
- `footprints/` freezes the footprint subset used for this revision. See `footprints/LICENSE.md` for the KiCad library licence. The USB connector's printed outline was moved to the fabrication layer to avoid clipping at the board edge. Its copper and drill geometry are unchanged. `AraBoard.pretty/SW_SKRPACE010.kicad_mod` is an original footprint derived from Alps Alpine's land and circuit drawings.
- `generate.py` creates component placement and exports a Specctra routing input. It writes `ara-unrouted.*`, never the finished board.
- `ara-routed.ses` retains the routing result from FreeRouting 2.6.0. The routing was checked again in KiCad, including schematic parity.
- `factory-placement.json` records absolute supplier rotations and offsets for the connector and IC origins. Numbered pad maps from the [JLCEDA](https://lceda.cn/) / [EasyEDA Official Library](https://easyeda.com/) were compared with the actual board pads. The retained residuals describe differences between land patterns, not physical measurement or a manufacturing tolerance approval.
- `placement.py` applies those corrections to an uncorrected KiCad CPL. The original and factory CPL files are both retained. Copper and drill geometry are unchanged by this conversion.
- `finish.py` imports routing, uses a native KiCad XML netlist for exact net names, adds screw-head keepouts and ground fills, and writes the finished board beside the schematic.
- `inspect.py` checks for vias in fitted solder pads, verifies short via-free clock routes and checks fitted pads against screw-head areas. It records geometry, not measured electrical performance.
- `package.py` checks the finished board before exporting the manufacturing files. It fails if native checks report findings or the BOM and CPL reference sets differ.

The native checks passed in KiCad 10.0.8. The circuit has not been powered, measured or listened to on this PCB. USB operation remains unverified. The corrected factory placement preview was inspected on 25 November 2026; final factory DFM placement approval is still required.

## Reproduce the exports

Use KiCad's Python for `generate.py` and `finish.py`. Use ordinary Python 3 for the other scripts. Keep the schematic, selected parts and routed board together when editing.

```sh
python3 hardware/schematic/generate.py
python3 hardware/schematic/check.py
kicad-cli sch export netlist --format kicadxml -o /tmp/ara-native.xml hardware/schematic/ara.kicad_sch
# Only regenerate placement if deliberately rebuilding the layout:
python3 hardware/pcb/generate.py
# Import the retained routing only when placement has not changed:
python3 hardware/pcb/finish.py hardware/pcb/ara-routed.ses --netlist /tmp/ara-native.xml
arduino-cli compile --fqbn arduino:avr:uno --libraries sketches/libraries --build-path /tmp/ara-production-build sketches/ara_birdsong
python3 hardware/pcb/package.py --firmware-build /tmp/ara-production-build
```

If placement changes, reroute its new DSN before calling `finish.py`. Do not import an older session into a moved component layout. Inspect the resulting copper and assembly drawings before ordering.

## Drawing references

- [Alps switch dimensions, land pattern and contact diagram](https://tech.alpsalpine.com/e/products/detail/SKRPACE010/).
- [Murata resonator comparison](https://www.murata.com/~/media/webrenewal/products/timingdevice/ceralock/nem/comparison/dr-e17-0030.ashx?la=en).
- [Murata ferrite bead specifications](https://www.murata.com/products/productdata/8796749987870/QNFA9114.pdf).
- [KiCad footprint library](https://github.com/KiCad/kicad-footprints).
