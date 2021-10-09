TARGET = fddEMU
MCU = atmega328p
OSC = 16000000UL

CC = avr-gcc
CXX = avr-g++

INCLUDES	= -I /usr/avr/include  -I libs/u8glib/csrc -I libs/petitfs -I .
ENABLE_DUAL_DRIVE = -DENABLE_DRIVE_B
ENABLE_DEBUG = -DDEBUG
FLIP_SCREEN = -DFLIP_SCREEN

CFLAGS = -Os -mmcu=$(MCU) -DF_CPU=$(OSC) -Wall $(INCLUDES)
CFLAGS += $(ENABLE_DUAL_DRIVE) $(ENABLE_DEBUG) $(FLIP_SCREEN)
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
CXXFLAGS= -Os -mmcu=$(MCU) -DF_CPU=$(OSC) -Wall $(INCLUDES)
CXXFLAGS += $(ENABLE_DUAL_DRIVE) $(ENABLE_DEBUG) $(FLIP_SCREEN)
CXXFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
LINKERFLAG = -lm

# Files
EXT_C   = c
EXT_C++ = cpp
EXT_ASM = S

OBJECTS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard src/*.$(EXT_C))) \
	$(patsubst %.$(EXT_C++),%.o,$(wildcard src/*.$(EXT_C++))) \
	$(patsubst %.$(EXT_ASM),%.o,$(wildcard src/*.$(EXT_ASM)))\
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
	avrdude -p m328p -c arduino -P /dev/ttyUSB0 -U flash:w:$(TARGET).hex
clean :
	rm $(TARGET).elf $(OBJECTS)
