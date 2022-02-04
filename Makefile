TARGET = fddEMU
#MCU = atmega32u4
MCU = atmega328p
OSC = 16000000UL

CC = avr-gcc
CXX = avr-g++

INCLUDES	= -I ./ -I libs/avrFlux -I libs/petitfs  -I /usr/avr/include -I libs/u8glib/csrc
INCLUDES	+= -I ./usb-cdc -I ./libs -I ./libs/LUFA/Drivers/
DUAL = 0	#enable drive B = 1
DEBUG = 0	#enable debug = 1
FLIP = 1	#flip scren upside down = 1
WDT = 1		#WDT enabled = 1
VFFS = 0	#Virtual FLoppy Disabled
GUI = 1		#Graphical User Interface enabled
SERIAL = 0	#Serial disabled
#OPT=3

ifeq	($(MCU),atmega32u4) 
	PORT = /dev/ttyACM0
else
	PORT = /dev/ttyUSB0
endif

# Files
EXT_C   = c
EXT_C++ = cpp
EXT_ASM = S

LUFAOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./usb-cdc/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/LUFA/Drivers/USB/Core/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/LUFA/Drivers/USB/Core/AVR8/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/LUFA/Drivers/USB/Core/AVR8/Template/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/LUFA/Drivers/USB/Class/Device/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/LUFA/Drivers/USB/Class/Common/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/LUFA/Drivers/USB/Peripheral/AVR8/*.$(EXT_C))) 	

U8GOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/u8glib/csrc/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/u8glib/sfntsrc/*.$(EXT_C))) 

PFSOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/petitfs/*.$(EXT_C)))

FLUXOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/avrFlux/*.$(EXT_C)))

SRCOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./src/*.$(EXT_C))) \
	$(patsubst %.$(EXT_C++),%.o,$(wildcard ./src/*.$(EXT_C++))) \
	$(patsubst %.$(EXT_ASM),%.o,$(wildcard ./src/*.$(EXT_ASM)))

OBJECTS := $(SRCOBJS) $(FLUXOBJS) $(PFSOBJS) $(U8GOBJS)
ifeq	($(MCU),atmega32u4) 
	OBJECTS += $(LUFAOBJS)
endif

COMMONFLAGS = -O$(OPT) -mmcu=$(MCU) -DF_CPU=$(OSC) -Wall $(INCLUDES)
COMMONFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
LUFAFLAGS = -DF_USB=$(OSC) -DUSE_LUFA_CONFIG_HEADER
SRCFLAGS += -DENABLE_DRIVE_B=$(DUAL) -DDEBUG=$(DEBUG) -DFLIP_SCREEN=$(FLIP) -DENABLE_WDT=$(WDT) 
SRCFLAGS += -DENABLE_VFFS=$(VFFS) -DENABLE_GUI=$(GUI) -DENABLE_SERIAL=$(SERIAL) $(LUFAFLAGS)

#$(SRCOBJS): CXXFLAGS := $(COMMONFLAGS) $(SRCFLAGS)
#$(SRCOBJS): CFLAGS := $(COMMONFLAGS) $(SRCFLAGS)
#$(FLUXOBJS): CFLAGS := $(COMMONFLAGS) $(SRCFLAGS)
#$(PFSOBJS): CFLAGS := $(COMMONFLAGS) $(SRCFLAGS)
#$(U8GOBJS): CFLAGS := $(COMMONFLAGS)
#$(LUFAOBJS): CFLAGS := $(COMMONFLAGS) $(LUFAFLAGS)
CFLAGS = $(COMMONFLAGS) $(SRCFLAGS) $(LUFAFLAGS)
CXXFLAGS = $(COMMONFLAGS) $(SRCFLAGS) $(LUFAFLAGS)
LINKERFLAG = -lm	 	


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
ifeq	($(MCU),atmega32u4) 
	avrdude -p $(MCU) -c avr109 -P $(PORT) -U flash:w:$(TARGET).hex
else
	avrdude -p $(MCU) -c arduino -P $(PORT) -U flash:w:$(TARGET).hex
endif
clean :
	rm $(TARGET).elf $(OBJECTS)
