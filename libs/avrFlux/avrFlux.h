// -----------------------------------------------------------------------------
// Modified for use with fddEMU "Floppy Disk Drive Emulator" 
// 2021 Acemi Elektronikci
// -----------------------------------------------------------------------------
// 3.5"/5.25" DD/HD Disk controller for Arduino
// Copyright (C) 2021 David Hansel
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

#ifndef AVRFLUX_H
#define AVRFLUX_H

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#include <stdint.h>
#include <stddef.h>


#define bit(X)  (1<<X)

// return status for readSector/writeSector and formatDisk functions
#define S_OK         0  // no error
#define S_NOTINIT    1  // ArduinoFDC.begin() was not called
#define S_NOTREADY   2  // Drive is not ready (no disk or power)
#define S_NOSYNC     3  // No sync marks found


uint16_t calc_crc(uint8_t *buf, int n);
//uint8_t getBitLength(enum DriveType driveType);
void setup_timer1_for_write();
void setup_timer1_for_read();
void genSectorID(uint8_t track, uint8_t side, uint8_t sector, uint8_t sector_length);
uint8_t read_data(uint8_t bitlen, uint8_t *buffer, unsigned int n);
void write_data(uint8_t bitlen, uint8_t *buffer, unsigned int n);
uint8_t track_start(uint8_t bitlen);
uint8_t sector_start(uint8_t bitlen);

#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  //AVR_FLUX_H
