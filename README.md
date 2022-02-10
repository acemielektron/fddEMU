# fddEMU
An AVR (atmega328p) based floppy drive emulator for PC
<br>
[![build for atmega328p & atmega32u4](https://github.com/acemielektron/fddEMU/actions/workflows/build-firmware.yml/badge.svg)](https://github.com/acemielektron/fddEMU/actions/workflows/build-firmware.yml)
<br>
![fddEMU on perfboard](/images/perfboard-2.png)
<br>
<br>
fddEMU is a DIY floppy drive emulator. You can immediately begin testing fddEMU on serial terminal with an arduino nano, an SD adapter that converts 5 volts arduino power supply and signals to 3.3 volts suitable for SD card, an SD card,  and some jumper cables. 
Standalone (using without usb serial connection) require SSD1306 i2c screen, buttons and a 5 volts power supply.
<br><br>

![fddEMU on breadboard](/images/breadboard.png)
<br><br>

**A very compact and configurable PCB design for fddEMU (created by @ikonko)**<br>
Please check [documentation for compact PCB](/images/ikonko/README.md) for detailed explanation and configuration options
![A really compact fddEMU on perfboard, courtes of ikonko](/images/ikonko/fddEMU_v1.png)
<br><br>

**Limitations**
<br>
* fddEMU is not an cycle exact floppy emulator , it is more of a hack (it's a bit slower than actual fdd) but it works because floppy controllers have sensible timeouts and we take advantage of that.
* fddEMU uses MFM encoding (used by HD and DD disks) so there is no FM encoding (used by SD disks) support as of now. But it is possible to implement FM encoding.
* fddEMU uses raw floppy images (images prepared with dd or rawread) does not require or use a MFM file. Converts raw image sectors to MFM on the fly using [ArduinoFDC](https://github.com/dhansel/ArduinoFDC) library.
* fddEMU supports fixed sector size of 512 bytes, other sector sizes are not supported.
* Requires a Floppy Drive Controller (FDC) on the PC side to communicate so it probably wont work with an Amiga.
<br><br>

**Hardware (Required)**
* Arduino Nano or Arduino Pro Micro (UNO will do but button inputs require ADC7, use serial commands instead)
* Micro SD card adapter (converts 5 volts arduino power supply and signals to 3.3 volts)
* A micro SD card (formatted FAT16/32, floppy images in root directory)
* Two 1kOhm resistors for Step (pin 20 of floppy connector) and WriteData (pin 22 of floppy connector). Atmega's internal pullups are not sufficicient for parasitic capacitance of long floppy drive cable
* Male to female jumper wires (for connecting arduino nano pins to FDC ribbon cable)
* Male to male jumper wires (for pullup on step and readdata pins)
<br>

**Hardware (Optional)**
* 0.96" i2c OLED (SSD1306) screen (for output)
* 4 or 5 1kOhm resistors (for making ADC buttons)
* 1 100kOhm resistor (ADC7 has no pullup, so an external pullup required)
* 4 or 5 push buttons (required for ADC input obviously)
* 5 volts power supply (Computer PSU 5v pins or an external PSU can be used)
* Female to female jumper wires (for connecting SD adapter to arduino nano)
* Breadboard or perfboard (for placing pullups and ADC buttons)
<br><br>

**Pin mapping (single drive)**
Arduino Pro Micro   |Arduino Nano   |FDD Ribbon Cable
--------------------|---------------|----------------------
GND                 |GND		    |1	(GND)
RX (PD2)            |D2 (PD2)       |20	(Step pulse)
TX (PD3)            |D3	(PD3)		|18	(Direction)
8  (PB4)            |D4	(PD4)		|16	(Motor on B)
10 (PB6)            |D5	(PD5)		|12	(Drive Select B)
5  (PC6)            |D6	(PD6)		|32	(Select head)
6  (PD7)            |D7	(PD7)		|8	(Index pulse)
4  (PD4)            |D8	(PB0)		|22	(Write data)
9  (PB5)            |D9	(PB1)		|30	(Read data)
7  (PE6)            |A0	(PC0)		|24	(Write enable)
A3 (PF4)            |A1	(PC1)		|26	(Track zero)
24 (PD5 - TXLED)    |A2	(PC2)	    |28	(Write protect)
A1 (PF6)            |A3	(PC3)		|34	(Disk changed)

<br>

**Pin mapping (dual drive)**
Arduino Pro Micro   |Arduino Nano   |FDD Ribbon Cable
--------------------|---------------|----------------------
GND                 |GND		    |1	(GND)
RX (PD2)            |D2 (PD2)       |20	(Step pulse)
TX (PD3)            |D3	(PD3)		|18	(Direction)
8  (PB4)            |D4	(PD4)		|14	(Drive Select A)
10 (PB6)            |D5	(PD5)		|12	(Drive Select B)
5  (PC6)            |D6	(PD6)		|32	(Select head)
6  (PD7)            |D7	(PD7)		|8	(Index pulse)
4  (PD4)            |D8	(PB0)		|22	(Write data)
9  (PB5)            |D9	(PB1)		|30	(Read data)
7  (PE6)            |A0	(PC0)		|24	(Write enable)
A3 (PF4)            |A1	(PC1)		|26	(Track zero)
24 (PD5 - TXLED)    |A2	(PC2)	    |28	(Write protect)
A1 (PF6)            |A3	(PC3)		|34	(Disk changed)

<br>

**Micro SD Adapter connections**
Arduino Pro Micro   |Arduino Nano   |Micro SD adapter
--------------------|---------------|-------------------
A0 (PF7)            |D10 (PB2)  	|CS
16 (PB2)            |D11 (PB3)	    |MOSI
14 (PB3)            |D12 (PB4)		|MISO
15 (PB1)            |D13 (PB5)		|SCK
VCC                 |5V             |VCC
GND                 |GND            |GND

<br>

**0.96" OLED SSD1306 connections**
Arduino Pro Micro   |Arduino Nano   |0.96" OLED SSD1306
--------------------|---------------|-------------------
GND                 |GND            |GND
VCC                 |5V             |VCC
2  (PD1)			|A4			    |SDA
3  (PD0)			|A5			    |SCL

<br><br>

**Resistor ladder for ADC buttons**
<br>
Connected to pin A7 (ADC7) on Arduino Nano or pin A2 (PF5) on Arduino Pro Micro
<br>
![Resistor Ladder 1x5](/images/ResistorLadder-1x5.png)
<br><br>

**Firmware**
<br>
Download with submodules(u8glib & lufa): `git clone https://github.com/acemielektron/fddEMU --recursive`<br>
If not downloaded with recursive option run (in fddEMU folder): `git submodule update --init --recursive`<br>
Current Makefile assumes avr-gcc and avr-binutils are installed and are in the path, 
avr specific includes are in "/usr/avr/include" (arch) or "/usr/lib/avr/include" (ubuntu) <br>
run `make` to build fddEMU.hex with default options.
<br>
**build options:**
* *GUI:* enables graphical user interface on OLED screen and ADC buttons (enabled default by Makefile). 
* *SERIAL:* enables serial user interface and serial commands.
* *VFFS:* enables virtual disk.
* *DEBUG:* enables debug output on serial.
* *FLIP:* flips the image on OLED screen 180 degrees (enabled default by Makefile).
* *WDT:* enables watchdog timer (enabled default by Makefile).<br><br>
These build options could either be switched on and off from Makefile or commandline 
(eg.: `make DUAL=1 GUI=1 VFFS=1 SERIAL=0 DEBUG=0`).<br>
Flashing the mcu could also be done by Makefile. `make flash` programs "fddEMU.hex" to 
default serial port "/dev/ttyUSB0". If you want to use another serial port assign it to 
PORT variable (eg.: `make flash PORT=/dev/ttyUSB2`).

**Note:** Due to larger bootloader on Atmega32U4 and added code overhead for USB-Serial all features of fddEMU will be avaialble on Atmega32U4

***Warning:*** *Writing > 28K (28672 bytes) firmware to Arduino Pro Micro results in bootloader being overwritten and requires reprogramming the bootloader using external programmer (eg. USBASP). Please check output of avr-size at the end of make process and make sure it is < 28K.*
<br><br>

**How to use**<br>
* fddEMU looks for a boot record on "sector 0" of the image. If the boot record reports that the image is formatted as "FAT1?", sector size is 512, number of heads is 2 and number of tracks and number of sectors are less than 255 the image file is loaded with the settings provided by the boot record. To be able to read these custom FAT12 images host system should support provided number of tracks and sectors. If there is no boot record file size is compared to the standart floppy sizes (1.44M, 1.2M, 720K and 360K), if file size matches one of these sizes it is loaded as raw image.
* Image file must be contiguous for fddEMU to be able to load, if the file is not contiguous an error message will be shown and loading will fail.
* Contiguity check would take long for large files (> 2MB) during this check fddEMU can not be used.
* For booting a host system, on startup fddEmu looks for "BOOT.IMG" on SD card. If there is a "BOOT.IMG" on the SD card fddEMU tries to load this file to drive A. 
* if image file has "read only" attribute set, fddEMU will report "Write Protected" to host system.
* To protect OLED screen fddEMU will put screen to sleep after some idle time, press "S1" (SELECT) button or "S" key inside serial terminal to wake screen up.
* When host is reading/writing one of the emulated drives, BUSY message will be shown on screen and serial, during this time fddEMU will not respond to input. *Warning: fddEMU will not put the screen to sleep while drive is active. If there is a hardware or software problem causing an emulated drive to go continuously active, OLED screen might get damaged.*
<br><br>

**How to use (GUI button interface)**
* S1: Load Virtual Disk
* S2: Next
* S3: Previous
* S4: Open file selection Menu / Load selected file
* S5: Eject selected disk / Cancel loading file
<br>
On the main screen, drive 0 "A" and -if enabled- drive 1 "B" are displayed. To use any of the functions a drive must first be selected through "S2" or "S3" buttons.<br>
After a drive is selected:<br>
"S1" button: in main status screen, if virtual disk is enabled, loads the virtual disk to selected drive.<br>
"S2" button: in main status screen selects drive down, in file selection menu selects file down.<br>
"S3" button: in main status screen selects drive up, in file selection menu selects file up.<br>
"S4" button: in main status screen opens file selection menu, in file selection menu loads the selected image file to the selected drive.<br>
"S5" button: in main status screen ejects image file loaded to the selected drive, in file section menu cancels file selection and returns to main status screen.
<br><br>

**How to use (Serial interface)**
<br>
![initial serial output](/images/serial-init.png)
<br><br>
fddEMU serial interface is used through keys 
* S: Select
* N: Next file
* P: Previous file
* L: Load selected file
* E: Eject selected disk / Cancel loading file
<br>
To use any of the functions a drive must first be selected through "S" key then serial terminal will report "Sel drive: A" or if drive B is selected "Sel drive: B".<br>
After a drive is selected:<br>
"S" key: shows serial status information and selects next drive.<br>
"N" key: selects next image file. If virtul disk is enabled and if the last file in the directory is reached selects virtual disk .<br>
"P" key: selects the previous image file. If the first file is selected, this file is reselected.<br>
"L" key: loads selected image file to the selected drive.<br>
"E" key: ejects image file loaded to selected drive. If pressed during file selection, cancels file selection.
<br><br>

**How to use through host system (Virtual Disk)**
<br>
-if virtual disk (VFFS) is enabled and loaded- Virtual disk root directory contains a "DISKS" directory, a "DRVA.TXT" file and if DUAL drives enabled a "DRVB.TXT" file. If an SD card is inserted "DISKS" directory contains the file listing of inserted SD card's root directory otherwise it will be empty. "DRVA.TXT" would contain name of the image file loaded the "drive 0" and "DRVB.TXT" would contain name of the image file loaded to "drive 1". Writing the name of an image file to either "DRVA.TXT" or "DRVB.TXT" would result loading requested image file to respective emulated drive upon host releasing the drive. However disk write cache might interfere writing into these files and a cache flush might be required.
<br><br>

**Troubleshooting**
* To test fddEMU or any other floppy drive emulator a Floppy Drive Controller (FDC) is very useful. Unfortunately new computers usually don't come with a FDC. Thankfully we already have a very good FDC that generates the precise debug information we need: the [ArduinoFDC](https://github.com/dhansel/ArduinoFDC). A direct pin to pin connection from fddEMU to ArduinoFDC works, if using shorter jumper wires external pullups are not necessary.
* If using a floppy drive ribbon cable make sure of external pullup resistors to +5V on Step (Pin 20 on floppy connector) and WriteData (pin 22 on floppy connector) pins.
* If WiteData pin (pin 22 on floppy connector) is disconnected or does not have sufficient external pullup fddEMU might hang up while reading data from host, which might cause a static image on screen. To protect the screen currently a watchdog timer of 8 seconds is set (which is the longest duration for watchdog timer). If mcu (arduino) can't access SD card after reset remove the sd card, wait a few seconds, reinsert the SD card then reset the arduino.

**Acknowledgements**
<br>
* [ArduinoFDC](https://github.com/dhansel/ArduinoFDC) by [David Hansel](https://github.com/dhansel). fddEMU uses a modified ArduinoFDC library renamed to avrFlux for communicating with FDC.
* [Petit FAT FS](https://github.com/greiman/PetitFS) by [Chan](http://elm-chan.org/fsw/ff/00index_p.html) adapted for hardware SPI by [Bill Greiman](https://github.com/greiman). fddEMU uses a modified Petit FAT FS.
* [rjhcoding.com/avrc-sd-interface](http://rjhcoding.com/avrc-sd-interface-1.php) Great tutorial for using SD card with AVR.
* [u8glib](https://github.com/olikraus/u8glib) Very useful for incorporating SSD1306 and can easily be used with various displays.
* [lufa](https://github.com/abcminiuser/lufa) Lightweight USB Framework for AVRs
<br><br>

**Releases**
<br>
* [fddEMU releases](https://github.com/acemielektron/fddEMU/releases)
<br><br>

**Note:**  Please report any errors on [github issues for fddEMU](https://github.com/acemielektron/fddEMU/issues). Suggestions for improvements and feedback could be posted on [discussions](https://github.com/acemielektron/fddEMU/discussions) or [fddEMU blog page](http://acemielektronikci.blogspot.com/2021/10/fddemu-disket-surucu-emulatoru.html) Although the blog page is in turkish, feel free to write comments either in turkish or english.
<br><br>
