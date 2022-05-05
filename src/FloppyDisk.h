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

//define FLAGS
#define FD_CHANGED  (1 << 0)
#define FD_READY    (1 << 1)
#define FD_READONLY (1 << 2)
#define FD_VIRTUAL  (1 << 3)    
#define FD_HALFSECTOR (1 << 4) //256 byte sectors
//define MACRO
#define IS_HALFSECTOR() flags & FD_HALFSECTOR

class FloppyDisk{
	protected:
		uint8_t flags;
		uint8_t numTrack;   //number of tracks
		uint8_t numSec;     //sectors per track
		uint8_t bitLength;  //16 for HD, 32 for DD
		long startSector;
		FloppyDisk();
		bool load(char *);
		void eject();
		bool isReady(void) {return (flags & FD_READY);}
		bool isReadonly(void) {return (flags & FD_READONLY);}
		bool isChanged(void) {return (flags & FD_CHANGED);}
		bool isVirtual(void) {return (flags & FD_VIRTUAL);}
		void clrChanged(void) {flags &= ~FD_CHANGED;}

	public:
		char fName[13];
		void loadVirtualDisk();
};

void errorMessage(const char *errMessageProgmem);
char *diskinfo(uint8_t r_drive);    //Generate disk CHS info string

#endif  //FLOPPYDISK_H
