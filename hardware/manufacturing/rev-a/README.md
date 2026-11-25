# Ara revision A prototype quotation package

The design is a 65 × 45 × 1.6 mm, two-layer board with 45 fitted components on its front. It uses the current Uno-compatible birdsong firmware, USB-C power and serial programming, an analogue speaker amplifier, a JST-PH speaker socket and a six-pin recovery programming header.

KiCad 10.0.8 reports zero ERC, DRC, unconnected-item and schematic-parity findings. These results establish the checked design's connectivity and configured manufacturing clearances. The corrected factory placement preview was inspected on 25 November 2026. Physical operation, sound and USB behaviour remain unverified; final factory DFM placement approval is still required. This is a prototype package.

## Upload for a quotation

Upload these three files to JLCPCB:

1. `gerbers.zip` contains the copper, masks, silkscreen, paste, outline and separate plated/non-plated drill files.
2. `bom.csv` contains 27 exact JLCPCB part numbers and all 45 fitted references.
3. `cpl.csv` contains those same 45 references, coordinates in millimetres and top-side rotations. It includes the supplier-specific origin and rotation corrections in `../../pcb/factory-placement.json`. `cpl-kicad.csv` retains the uncorrected reference export and must not be uploaded instead. Test pads and mounting holes are excluded from assembly.

The Gerbers, drill files and CPL share an origin at the board's lower-left corner. Board coordinates are positive rightwards and upwards from that origin. The component-specific CPL offsets account for the supplier's different footprint origins; they do not move PCB pads. The native KiCad board is in `../../schematic/ara.kicad_pcb`, with its schematic and project beside it.

Quote five and ten fully assembled boards with these options:

- FR-4, two layers, 1.6 mm, 1 oz copper, green mask and white silkscreen.
- Lead-free HASL as the initial finish option. Ask the factory about any fine-pitch finish restriction it identifies.
- Standard 0.3 mm through vias. The board uses 0.65 mm via pads and at least 0.15 mm configured net clearance.
- All components on the top. Include through-hole soldering for J2 and J3 and the USB receptacle's mounting tabs as required by its process.
- Initial ISP programming of the bootloader, fuses and birdsong firmware. See `PROGRAMMING.md`.
- A basic powered startup-song check with a 4-ohm speaker if the factory can provide it at a reasonable additional cost. Agree the method and price separately.
- Shipping to the UK and any applicable tax/import charges.

Before accepting a quote, check that every BOM row is matched, every reference is fitted and no processor substitution has been proposed. Inspect the factory's placement preview for each IC's pin one, the reset diode's cathode, the USB-C connector and the speaker socket. The initial preview exposed origin and rotation differences. Corrections were calculated from numbered supplier CAD pads and the actual KiCad board pads, then applied to the CPL. Differences in supplier land-pattern dimensions remain a factory DFM check. Do not treat a successful file upload as placement approval.

The speaker, enclosure, USB cable and power adapter are separate from the PCB assembly order. The planned speaker is The Pi Hut SKU 102826 with a factory-fitted JST-PH cable. Its sound and cable polarity have not been tested with this board.

## Other files

- `checks/erc.json` and `checks/drc.json` retain native KiCad reports.
- `checks/drill.txt` records the drill sizes and counts.
- `board-top.svg`, `board-bottom.svg` and `assembly.svg` are native KiCad drawings.
- `board-outline.step` models the bare board and holes. It omits component heights and cable access.
- `schematic/ara.svg` is KiCad's native schematic export.
- `firmware/` contains the normal USB-update HEX and the combined bootloader/application HEX for initial ISP programming.
- `manifest.json` records source hashes, toolchain and export hashes. The firmware timing is a snapshot and can be changed by USB upload after initial programming.
- `TESTING.md` describes checks the owner can perform after delivery.

Five prototype boards were ordered and paid on 25 November 2026 for $94.04 including standard UK shipping and tax after a $10 assembly coupon. JLCPCB batch `W2026112603479484` is marked Paid; PCB and assembly file checks are marked Reviewing. This does not establish production or shipment. Initial programming will be performed after delivery. See `../../pcb/quotes.json` for the confirmation.
