// -----------------------------------------------------------------------------
// This file is part of fddEMU "Floppy Disk Drive Emulator"
// Copyright (C) 2021 Acemi Elektronikci
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
// -----------------------------------------------------------------------------

#ifndef FLOPPYDRIVE_H
#define FLOPPYDRIVE_H

#include "FloppyDisk.h"
#include <stdint.h>

#if defined (__AVR_ATmega328P__)
// -------------------------------  Pin assignments for Arduino UNO/Nano/Pro Mini (Atmega328p)  ------------------------------
//PORTD
#define PIN_STEP       2
#define PIN_STEPDIR    3
#define PIN_MOTORA     4
#define PIN_SELECTA    5
#define PIN_SIDE       6
#define PIN_INDEX      7

//PORTB
#define PIN_READDATA   0
#define PIN_WRITEDATA  1
//rest of PORTB pins are SPI

//PORTC
#define PIN_WRITEGATE 0
#define PIN_TRACK0    1
#define PIN_WRITEPROT 2
#define PIN_DSKCHANGE 3

//Output commands
#define SET_INDEX_LOW()    (DDRD |= (1 << PIN_INDEX))
#define SET_INDEX_HIGH()   (DDRD &= ~(1 << PIN_INDEX))
#define SET_TRACK0_LOW()    (DDRC |= (1 << PIN_TRACK0))
#define SET_TRACK0_HIGH()   (DDRC &= ~(1 << PIN_TRACK0))
#define SET_DSKCHANGE_LOW()    (DDRC |= (1 << PIN_DSKCHANGE))
#define SET_DSKCHANGE_HIGH()   (DDRC &= ~(1 << PIN_DSKCHANGE))
#define SET_WRITEPROT_LOW()    (DDRC |= (1 << PIN_WRITEPROT))
#define SET_WRITEPROT_HIGH()   (DDRC &= ~(1 << PIN_WRITEPROT))

//Inputs
#define IS_WRITE()          ( !(PINC & (1 << PIN_WRITEGATE)) )
#define SIDE()              (PIND & (1 << PIN_SIDE))
#define STEPDIR()           (PIND & (1 << PIN_STEPDIR))
#define IS_STEP()           ( !(PIND & (1 << PIN_STEP)) )
#define IS_SELECTA()        ( !(PIND & (1 << PIN_SELECTA)) )
#define IS_MOTORA()         ( !(PIND & (1 << PIN_MOTORA)) )
#define IS_SELECTB()        ( !(PIND & (1 << PIN_MOTORA)) )
// -------------------------------  Pin assignments for Arduino UNO/Nano/Pro Mini (Atmega328p)  ------------------------------
#elif defined (__AVR_ATmega32U4__)
// -------------------------------  Pin assignments for Arduino (Atmega32U4)  ------------------------------
//PORTD PD0(SCL) and PD1(SDA) I2C pins (D2 and D3 on pro micro)
//PORTB PB0(SS), PB1(SCK), PB2(MISO), PB3(MOSI) SPI pins (D17, D15, D14, D16)  D17->TX led

//Unchangeable pins ICP&OCP for timer1
#define PIN_READDATA    4   //PD4 must be pin4 (ICP for timer1)
#define PIN_WRITEDATA   5   //PB5 must be pin9 (OCP for timer1)
//Pin Change Interrupt pins
#define PIN_MOTORA      4   //PB4-PCINT4-pin8
#define PIN_SELECTA     6   //PB6-PCINT6-pin10
//External Interrupt pins
#define PIN_STEP        2   //PD2-INT2-RX1 will use external interrupt 2
//Changeable pins
#define PIN_STEPDIR     3   //PD3-INT3-TX1
#define PIN_SIDE        6   //PC6-pin5
#define PIN_INDEX       7   //PD7-pin6
#define PIN_WRITEGATE   6   //PE6-INT6-pin7
#define PIN_TRACK0      4   //PF4-pinA3
#define PIN_WRITEPROT   5   //PD5-pin24-TXLED
#define PIN_DSKCHANGE   6   //PF6-pinA1
//SS pin for SD card is PF7-pinA0 for atmega32u4 (Arduino pro micro)
//SS pin is set in "pffArduino.h" to change the pin
//also changing the macros in "pffArduino.h" is required.

