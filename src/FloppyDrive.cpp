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


#include "fddEMU.h"
#include "FloppyDrive.h"
#include "petitfs/pff.h"
#include "petitfs/diskio.h"
#include "avrFlux/avrFlux.h"
#include "UINotice.h" //msg.error
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h> // for strcat,strcpy,...
#include <stdlib.h> // for itoa

//Global variables
bool pinsInitialized = false;
static struct floppySectorHeader secHeader;
static struct floppySectorData secData;
volatile int8_t iTrack = 0;
class driveStatus ds;

class FloppyDrive drive[N_DRIVE]; //will be used as extern

//Interrupt routines
#if defined (__AVR_ATmega328P__)
ISR(INT0_vect) //int0 pin 2 of port D
#elif defined (__AVR_ATmega32U4__)
ISR(INT2_vect) //int2
#endif //(__AVR_ATmega32U4__)
{
	if (IS_STEP() ) //debounce
		(STEPDIR()) ? --iTrack : ++iTrack;
	ds.setTrackChanged();
}

void initFDDpins()
{
	//To emulate open collector outputs Output pins are set to LOW "0"
	//To write a "1" to output pin, respective pin is set to input
	//To write a "0" to output pin, respective pin is set to output
	pinsInitialized = false;
	cli(); //disable interrupts
#if defined (__AVR_ATmega328P__)
	//Setup Input and Output pins as Inputs
	DDRD &= 0b00000011; //D0 and D1 is RX & TX
	DDRB &= 0b11000000; //B6 & B7 is XTAL1 & XTAL2
	DDRC &= 0b11110000; //C7 is nil, C6 is RST, C4 & C5 is SDA & SCL
	//Assign Output pins LOW "0"
	PORTD &= ~(1 << PIN_INDEX);
	PORTB &= ~(1 << PIN_WRITEDATA);
	PORTC &= ~((1 << PIN_TRACK0)|(1 << PIN_WRITEPROT)|(1 << PIN_DSKCHANGE));
	//Assign Input pins HIGH "1" (Activate Pullups)
	PORTD |= (1 << PIN_MOTORA)|( 1 << PIN_SELECTA);
	PORTD |= (1 << PIN_STEP)|(1 << PIN_STEPDIR)|(1 << PIN_SIDE);
	PORTB |= (1 << PIN_READDATA);
	PORTC |= (1 << PIN_WRITEGATE);
	//Setup External Interrupt
	EICRA &=~((1 << ISC01)|(1 << ISC00)); //clear ISC00&ISC01 bits
	EICRA |= (1 << ISC01); //set ISC01 "falling edge"
	EIMSK |= (1 << INT0); //External Interrupt Mask Register enable INT0
#elif defined (__AVR_ATmega32U4__)
	//Setup Input and Output pins as Inputs
	DDRB |= (1 << PIN_WRITEDATA); //set WRITEDATA as OUTPUT (Not sure it is necessary but datasheet says so)
	DDRB &= ~((1 << PIN_MOTORA)|(1 << PIN_SELECTA)); //PB0 RXLED, PB1 SCK, PB2 MOSI, PB3 MISO, PB4 MOTORA, PB5 OCP1, PB6 SELECTA
	DDRC &= ~(1 << PIN_SIDE); //PC6 SIDE
	DDRD &= ~((1 << PIN_STEP)|(1 << PIN_STEPDIR)|(1 << PIN_READDATA)|(1 << PIN_INDEX)|(1 << PIN_WRITEPROT)); //PD0 SCL, PD1 SDA, PD5 TXLED, PD2 STEP, PD3 STEPDIR, PD4 ICP1, PD7 INDEX
	DDRE &= ~(1 << PIN_WRITEGATE); //PE6 WRITEGATE
	DDRF &= ~((1 << PIN_TRACK0)|(1 << PIN_DSKCHANGE)); //PF4 TRACK0, PF5 WRITEPROT, PF6 DISKCHANGE, PF7 SS
	//Assign Output pins LOW "0"
	PORTB &= ~(1 << PIN_WRITEDATA);
	PORTD &= ~(1 << PIN_INDEX)|(1 << PIN_WRITEPROT);
	PORTF &= ~((1 << PIN_TRACK0)|(1 << PIN_DSKCHANGE));
	//Assign Input pins HIGH "1" (Activate Pullups)
	PORTB |= (1 << PIN_MOTORA)|(1 << PIN_SELECTA); //PB0 RXLED, PB1 SCK, PB2 MOSI, PB3 MISO, PB4 MOTORA, PB5 OCP1, PB6 SELECTA
	PORTC |= (1 << PIN_SIDE); //PC6 SIDE
	PORTD |= (1 << PIN_STEP)|(1 << PIN_STEPDIR)|(1 << PIN_READDATA); //PD0 SCL, PD1 SDA, PD5 TXLED, PD2 STEP, PD3 STEPDIR, PD4 ICP1, PD7 INDEX
	PORTE |= (1 << PIN_WRITEGATE); //PE6 WRITEGATE
	//Setup External Interrupt
	EICRA &=~((1 << ISC21)|(1 << ISC20)); //clear ISC20&ISC21 bits
	EICRA |= (1 << ISC21); //set ISC21 "falling edge"
	EIMSK |= (1 << INT2); //External Interrupt Mask Register enable INT2
#endif //defined (__AVR_ATmega32U4__)
	pinsInitialized = true; //done
	sei(); //Turn interrupts on
}

