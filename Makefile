TARGET = fddEMU

#Default build options
DUAL = 0	#enable drive B = 1
DEBUG = 0	#enable debug = 1
FLIP = 0	#flip scren upside down = 1
WDT = 1		#WDT enabled = 1
VFFS = 0	#Virtual FLoppy Disabled
GUI = 1		#Graphical User Interface enabled
SERIAL = 1	#Serial disabled

#if DEBUG is enabled, enable SERIAL
ifneq ($(DEBUG),0)
	SERIAL = 1
endif 

# Files
EXT_C   = c
EXT_C++ = cpp
EXT_ASM = S

LUFAOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./src/usb-cdc/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/lufa/LUFA/Drivers/USB/Core/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/lufa/LUFA/Drivers/USB/Core/AVR8/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/lufa/LUFA/Drivers/USB/Core/AVR8/Template/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/lufa/LUFA/Drivers/USB/Class/Device/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/lufa/LUFA/Drivers/USB/Class/Common/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./libs/lufa/LUFA/Drivers/USB/Peripheral/AVR8/*.$(EXT_C))) 	

SERIALOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard src/serial/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C++),%.o,$(wildcard src/serial/*.$(EXT_C++)))

GUIOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard src/gui/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C++),%.o,$(wildcard src/gui/*.$(EXT_C++))) \
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/u8glib/csrc/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C),%.o,$(wildcard libs/u8glib/sfntsrc/*.$(EXT_C)))

VFFSOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard src/vffs/*.$(EXT_C)))\
	$(patsubst %.$(EXT_C++),%.o,$(wildcard src/vffs/*.$(EXT_C++)))

SRCOBJS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./src/avrFlux/*.$(EXT_C))) \
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./src/petitfs/*.$(EXT_C))) \
	$(patsubst %.$(EXT_C),%.o,$(wildcard ./src/*.$(EXT_C))) \
	$(patsubst %.$(EXT_C++),%.o,$(wildcard ./src/*.$(EXT_C++))) 
#$(patsubst %.$(EXT_ASM),%.o,$(wildcard ./src/*.$(EXT_ASM)))
#$(patsubst %.$(EXT_ASM),%.o,$(wildcard ./src/avrFlux/*.$(EXT_ASM))) \	

#MCU = atmega32u4
MCU = atmega328p
OSC = 16000000UL

CC = avr-gcc
CXX = avr-g++
INCLUDES	= -I ./ -I src  -I /usr/avr/include -I /usr/lib/avr/include 

CFLAGS = -Os -mmcu=$(MCU) -DF_CPU=$(OSC) -Wall $(INCLUDES)
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS += -DENABLE_WDT=$(WDT)
#CFLAGS += --save-temps #save temporary files (.s,.i,.ii)
CXXFLAGS = -Os -mmcu=$(MCU) -DF_CPU=$(OSC) -Wall $(INCLUDES)
CXXFLAGS += -DENABLE_DRIVE_B=$(DUAL) -DDEBUG=$(DEBUG) -DFLIP_SCREEN=$(FLIP) -DENABLE_WDT=$(WDT) 
CXXFLAGS += -DENABLE_VFFS=$(VFFS) -DENABLE_GUI=$(GUI) -DENABLE_SERIAL=$(SERIAL)
LDFLAGS = -lm -g
ASFLAGS = -mmcu=$(MCU) -Os -DF_CPU=$(OSC)

#check target microcontroller
ifeq ($(MCU),atmega32u4)
	OBJECTS = $(SRCOBJS)		
	PORT = /dev/ttyACM0
ifeq ($(SERIAL),1)
	INCLUDES	+= -I src/usb-cdc -I libs/lufa -I libs/lufa/LUFA/Drivers
	OBJECTS += $(LUFAOBJS)
	CFLAGS += -DF_USB=$(OSC) -DUSE_LUFA_CONFIG_HEADER
	CXXFLAGS += -DF_USB=$(OSC) -DUSE_LUFA_CONFIG_HEADER
endif
else ifeq ($(MCU),atmega328p)
	OBJECTS = $(SRCOBJS)
	PORT = /dev/ttyUSB0
else
	@echo Unsupported microcontroller $(MCU)
endif

#SERIAL requires LUFA on atmega32u4
ifeq ($(MCU),atmega32u4)

endif

#if SERIAL enabled
ifneq ($(SERIAL),0)
	INCLUDES += -I src/serial
	OBJECTS += $(SERIALOBJS)
endif	
#if GUI is enabled add U8G
ifneq ($(GUI),0)
	INCLUDES += -I src/gui -I libs/u8glib/csrc
	OBJECTS += $(GUIOBJS)
endif	
#check VFFS
ifeq ($(VFFS),1)
	OBJECTS += $(VFFSOBJS)
endif

all: $(TARGET).elf size hex

$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	${CXX} -c $(CXXFLAGS) $< -o $@

%.o: %c 
	${CC} -c $(CFLAGS) $< -o $@

%.o: %S 
	${CC} -c $(ASFLAGS) $< -o $@	

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
removetemp:
	find . -name "*.i" -type f -delete
	find . -name "*.ii" -type f -delete
#	find . -name "*.s" -type f -delete
