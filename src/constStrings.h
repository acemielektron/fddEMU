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
// ------------------------------------------------------------------------------

#ifndef CONSTSTRINGS_H
#define CONSTSTRINGS_H

#include <avr/pgmspace.h>

#define errorMessage(x) { \
	disp.showNoticeP(errHDR, x); \
	Serial.print_P(errHDR); \
	Serial.print_P(str_colon); \
	Serial.print_P(x); }


const char str_colon[]		PROGMEM = ": ";
const char str_nodisk[]		PROGMEM = "No disk";
const char errHDR[] 		PROGMEM = "ERROR";
const char err_initSD[]		PROGMEM = "Initialize SD card";
const char err_fopen[] 		PROGMEM = "File open failed";
const char err_fnopen[]		PROGMEM = "File not opened";
const char err_noncontig[]	PROGMEM = "Non-contiguous file";
const char err_diskread[]	PROGMEM = "Read SD failed";
const char err_diskwrite[]	PROGMEM = "Write SD failed";
const char err_invboot[]	PROGMEM = "Invalid boot record";
const char err_notfat12[]	PROGMEM = "Not FAT12 image";
const char err_geometry[] 	PROGMEM = "Incompat geometry";
const char err_test[]		PROGMEM = "Testing 1, 2, 3";


#endif //CONSTSTRINGS_H
