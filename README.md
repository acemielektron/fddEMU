# fddEMU
An AVR (atmega328p) based floppy emulator for PC

fddEMU is a DIY floppy emulator. You can immediately begin testing fddEMU on serial terminal with an arduino nano, an SD adapter that converts 5 volts arduino power supply and signals to 3.3 volts suitable for SD card, an SD card,  and some jumper cables. 
Standalone (using without usb serial connection) require SSD1306 i2c screen, buttons and a 5 volts power supply.
<br><br>

**Requirements**
* Arduino nano (UNO will do but button inputs require ADC7, use serial commands instead)
* Micro SD card adapter (converts 5 volts arduino power supply and signals to 3.3 volts)
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
<br><br>

**Programming**
<br>
Arduino bootloader could be used for uploading (see command below), simply replace /dev/ttyUSB0 with your usb port
<br>
*avrdude -B 10 -p m328p -c arduino -P /dev/ttyUSB0 -U flash:w:fddEMU.hex*
<br><br>

**Pin mapping**
Pin         |Arduino Nano|FDD Ribbon Cable
------------|-----------|----------------------
GND			|GND		|1	(GND)
STEP		|D2			|20	(Step pulse)
STEP_DIR	|D3			|18	(Direction)
MOTOR_A		|D4			|16	(Motor on B)
SELECT_A	|D5			|12	(Drive Select B)
SIDE		|D6			|32	(Select head)
INDEX		|D7			|8	(Index pulse)
READDATA	|D8			|22	(Write data)
WRITEDATA	|D9			|30	(Read data)
WRITE_GATE	|A0			|24	(Write enable)
TRACK_0		|A1			|26	(Track zero)
WRITE_PROTECT|A2		|28	(Write protect)
DISK_CHANGE	|A3			|34	(Disk changed)

<br><br>

Pin			|Arduino Nano|Micro SD adapter
------------|------------|-------------------
Slave Select|D10			|CS
MOSI		|D11			|MOSI
MISO		|D12			|MISO
SCK			|D13			|SCK
VCC         |5V             |VCC
GND         |GND            |GND

<br><br>

Pin			|Arduino Nano|0.96" OLED SSD1306
------------|------------|-------------------
GND         |GND        |GND
VCC         |5V         |VCC
SDA			|A4			|SDA
SCL			|A3			|SCL

<br><br>

**Resistor ladder for ADC buttons**
<br>
![Resistor Ladder 1x5](/images/ResistorLadder-1x5.png)
<br><br>
**Limitations**
<br>
* Uses raw floppy images (images prepared with dd or rawread) does not require or use a MFM file. Converts raw image sectors to MFM on the fly using [ArduinoFDC](https://github.com/dhansel/ArduinoFDC) library.
* fddEMU supports fixed sector size of 512 bytes, other sector sizes are not supported
* Requires a Floppy Drive Controller (FDC) on the PC side to communicate so it probably wont work with an Amiga.
* Repeated write on same SD card sectors might lead to SD failure so use floppy images in read only mode (set read only attribute) when possible. Modern O.S. such as linux write and clear the dirty bit every mount/unmount even if you dont write anything to floppy image.
<br><br>

**Acknowledgements**
<br>
* [ArduinoFDC](https://github.com/dhansel/ArduinoFDC) by [David Hansel](https://github.com/dhansel). fddEMU uses a modified ArduinoFDC library renamed to avrFlux for communicating with FDC.
* [Petit FAT FS](https://github.com/greiman/PetitFS) by [Chan](http://elm-chan.org/fsw/ff/00index_p.html) adapted for hardware SPI by [Bill Greiman](https://github.com/greiman). fddEMU uses a modified Petit FAT FS.
* [u8glib](https://github.com/olikraus/u8glib)
<br><br>

**Releases**
<br>
* [fddEMU v0.9](https://github.com/acemielektron/fddEMU/releases)
<br><br>

**How to use**<br>
* fddEMU supports standart floppy image files of 360K, 720K, 1.2M and 1.44M. If selected image file size match one of these sizes the image is loaded as raw floppy image and number of tracks and sectors are selected accordingly. 
* If selected image file size doesn't match one of the standart floppy sizes, fddEMU looks for a boot record on "sector 0" of the image. If the boot record reports that the image is formatted as "FAT12", sector size is 512, number of heads is 2 and number of tracks and number of sectors are less than 255 the image file is loaded with the settings provided by the boot record. To be able to read these custom FAT12 images host system should support provided number of tracks and sectors.
* Image file must be contiguous for fddEMU to be able to load, if the file is not contiguous an error message will be shown and loading will fail.
* For booting a host system, on startup fddEmu looks for "BOOT.IMG" on SD card. If there is a "BOOT.IMG" on the SD card fddEMU tries to load this file to drive A. 
* To protect OLED screen fddEMU will put screen to sleep after some idle time, press "S1" (SELECT) button or "S" key inside serial terminal to wake screen up.
* When host selected (reading/writing) one the emulated drives, the selected (active) drive's icon become inverted and stays this way till host unselects the drive. During this time fddEMU will not accept input.<br>*Warning: fddEMU will not put the screen to sleep while drive is active. If there is a hardware or software problem causing an emulated drive to go continuously active OLED screen might be damaged.*

<br><br>

fddEMU button interface:
* S1: Select
* S2: Next file
* S3: Previous file
* S4: Load selected file
* S5: Eject selected disk / Cancel loading file

On the main screen, drive A and if enabled drive B are displayed. To use any of the functions a drive must first be selected through "S1" button then a box frame will be shown around the selected drive.<br>
After a drive is selected:<br>
"S1" button selects the next drive or if last drive in the list is selected deselects drive<br>
"S2" button opens the file selection menu on the screen. Pressing again selects next file in the menu. If end of the menu is reached last file selected.<br>
"S3" button opens the file selection menu on the screen. Pressing again selects previous file in the menu. If start of the menu is reached first file is selected.<br>
"S4" button loads the image file selected inte menu to the selected drive.<br>
"S5" button ejects image file loaded to the selected drive. If pressed during file selection menu, cancels file selection menu and returns to main screen.<br>

<br><br>
**How to use (Serial)**
<br>
![initial serial output](/images/serial-init.png)
<br><br>
fddEMU serial interface is used through keys 
* S: Select
* N: Next file
* P: Previous file
* L: Load selected file
* E: Eject selected disk / Cancel loading file

To use any of the functions a drive must first be selected through "S" key then serial terminal will report "Sel drive: A" or if drive B is selected "Sel drive: B".<br>
After a drive is selected:<br>
"S" key selects the next drive or if last drive in the list is selected deselects drive<br>
"N" key selects next file in root directory. If the last file in the root directory is selected this file is reselected.<br>
"P" key selects the previous file in the root directory listing. If the first file is selected, this file is reselected.<br>
"L" key loads the selected image file to the selected drive.<br>
"E" key ejects image file loaded to selected drive. If pressed during file selection, cancels file selection.<br>


<br><br>
Pictures and further documentation is coming ...

