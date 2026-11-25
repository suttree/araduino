# Schematic sources

Updated on 25 November 2026. Exact stock, package revisions and order terms must be checked again before ordering.

| Circuit element | Reference and use |
| --- | --- |
| ATmega328P | [Microchip product page](https://www.microchip.com/en-us/product/ATMEGA328P?tab=documents), including the current NRND status. Keep for compatibility in this small prototype run, subject to sourcing review. |
| AVR TQFP pin map | [KiCad official AVR symbol library](https://raw.githubusercontent.com/KiCad/kicad-symbols/master/MCU_Microchip_ATmega.lib), ATmega328P-A alias of ATmega48PV-10A. The drawn symbols here are original, with all pins exposed individually. |
| AVR build configuration | Arduino AVR 1.8.6 `boards.txt`, Uno entries: ATmega328P, 16 MHz, Optiboot and fuse settings. The current sketch and bundled Mozzi AVR configuration provide the audio pin and default PWM rate. |
| AVR hardware | [Microchip AN2519](https://www.microchip.com/en-us/application-notes/an2519), hardware design considerations. |
| Audio amplifier | [Diodes PAM8302A datasheet](https://www.diodes.com/datasheet/download/PAM8302A.pdf), DS41333 Rev. 6-2. Pinout, SO-8 order code PAM8302AADCR, fixed gain, input coupling, decoupling, shutdown timing and ferrite output filter. |
| USB serial | [WCH CH340 datasheet download](https://www.wch-ic.com/downloads/CH340DS1_PDF.html); [manufacturer datasheet hosted by SparkFun](https://cdn.sparkfun.com/assets/5/0/a/8/5/CH340DS1.PDF), version 3B. CH340C pinout, internal oscillator and 100nF V3 decoupling at 5 V. |
| Power ramp | [TI TPS22918 datasheet](https://www.ti.com/lit/ds/symlink/tps22918.pdf), Rev. C. SOT-23 pin assignments, ON connection, CT slew control and QOD connection. |
| USB ESD | [ST USBLC6-2 datasheet](https://www.st.com/resource/en/datasheet/usblc6-2.pdf), Rev. 7. Pin mapping and layout requirements. |
| Clock | [Murata CSTNE16M0V530000R0 datasheet](https://www.murata.com/en-us/api/pdfdownloadapi?partno=CSTNE16M0V530000R0). A three-terminal 16 MHz resonator with built-in capacitors. |
| USB footprint | [KiCad HRO TYPE-C-31-M-12 footprint](https://raw.githubusercontent.com/KiCad/kicad-footprints/master/Connector_USB.pretty/USB_C_Receptacle_HRO_TYPE-C-31-M-12.kicad_mod). All 16 contact identifiers and SH shell pads were compared with the schematic symbol. Confirm the current supplier drawing before routing. |
| Resonator footprint | [KiCad Murata CSTxExxV footprint](https://raw.githubusercontent.com/KiCad/kicad-footprints/master/Crystal.pretty/Resonator_SMD_Murata_CSTxExxV-3Pin_3.0x1.1mm.kicad_mod). Three pads 1/2/3, with the centre connected to ground. |
| Speaker socket | [JST PH datasheet](https://www.jst-mfg.com/product/pdf/eng/ePH.pdf); [KiCad S2B-PH-K footprint](https://raw.githubusercontent.com/KiCad/kicad-footprints/master/Connector_JST.pretty/JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal.kicad_mod). Through-hole assembly must be included in the factory quote. |
| Speaker candidate | [The Pi Hut SKU 102826](https://thepihut.com/products/mono-enclosed-speaker-3w-4-ohm), 4 ohms, advertised 3 W, 70 × 30 × 17 mm and factory-fitted JST-PH cable. Sound and cable polarity have not been tested. |
| Sourcing leads | [ATMEGA328P-AU, C14877](https://www.lcsc.com/product-detail/C14877.html), [CH340C, C84681](https://www.lcsc.com/product-detail/C84681.html), [USB receptacle, C165948](https://jlcpcb.com/partdetail/C165948), [resonator, C341521](https://www.lcsc.com/product-detail/C341521.html). LCSC listing does not guarantee availability in a particular JLCPCB assembly tier. |

The Arduino Uno schematic was consulted as a compatibility reference: [Arduino Uno R3 schematic and its licence notice](https://docs.arduino.cc/resources/schematics/A000066-schematics.pdf). This draft is a new drawing with a different USB interface and audio stage; it is not a copy of the Uno PCB layout.

The revision A PCB uses KiCad 10.0.8 footprints frozen in `../pcb/footprints/`. The USB shell identifier is `SH` in that library version. The reset switch uses an original footprint from the [Alps SKRPACE010 dimensions, land pattern and contact diagram](https://tech.alpsalpine.com/e/products/detail/SKRPACE010/). Its upper contacts 1 and 2 are common; lower contacts 3 and 4 are common. The [Murata comparison drawing](https://www.murata.com/~/media/webrenewal/products/timingdevice/ceralock/nem/comparison/dr-e17-0030.ashx?la=en) supports the resonator replacement and land-pattern check.

Exact selected MPNs, part specifications and dated stock/prices are in `../pcb/parts.json` and `../pcb/sourcing.json`. The earlier generic capacitor dielectric/voltage entries were replaced with the selected parts' actual values. Native KiCad ERC, DRC and schematic-parity reports are now retained in `../manufacturing/rev-a/checks/`. Physical tests remain outstanding.
