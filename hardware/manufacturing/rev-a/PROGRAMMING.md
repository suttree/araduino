# Initial programming and USB updates

These files target the genuine ATMEGA328P-AU at 5 V and 16 MHz. Do not substitute ATmega328PB or a different processor. Build configuration: Arduino AVR 1.8.6, `arduino:avr:uno`, bundled Mozzi 2.0.2 and FixMath 1.0.7.

The quoted assembly must include initial programming if the boards are to work on delivery. Soldering a blank chip onto the board does not install Arduino's bootloader or the birdsong application. Ask JLCPCB to confirm support, its required programmer/fixture, and the cost before ordering.

## Factory ISP procedure

Use J2, the standard 2 × 3 AVR ISP header. The square pad identifies pin one. From the top, its numbering is:

```text
1 MISO       2 +5V reference
3 SCK        4 MOSI
5 RESET      6 GND
```

Power the target from a regulated 5 V USB supply. The programmer must sense the target's voltage and must not supply a second independent voltage to J2 pin 2. There is no power-source arbitration on this board.

1. Read and verify the ATmega328P signature `0x1E950F`. Use an ISP clock suitable for the blank chip's default clock, such as 125 kHz, before setting the clock fuses.
2. Erase and program with the Arduino Uno bootloader procedure. The Uno settings in Arduino AVR 1.8.6 are low fuse `0xFF`, high fuse `0xDE`, extended fuse `0xFD`, unlock `0x3F` and final lock `0x0F`. Apply the toolchain's ATmega328P extended-fuse masking when verifying unused bits.
3. Write `firmware/ara-birdsong-with-bootloader.hex` as the combined flash image. It includes the application at address zero and Optiboot at byte address `0x7E00`. Verify the full programmed image before applying the final lock settings.
4. Read back the fuses and confirm that reset, SPI programming, bootloader start and the external 16 MHz clock are enabled as intended.
5. Remove the programmer, connect the specified speaker, and cycle USB power. The current application starts one randomly generated song on startup.
6. Confirm a normal Arduino Uno serial upload works through the USB-C port at 115200 baud. This also tests the CH340C and automatic reset circuit.

The HEX files do not encode fuse or lock settings. Supplying the combined HEX alone is insufficient. JLCPCB must agree its exact programming sequence before the quote is accepted. This procedure is prepared from the installed Arduino toolchain and has not been tried on the custom PCB.

## Later firmware changes

Once initial programming is complete, the owner can use Arduino IDE with board `Arduino Uno` and the CH340 serial port. Compile `sketches/ara_birdsong` with the repository's bundled libraries. The separate `firmware/ara-birdsong.hex` is the application image for serial uploads that preserve the bootloader.

The packaged snapshot uses 350% software gain and random singing attempts between nine and 120 minutes. Each attempt has a 50% chance of singing. Song frequency remains under testing. This packaging work did not upload firmware to the owner's Arduino or change its timing.
