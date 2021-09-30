# fddEMU
An AVR (atmega328p) based floppy emulator for PC

fddEMU is a DIY floppy emulator. You can immediately begin testing with an arduino nano, an SD adapter, SD card,  and some jumper cables. 
Standalone (using without usb serial connection) require SSD1306 i2c screen, buttons and a 5 volts power supply.

**Requirements**
* Arduino nano (UNO will do but button inputs require ADC7, use serial commands instead)
* Micro SD card adapter
* A micro SD card (formatted FAT16/32, floppy images in root directory)
* 2 1kOhm resistors (for step and readdata pins - atmega's internal pullups are not sufficicient for parasitic capacitance of long floppy drive cable)
* Male to female jumper wires (for connecting arduino nano pins to FDC ribbon cable)
* Male to male jumper wires (for pullup on step and readdata pins)

**Optional**
* 0.96" i2c OLED (SSD1306) screen (for output)
* 5 1kOhm resistors (for making ADC buttons)
* 1 100kOhm resistor (ADC7 has no pullup, so an external pullup required)
* 5 push buttons (required for ADC input obviously)
* 5 volts power supply (Computer PSU 5v pins or an external PSU can be used)

* Female to female jumper wires (for connecting SD adapter to arduino nano)
* Breadboard (for placing pullups and ADC buttons)

**Programming**

Arduino bootloader could be used for uploading

*avrdude -B 10 -p m328p -c arduino -P /dev/ttyUSB0 -U flash:w:fddEMU.hex*

simply replace /dev/ttyUSB* with your usb port

**Limitations**
* Uses raw floppy images (images prepared with dd or rawread) does not require or use a MFM file. Converts raw image sectors to MFM on the fly using [ArduinoFDC](https://github.com/dhansel/ArduinoFDC) library.
* fddEMU supports fixed sector size of 512 bytes, other sector sizes are not supported
* Requires a Floppy Drive Controller (FDC) on the PC side to communicate so it probably wont work with an Amiga.
* Repeated write on same SD card sectors might lead to SD failure so use floppy images in read only mode (set read only attribute) when possible. Modern O.S. such as linux write and clear the dirty bit every mount/umount even if you dont write anaything to floppy image.

**Acknowledgements**
* [ArduinoFDC](https://github.com/dhansel/ArduinoFDC) by [David Hansel](https://github.com/dhansel). fddEMU uses a modified ArduinoFDC library renamed to avrFlux for communicating with FDC.
* [Petit FAT FS](https://github.com/greiman/PetitFS) by [Chan](http://elm-chan.org/fsw/ff/00index_p.html) adapted for hardware SPI by [Bill Greiman](https://github.com/greiman). fddEMU uses a modified Petit FAT FS.
* [u8glib](https://github.com/olikraus/u8glib)

Pictures and further documentation is coming ...

