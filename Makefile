TARGET = fddEMU
MCU = atmega32u4
OSC = 16000000UL

CC = avr-gcc
CXX = avr-g++

INCLUDES	= -I . -I libs/avrFlux -I libs/petitfs  -I /usr/avr/include -I libs/u8glib/csrc
DUAL = 0	#enable drive B = 1
DEBUG = 0	#enable debug = 1
FLIP = 1	#flip scren upside down = 1
WDT = 1		#WDT enabled = 1
VFFS = 0	#Virtual FLoppy Disabled
GUI = 1		#Graphical User Interface enabled
SERIAL = 0	#Serial disabled
PORT = /dev/ttyUSB0

CFLAGS = -Os -mmcu=$(MCU) -DF_CPU=$(OSC) -Wall $(INCLUDES)
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS += -DENABLE_WDT=$(WDT)
CXXFLAGS= -Os -mmcu=$(MCU) -DF_CPU=$(OSC) -Wall $(INCLUDES)
CXXFLAGS += -DENABLE_DRIVE_B=$(DUAL) -DDEBUG=$(DEBUG) -DFLIP_SCREEN=$(FLIP) -DENABLE_WDT=$(WDT) 
CXXFLAGS += -DENABLE_VFFS=$(VFFS) -DENABLE_GUI=$(GUI) -DENABLE_SERIAL=$(SERIAL)
LINKERFLAG = -lm

# Files
EXT_C   = c
EXT_C++ = cpp
EXT_ASM = S

OBJECTS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard src/*.$(EXT_C))) \
	$(patsubst %.$(EXT_C++),%.o,$(wildcard src/*.$(EXT_C++))) \
	$(patsubst %.$(EXT_ASM),%.o,$(wildcard src/*.$(EXT_ASM)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/avrFlux/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/petitfs/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/u8glib/csrc/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/u8glib/sfntsrc/*.$(EXT_C))) 	 	


all: $(TARGET).elf size hex

$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(LINKERFLAG) $^ -o $@

%.o: %.cpp
	${CXX} -c $(CXXFLAGS) $< -o $@

%.o: %c 
	${CC} -c $(CFLAGS) $< -o $@

.PHONY : clean, hex, flash, bin, size, all
hex:
	avr-objcopy -O ihex $(TARGET).elf $(TARGET).hex
bin:
	avr-objcopy -O binary $(TARGET).elf $(TARGET).bin
size:
	avr-size --mcu=$(MCU) -C -x $(TARGET).elf
flash:
	avrdude -p m328p -c arduino -P $(PORT) -U flash:w:$(TARGET).hex
clean :
	rm $(TARGET).elf $(OBJECTS)
