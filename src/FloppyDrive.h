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
#define PIN_STEP      2
#define REG_STEP			PIND
#define PIN_STEPDIR   3
#define REG_STEPDIR		PIND
#define PIN_MOTORA    4
#define REG_MOTORA		PIND
#define PIN_SELECTA   5
#define REG_SELECTA		PIND
#define PIN_SIDE      6
#define REG_SIDE			PIND
#define PIN_INDEX     7
#define DDR_INDEX			DDRD

//PORTB
#define PIN_READDATA  0
#define DDR_READDATA	DDRB
#define PIN_WRITEDATA 1
#define DDR_WRITEDATA DDRB
//rest of PORTB pins are SPI

//PORTC
#define PIN_WRITEGATE 0
#define REG_WRITEGATE PINC
#define PIN_TRACK0    1
#define DDR_TRACK0 		DDRC
#define PIN_WRITEPROT 2
#define DDR_WRITEPROT DDRC
#define PIN_DSKCHANGE 3
#define DDR_DSKCHANGE DDRC


// -------------------------------  Pin assignments for Arduino UNO/Nano/Pro Mini (Atmega328p)  ------------------------------
#elif defined (__AVR_ATmega32U4__)
// -------------------------------  Pin assignments for Arduino (Atmega32U4)  ------------------------------
//PORTD PD0(SCL) and PD1(SDA) I2C pins (D2 and D3 on pro micro)
//PORTB PB0(SS), PB1(SCK), PB2(MISO), PB3(MOSI) SPI pins (D17, D15, D14, D16)  D17->TX led

//Unchangeable pins ICP&OCP for timer1
#define PIN_READDATA    4   //PD4 must be pin4 (ICP for timer1)
#define DDR_READDATA		DDRD
#define PIN_WRITEDATA   5   //PB5 must be pin9 (OCP for timer1)
#define DDR_WRITEDATA		DDRB
//Pin Change Interrupt pins
#define PIN_MOTORA      4   //PB4-PCINT4-pin8
#define REG_MOTORA			PINB
#define PIN_SELECTA     6   //PB6-PCINT6-pin10
#define REG_SELECTA			PINB
//External Interrupt pins
#define PIN_STEP        2   //PD2-INT2-RX1 will use external interrupt 2
#define REG_STEP				PIND
//Changeable pins
#define PIN_STEPDIR     3   //PD3-INT3-TX1
#define REG_STEPDIR			PIND
#define PIN_SIDE        6   //PC6-pin5
#define REG_SIDE				PINC
#define PIN_INDEX       7   //PD7-pin6
#define DDR_INDEX				DDRD
#define PIN_WRITEGATE   6   //PE6-INT6-pin7
#define REG_WRITEGATE		PINE
#define PIN_TRACK0      4   //PF4-pinA3
#define DDR_TRACK0			DDRF
#define PIN_WRITEPROT   5   //PD5-pin24-TXLED
#define DDR_WRITEPROT		DDRD
#define PIN_DSKCHANGE   6   //PF6-pinA1
#define DDR_DSKCHANGE		DDRF
//SS pin for SD card is PF7-pinA0 for atmega32u4 (Arduino pro micro)
//SS pin is set in "pffArduino.h" to change the pin
//also changing the macros in "pffArduino.h" is required.

// -------------------------------  Pin assignments for Arduino (Atmega32U4)  ------------------------------
#endif //defined (__AVR_ATmega32U4__)

//Output commands
#define SET_INDEX_LOW()    (DDR_INDEX |= (1 << PIN_INDEX))
#define SET_INDEX_HIGH()   (DDR_INDEX &= ~(1 << PIN_INDEX))
#define SET_TRACK0_LOW()    (DDR_TRACK0 |= (1 << PIN_TRACK0))
#define SET_TRACK0_HIGH()   (DDR_TRACK0 &= ~(1 << PIN_TRACK0))
#define SET_DSKCHANGE_LOW()    (DDR_DSKCHANGE |= (1 << PIN_DSKCHANGE))
#define SET_DSKCHANGE_HIGH()   (DDR_DSKCHANGE &= ~(1 << PIN_DSKCHANGE))
#define SET_WRITEPROT_LOW()    (DDR_WRITEPROT |= (1 << PIN_WRITEPROT))
#define SET_WRITEPROT_HIGH()   (DDR_WRITEPROT &= ~(1 << PIN_WRITEPROT))

