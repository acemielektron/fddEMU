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

uint16_t calc_crc(uint8_t *buf, int n);
void fdcWriteMode();
void fdcReadMode();
uint8_t fdcReadData(uint8_t bitlen, uint8_t *buffer, unsigned int n);
uint8_t fdcWriteData(uint8_t bitlen, uint8_t *buffer, unsigned int n);
void fdcWriteHeader(uint8_t bitlen, uint8_t *buffer);
void fdcWriteGap(uint8_t bitlen, uint8_t gaplen);

#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  //AVR_FLUX_H