void debugPrint_P(const char *debugStr)
{
#if DEBUG && ENABLE_SERIAL
	Serial.print_P(debugStr);
	Serial.write('\n');
#endif //DEBUG && ENABLE_SERIAL
}

void debugPrintSector(char charRW, uint8_t* buffer, int16_t len)
{
#if DEBUG && ENABLE_SERIAL
	uint8_t track  = secHeader.track;
	uint8_t head   = secHeader.side;	
	uint8_t sector = secHeader.sector;

	ds.isDrive0() ? Serial.write('A'):Serial.write('B');
	Serial.write(':');
	Serial.write(charRW);
	Serial.print(track);
	Serial.write('/');
	Serial.print(head);
	Serial.write('/');
	Serial.print(sector);
	Serial.write('\n');	

	if (charRW == 'W')
	{
		int8_t bytesPerLine = 16;

		for (int i = 0; i < len; i += bytesPerLine)
		{
			Serial.printHEX( (uint8_t) (i >> 8) );
			Serial.printHEX( (uint8_t) (i & 0xFF) );
			Serial.write(':');
			//write hex values
			for (uint8_t j = 0; j < bytesPerLine; j++)
			{
				Serial.write(' ');
				Serial.printHEX(buffer[i+j]);				
			}			
			//write printable chars
			Serial.write('\t');
			for (uint8_t j = 0; j < bytesPerLine; j++)
			{
				char ch = buffer[i+j];
				if (ch >= 32 && ch < 127) Serial.write(ch);
				else Serial.write('.');
			}
			Serial.write('\n');
		}
	}
#endif //DEBUG && ENABLE_SERIAL
}

uint8_t *prepSectorBuffer(uint8_t track, uint8_t head, uint8_t sector, uint8_t seclen)
{
	uint16_t crc;
	uint8_t *buffer = secData.buffer;
	
	//prepare header
	secHeader.id = 0xFE; // ID mark
	secHeader.track = track;
	secHeader.side = head;
	secHeader.sector = sector + 1;
	secHeader.length = seclen;
	crc = calc_crc((uint8_t*)&secHeader, 5);
	secHeader.crcHI = crc >> 8;
	secHeader.crcLO = crc & 0xFF;
	secHeader.gap = 0x4E;

	//prepare sector data	
	secData.id = 0xFB;
	secData.gap = 0x4E; 		
	switch (seclen)
	{
	case 2:
		crc = calc_crc(secData.buffer, 512+1);		
		secData.crcHI = crc >> 8;
		secData.crcLO = crc & 0xFF;
		break;
	case 1:	
		if ((sector & 1) == 0) //first 256b
		{
			crc = calc_crc(secData.buffer, 256+1);
			memcpy(secData.save, secData.data+256, 3); //save first 3 bytes of second half
			secData.crcHI = crc >> 8;
			secData.crcLO = crc & 0xFF;
			memcpy(secData.data+256, &secData.crcHI, 3); //copy crc + gap over
		}
		else //second 256b
		{			
			secData.save[2] = secData.data[256 -1]; //save last byte of first half
			secData.data[256 -1] = secData.id; //copy id over			
			crc = calc_crc(secData.buffer+256, 256+1);
			secData.crcHI = crc >> 8;
			secData.crcLO = crc & 0xFF;
			buffer = secData.buffer+256;	
		}
		break;
	default:
		debugPrint_P(err_secSize);		
	}
	debugPrintSector('R', NULL, 0);
	return buffer;
}

bool checkCRC() // crc check & restore sector to SD writable format
{
	uint16_t crc;
	int8_t status = true; // CRC error
	if (secHeader.length == 2)
	{
		crc = calc_crc(secData.buffer, 512+1);
	}
	else if (secHeader.length == 1)
	{
		if ((secHeader.sector &1) == 0) //first 256b
		{
			crc = calc_crc(secData.buffer, 256+1);
			secData.crcHI = secData.data[256]; //copy crc to it's proper place
			secData.crcLO = secData.data[257];			
			memcpy(secData.data+256, secData.save, 3); //restore first 3 bytes of second half		
		}
		else //second 256b
		{
			crc = calc_crc(secData.buffer+256, 256+1);
			secData.data[256 -1] = secData.save[2]; //restore last byte of first half
		}
	}
	if ( (secData.crcHI == (crc >> 8)) && (secData.crcLO == (crc & 0xFF)) ) status = false; //no error
#if DEBUG
	else 
	{
		Serial.print(F("CRC expected: "));
		Serial.printHEX(crc);
		Serial.print(F(" received: "));
		Serial.printHEX(secData.crcHI);
		Serial.printHEX(secData.crcLO);
		Serial.write('\n');
	}
#endif //DEBUG		
	uint8_t *pbuf = ( (secHeader.length == 1) && ((secHeader.sector&1)==0) ) ? secData.data+256:secData.data;
	debugPrintSector('W', pbuf, 128  << secHeader.length);
	return status;
}