//Inputs
#define IS_WRITE()          ( !(REG_WRITEGATE & (1 << PIN_WRITEGATE)) )
#define SIDE()              (REG_SIDE & (1 << PIN_SIDE))
#define STEPDIR()           (REG_STEPDIR & (1 << PIN_STEPDIR))
#define IS_STEP()           ( !(REG_STEP & (1 << PIN_STEP)) )
#define IS_SELECTA()        ( !(REG_SELECTA & (1 << PIN_SELECTA)) )
#define IS_MOTORA()         ( !(REG_MOTORA & (1 << PIN_MOTORA)) )
#define IS_SELECTB()        IS_MOTORA()
//Two drive mode requires SELECT and MOTOR pins combined trough an AND gate
//if two drive mode is enabled SELECTA pin is used for combined SELECTA & MOTORA
//and MOTORA pin is used for combined SELECTB & MOTORB

struct __attribute__((__packed__)) driveControlFlags
{
	unsigned int drive0 : 1;	
	unsigned int drive1 : 1;
	unsigned int driveChanged : 1;
	unsigned int trackChanged : 1;
	unsigned int empty : 4;
};

class driveStatus
{
	private:
		volatile struct driveControlFlags f;
	public:
		void setTrackChanged() {f.trackChanged = 1;}
		void clrTrackChanged() {f.trackChanged = 0;}
		int isTrackChanged() {return f.trackChanged;}		
		void chkDrvChanged() { if (f.driveChanged) return;
			f.driveChanged |= (isDrive1() != f.drive1 || isDrive0() != f.drive0) ? 1:0;
			f.drive0 = isDrive0(); f.drive1 = isDrive1();}
		int isDrvChanged() {chkDrvChanged(); return f.driveChanged;}
		void clrDrvChanged() {f.driveChanged = 0;}	
		int getDriveSel () {chkDrvChanged(); return isDrive1() << 1|isDrive0();}
	#if ENABLE_DRIVE_B // Dual drive mode		
		int isDrive0() {return IS_SELECTA() ? 1:0;}
		int isDrive1() {return IS_SELECTB() ? 1:0;}
	#else // Single drive mode
		int isDrive0() {return IS_SELECTA() && IS_MOTORA() ? 1:0;}
		int isDrive1() {return 0;}
	#endif // ENABLE_DRIVE_B 
};

struct __attribute__((__packed__)) floppySectorHeader
{
	union 
	{
		uint8_t buffer[8];
		struct __attribute__((__packed__)) 
		{
			uint8_t	id;	// sector header id: 0xFE
			uint8_t track; // current cylinder
			uint8_t	side; // current head
			uint8_t	sector;	// current sector: starts at 1
			uint8_t	length; // 0: 128b, 1: 256b, 2: 512b, 3: 1024b
			uint8_t crcHI; // crc / 256
			uint8_t	crcLO; // crc & 255
			uint8_t gap; // first byte of post-header gap: 0x4E

		};
	};
	
};

struct __attribute__((__packed__)) floppySectorData
{
	union 
	{
		uint8_t buffer[1+512+3+3];
		struct __attribute__((__packed__))
		{
			uint8_t	id; // sector data id: 0xFB
			uint8_t	data[512]; //sector data 512 bytes
			uint8_t crcHI; // crc / 256
			uint8_t	crcLO; // crc & 255
			uint8_t gap; // first byte of post-data gap: 0x4E
			uint8_t save[3]; //storage for saving half_sector data			
		};		
	};	
};

class FloppyDrive : public FloppyDisk
{
	private:
		int track;
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
extern class driveStatus ds;

#endif //PLOPPYDRIVE_H
