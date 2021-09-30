# fddEMU
An AVR (atmega328p) based floppy emulator for PC

**Requirements**
* Arduino nano (UNO will do but button inputs require ADC7, use serial comands instead)
* Micro SD card adapter
* A micro SD card (formatted FAT16/32, floppy images in root directory)
* 7 1kOhm resistors (2 for step and readdata pins, the rest required for making ADC buttons)
* 1 100kOhm resistor (ADC7 has no pullup, so an external pullup required)
* 5 push buttons (required for ADC input obviously)
* Male to female jumper wires (for connecting arduino nano pins to FDC ribbon cable)
* Female to female jumper wires (for connecting SD adapter to arduino nano)
* Male to male jumper wires (for pullup on step and readdata pins)
* Breadboard (for placing pullups and ADC buttons)

**Programming**
Arduino bootloader could be used for uploading
*avrdude -B 10 -p m328p -c arduino -P /dev/ttyUSB0 -U flash:w:fddEMU.hex*
simply replace /dev/ttyUSB* with your usb port

Pictures and further documentation is coming ...