FloppyDrive::FloppyDrive(void)
{
	if (!pinsInitialized) initFDDpins();
	bitLength = BIT_LENGTH_DD;	//To be more compatible: HD controllers support DD
	track = 0;
}

char *FloppyDrive::diskInfoStr()	//Generate disk CHS info string
{
	static char infostring[12]; //drive C/H/S info string
	char convbuf[4];

	if (fName[0] == 0)
	{
		strcpy_P(infostring, str_nodisk);
		return infostring;
	}
	infostring[0] = 'C';
	infostring[1] = 0;
	itoa(numTrack, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
	strcat(infostring, "H2S");
	itoa(numSec, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
	return infostring;
}

int FloppyDrive::getSectorData(int lba)
{
	int n = FR_DISK_ERR;

	if (isReady())
	{
		if (flags.seclen == 2) n = disk_read_sector(secData.data, startSector+lba);
		else if (flags.seclen == 1) n = disk_read_sector(secData.data, startSector+(lba >> 1));
		else debugPrint_P(err_secSize);
	}
	else if (isVirtual())
	{
	#if ENABLE_VFFS	
		n = vffs.readSector(secData.data, lba);
	#endif //ENABLE_VFFS
	}
	return n;
}

int FloppyDrive::setSectorData(int lba)
{
	int n = FR_DISK_ERR;
	
	if (isReady())
	{
		if (flags.seclen == 2) n = disk_write_sector(secData.data, startSector+lba);
		else if (flags.seclen == 1) n = disk_write_sector(secData.data, startSector+ (lba >> 1) );
		else debugPrint_P(err_secSize);
	}
	else if (isVirtual())
	{
	#if ENABLE_VFFS
		n = vffs.writeSector(secData.data, lba);
	#endif //ENABLE_VFFS	
	}
	return n;
}

bool FloppyDrive::load(char *r_file)
{
	return (FloppyDisk::load(r_file));
}

void FloppyDrive::eject()
{
	FloppyDisk::eject();
	track = 0;
}

void FloppyDrive::run()
{
	int16_t lba;
	uint8_t *wBuffer;
	uint8_t side = 0;
	uint8_t sector = 0;	//sectors in this loop are "0" based

	if (isChanged())
	{
		SET_DSKCHANGE_LOW();
		if (isReady() || isVirtual()) clrChanged();//if a disk is loaded clear diskChange flag
	}
	(isReadonly()) ? SET_WRITEPROT_LOW() : SET_WRITEPROT_HIGH();  //check readonly
	fdcWriteMode();	
	while(!ds.isDrvChanged())
	{
	#if ENABLE_WDT
		wdt_reset();
	#endif  //ENABLE_WDT
	#if defined (__AVR_ATmega32U4__) && ENABLE_SERIAL
		Serial.rcvRdy(); //service usb
	#endif //defined (__AVR_ATmega32U4__) && ENABLE_SERIAL	
		if (ds.isTrackChanged()) //if track changed
		{
			ds.clrTrackChanged();
			track += iTrack;	//add iTrack to current track
			iTrack = 0;				//reset iTrack
			if (track < 0) track=0; //Check if track valid
			else if (track >= numTrack) track = numTrack-1;
			(track == 0) ? SET_TRACK0_LOW() : SET_TRACK0_HIGH();
			isReady() || isVirtual() ? SET_DSKCHANGE_HIGH() : SET_DSKCHANGE_LOW(); //disk present ?
		}
		side = (SIDE()) ? 0:1; //check side
		//start sector	
		lba=(track*2+side)*numSec+sector;//LBA = (C × HPC + H) × SPT + (S − 1)			
		getSectorData(lba); //get sector from SD
		wBuffer = prepSectorBuffer(track, side, sector, flags.seclen);
		fdcWriteHeader(bitLength, secHeader.buffer);
		if ( fdcWriteData(bitLength, wBuffer, (128 << flags.seclen)+4) ) //if WRITE_GATE asserted
		{				
			fdcReadMode();
			uint8_t res = fdcReadData(bitLength, wBuffer, (128 << flags.seclen)+3);
			fdcWriteMode();
			if (res ==  0)
			{
				if (!checkCRC()) setSectorData(lba);
			}					
			else debugPrint_P(err_readFDC); //couldnt read full sector		
			while (IS_WRITE());//wait for WRITE_GATE to deassert
		}	
		else fdcWriteGap(bitLength, 54);
		sector++; //next sector
		if (sector >= numSec) sector = 0;
	}//selected
	SET_DSKCHANGE_HIGH();
	SET_WRITEPROT_HIGH();
	SET_TRACK0_HIGH();
	SET_INDEX_HIGH();
}
