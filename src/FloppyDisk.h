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

//define FLAGS
#define FD_CHANGED   (1 << 0)
#define FD_READY     (1 << 1)
#define FD_READONLY  (1 << 2)

class FloppyDisk{
    public:
    uint8_t flags;
    uint8_t numTrack;   //number of tracks
    uint8_t numSec;     //sectors per track
    long startSector;
    char fName[13];    
    FloppyDisk();
    bool load(char *);
    void eject();
    bool isReady(void) {return (flags & FD_READY);}
    bool isReadonly(void) {return (flags & FD_READONLY);}
    bool isChanged(void) {return (flags & FD_CHANGED);}
    void clrChanged(void) {flags &= ~FD_CHANGED;}
};

#endif  //FLOPPYDISK_H
