# Testing the delivered prototype boards

The owner can perform these checks without soldering. Label the five or ten boards so results stay associated with each unit. Keep the working Arduino nearby as the sound reference.

## First power-up

1. Inspect the board for loose connectors, bent header pins, visible solder bridges or shipping damage. Check that the socket labels and board revision match the order.
2. With USB unplugged, fit the enclosed 4-ohm speaker's factory-fitted JST-PH plug. Do not connect either speaker terminal to ground. The amplifier drives both leads.
3. Put the board on a clean, nonconductive surface and connect a regulated 5 V USB power adapter. Check that the startup song plays. Unplug it if a component becomes unusually hot, smells burnt or repeatedly restarts.
4. Compare its volume, high notes, tone and distortion with the working Arduino. Listen for a persistent hiss, buzz or whine after the song ends. Note clicks when power is connected and removed.
5. Repeat with both orientations of the USB-C plug. Test a USB-C-to-USB-C cable and a USB-A-to-USB-C cable with suitable supplies. The board should run in both orientations.

## Firmware updates and song timing

Connect to a computer and confirm the CH340 serial port appears. Upload the current sketch as an Arduino Uno and confirm it restarts and sings. Repeat one USB upload to each board. A missing bootloader or incorrect fuse settings requires ISP programming; ask the factory or an equipped helper to resolve it.

The packaged production sketch waits a random nine to 120 minutes between attempts, with a 50% singing chance at each attempt. A long quiet period is therefore expected. Use power cycling to test the startup song without waiting hours. For a longer sound test, deliberately build a temporary frequent-song version, then restore the intended firmware and record its source commit.

## Sustained operation and enclosure

Run each board for several days, including overnight. Record resets, unexpected silence, changing volume and background noise. A USB power meter can help compare current across boards, but is not needed for the initial listening checks.

Build the enclosure around the board, speaker and both cable plugs. Allow for the ISP header's height and keep its pins clear of conductive material. Use 3.2 mm mounting holes with suitable M3 hardware and keep washers within the 6.4 mm copper-free circles. Do not flex the PCB to fit the enclosure.

Secure the board and speaker independently. Leave cable slack and strain relief so handling does not pull on the JST-PH plug or USB socket. The JST-PH plug uses friction retention. Pack the device as intended for shipping, handle and move the package, then reopen it and confirm everything remains seated and works.

## If something fails

Record the board identifier, supply/cable, what happened, and whether the working Arduino still behaves normally with its own setup. Swap only one external item at a time. Stop using a board that overheats or has an obvious electrical fault.

Electrical measurements become useful when a failure needs diagnosis. An engineer can check supply voltage, startup current, clock, audio waveform and amplifier behaviour. Successful listening tests alone do not measure those quantities. Resolve repeated failures and confirm the fix on assembled boards before treating this revision as ready for customer units.