//Output commands
#define SET_INDEX_LOW()    (DDRD |= (1 << PIN_INDEX))
#define SET_INDEX_HIGH()   (DDRD &= ~(1 << PIN_INDEX))
#define SET_TRACK0_LOW()    (DDRF |= (1 << PIN_TRACK0))
#define SET_TRACK0_HIGH()   (DDRF &= ~(1 << PIN_TRACK0))
#define SET_DSKCHANGE_LOW()    (DDRF |= (1 << PIN_DSKCHANGE))
#define SET_DSKCHANGE_HIGH()   (DDRF &= ~(1 << PIN_DSKCHANGE))
#define SET_WRITEPROT_LOW()    (DDRD |= (1 << PIN_WRITEPROT))
#define SET_WRITEPROT_HIGH()   (DDRD &= ~(1 << PIN_WRITEPROT))

//Inputs
#define IS_WRITE()          ( !(PINE & (1 << PIN_WRITEGATE)) )
#define SIDE()              (PINC & (1 << PIN_SIDE))
#define STEPDIR()           (PIND & (1 << PIN_STEPDIR))
#define IS_STEP()           ( !(PIND & (1 << PIN_STEP)) )
#define IS_SELECTA()        ( !(PINB & (1 << PIN_SELECTA)) )
#define IS_MOTORA()         ( !(PINB & (1 << PIN_MOTORA)) )
#define IS_SELECTB()        ( !(PINB & (1 << PIN_MOTORA)) )

// -------------------------------  Pin assignments for Arduino (Atmega32U4)  ------------------------------
#endif //defined (__AVR_ATmega32U4__)

//define bits of iFlags
#define BIT_TRACKCHANGE 7
#define BIT_DRIVE0      0
#define BIT_DRIVE1      1

#define F_TRACKCHANGED  (1 << BIT_TRACKCHANGE)
#define DRIVE0 (1 << BIT_DRIVE0)
#define DRIVE1 (1 << BIT_DRIVE1)

#define SET_TRACKCHANGED()  (iFlags |= F_TRACKCHANGED)
#define CLR_TRACKCHANGED()  (iFlags &= ~F_TRACKCHANGED)
#define SEL_DRIVE0()  (iFlags |= DRIVE0)
#define SEL_DRIVE1()  (iFlags |= DRIVE1)
#define CLR_DRVSEL() (iFlags &= ~(DRIVE0|DRIVE1) )

#define IS_TRACKCHANGED() (iFlags & (1 << BIT_TRACKCHANGE))
#define IS_DRIVE0() (iFlags & DRIVE0)
#define IS_DRIVE1() (iFlags & DRIVE1)
#define GET_DRVSEL() (iFlags & (DRIVE0|DRIVE1) )

struct __attribute__((__packed__)) floppySector
{
	struct __attribute__((__packed__)) sectorHeader
	{
		uint8_t	id;	// sector header id: 0xFE
		uint8_t track; // current cylinder
		uint8_t	side; // current head
		uint8_t	sector;	// current sector: starts at 1
		uint8_t	length; // 0: 128b, 1: 256b, 2: 512b, 3: 1024b
		uint8_t crcHI; // crc / 256
		uint8_t	crcLO; // crc & 255
		uint8_t gap; // first byte of post-header gap: 0x4E
	} header;
		uint8_t	id; // sector data id: 0xFB
		uint8_t	data[512]; //sector data 512 bytes
		uint8_t crcHI; // crc / 256
		uint8_t	crcLO; // crc & 255
		uint8_t gap; // first byte of post-data gap: 0x4E
		uint8_t extra[9]; //extra for half_sector data
};

class FloppyDrive : public FloppyDisk
{
	private:
		int track;
		uint8_t side;
		uint8_t sector;
		int getSectorData(int lba);
		int setSectorData(int lba);

	public:
		FloppyDrive();
		char *diskInfoStr();
		bool load(char *);
		void eject();
		void run();
};

#if ENABLE_DRIVE_B
		#define N_DRIVE 2
#else
		#define N_DRIVE 1
#endif //ENABLE_DRIVE_B

extern volatile uint8_t iFlags; //flags set by interrupt
extern class FloppyDrive drive[N_DRIVE];

#endif //PLOPPYDRIVE_H
