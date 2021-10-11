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

#ifndef FDDPINS_H
#define FDDPINS_H

// -------------------------------  Pin assignments for Arduino UNO/Nano/Pro Mini (Atmega328p)  ------------------------------

//PORTD
#define PIN_STEP       2  // can be changed to different pin
#define PIN_STEPDIR    3  // can be changed to different pin
#define PIN_MOTORA     4  // can be changed to different pin
#define PIN_SELECTA    5  // can be changed to different pin
#define PIN_SIDE       6  // can be changed to different pin
#define PIN_INDEX      7  // accesses via IDXPORT/IDXBIT #defines below

//PORTB
#define PIN_READDATA   0  // must be pin 8 (ICP for timer1)
#define PIN_WRITEDATA  1  // must be pin 9 (OCP for timer1)
//rest of PORTB pins are SPI

//PORTC
#define PIN_WRITEGATE 0
#define PIN_TRACK0    1  // can be changed to different pin
#define PIN_WRITEPROT 2  // can be changed to different pin or commented out
#define PIN_DSKCHANGE 3  // can be changed to different pin or commented out

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

#endif  //FDDPINS_H
