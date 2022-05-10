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

#ifndef FLOPPYDISK_H
#define FLOPPYDISK_H

#include <stdint.h>
#include <stdbool.h>

//define BIT LENGTH
#define BIT_LENGTH_HD	16
#define BIT_LENGTH_DD	32  

struct __attribute__((__packed__)) fd_flags {
	unsigned int changed : 1;
	unsigned int ready : 1;
	unsigned int readonly : 1;
	unsigned int vdisk : 1;
	unsigned int seclen : 2; //sector lenth: 1:256, 2:512
	unsigned int empty : 2;
};

class FloppyDisk{
	protected:
		struct fd_flags flags;
		uint8_t sectorLen;
		uint8_t numTrack;   //number of tracks
		uint8_t numSec;     //sectors per track
		uint8_t bitLength;  //16 for HD, 32 for DD
		long startSector;
		FloppyDisk();
		bool load(char *);
		void eject();
		bool isReady(void) {return (flags.ready);}
		bool isReadonly(void) {return (flags.readonly);}
		bool isChanged(void) {return (flags.changed);}
		bool isVirtual(void) {return (flags.vdisk);}
		void clrChanged(void) {flags.changed = 0;}

	public:
		char fName[13];
		void loadVirtualDisk();
};

void errorMessage(const char *errMessageProgmem);
char *diskinfo(uint8_t r_drive);    //Generate disk CHS info string

#endif  //FLOPPYDISK_H
