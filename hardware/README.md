# Ara custom board brief

Status on 25 November 2026: the [revision A prototype design](schematic/README.md) now has a routed 65 × 45 mm PCB and a [manufacturing package](manufacturing/rev-a/README.md). KiCad 10.0.8 reports zero electrical-rule, board-rule, unconnected-net and schematic-consistency findings. The circuit has not been physically tested. The package is ready for a prototype quotation, subject to the factory's part matching and placement review.

The owner has chosen to compare the cost of five and ten assembled prototypes before deciding whether to pay for an external engineer's review. No order has been placed. See the [cost worksheet](pcb/COSTS.md) for the dated component prices and the charges still to quote.

## Agreed requirements

- Base the first board on the working Elegoo Uno R3 prototype and its ATmega328P processor. An ESP32 port is not required for this version.
- Use the current `sketches/ara_birdsong/ara_birdsong.ino` firmware. Preserve its voice, pitch, phrases and adjustable song timing.
- Use USB-C for power instead of the existing barrel socket.
- Include the speaker amplifier on the PCB. Final assembly must require no soldering by the owner.
- Use a separate enclosed passive speaker with a factory-fitted two-pin JST-PH cable and a matching PCB socket.
- Design the enclosure around the board. The existing plant pot concept provides the starting point; there is no fixed board outline yet.
- Initial batch: five or ten fully assembled prototypes. Proposed ordering approach: compare quotes for both quantities and start with five unless the ten-board quote justifies the extra units.

## Existing prototype

The supplied photos show an Elegoo Uno R3 connected directly to a speaker through GND and digital pin 9. The bundled Mozzi configuration uses pin 9 for mono output on the Uno. No separate amplifier is visible.

The existing CQRobot speaker is the 4-ohm enclosed model sold through Amazon as B0822XCPT8. Its manufacturer specifies approximately 70 × 31 × 16 mm and 2 W rated / 3 W maximum power. It currently has Dupont leads.

The proposed plug-in replacement is The Pi Hut's mono enclosed speaker, SKU 102826: 4 ohms, advertised as 3 W, approximately 70 × 30 × 17 mm, with a factory-fitted JST-PH two-pin cable. Its sound and fit remain untested. JST-PH uses friction retention rather than a release latch. Secure the board and speaker and provide cable slack and strain relief inside the enclosure.

## Circuit design requirements

- Preserve the ATmega328P's 5 V operation and 16 MHz clock as the compatibility starting point. Confirm the exact firmware build configuration before schematic sign-off.
- Feed the PWM audio through a suitable filter and analogue-input speaker amplifier. Do not reproduce the direct 4-ohm speaker load on the processor pin.
- Set amplifier gain and output limits for quiet birdsong and the chosen speaker. Compare the result with the working prototype before approving production.
- Do not substitute the ESP32 prototype's MAX98357 amplifier directly into the pin-9 circuit. It requires I2S digital audio.
- Implement USB-C as a 5 V power sink, including the required configuration-channel terminations for USB-C-to-USB-C supplies. No USB Power Delivery voltage negotiation is required.
- Select and review the firmware programming method. The proposed preference is programming through the same USB-C connector, using a suitable USB-to-serial circuit and bootloader. Include an accessible recovery programming interface.
- Provide mounting holes, readable connector labels and a board revision identifier. Confirm speaker connector orientation and polarity against a physical sample.
- Select components and connectors that JLCPCB can source and assemble. Confirm that the quote includes every connector and any through-hole assembly.

## Work before ordering

1. Upload the Gerbers, bill of materials and placement file for five- and ten-board quotes. Confirm every part is matched and included, including through-hole soldering and initial firmware programming.
2. Inspect the factory's component placement preview for pin-one orientation, diode polarity and connector orientation. The supplied CPL applies the retained supplier-specific corrections; check them again in the factory preview.
3. Compare the delivered price with the owner's budget. Decide whether to buy an external review before ordering the prototype batch.
4. Source the separate enclosed speaker with its factory-fitted JST-PH cable. Its sound, cable polarity and retention remain physical checks.
5. Follow the [arrival test](manufacturing/rev-a/TESTING.md). Electrical consumption, USB startup/suspend behaviour and the new amplifier's sound remain unmeasured.

## References

- [Firmware](../sketches/ara_birdsong/ara_birdsong.ino)
- [Bundled Mozzi AVR output configuration](../sketches/libraries/Mozzi/internal/config_checks_avr.h)
- [CQRobot speaker specifications](https://cqrobot.com/product/CQRLB4O3W-B)
- [Proposed JST-PH speaker](https://thepihut.com/products/mono-enclosed-speaker-3w-4-ohm)
- [Mozzi output circuit guidance](https://sensorium.github.io/Mozzi/learn/output_circuits/)
- [JLCPCB assembly ordering requirements](https://jlcpcb.com/help/article/pcb-assembly-faqs)
