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

#include "FloppyDrive.h"
#include "FDDcommon.h"
#include "pff.h"
#include <avr/io.h>
#include <avr/interrupt.h>


bool pinsInitialized = false;

void initFDDpins()
{
  pinsInitialized = false;
  //Set ports as input
  DDRD &= 0b00000011; //D0 and D1 is RX & TX
  DDRB &= 0b11000000; //B6 & B7 is XTAL1 & XTAL2
  DDRC &= 0b11110000; //C7 is nil, C6 is RST, C4 & C5 is SDA & SCL
  //Outputs
  PORTD &= ~bit(PIN_INDEX);//LOW "0"
  PORTB &= ~bit(PIN_WRITEDATA);//LOW "0"
  PORTC &= ~(bit(PIN_TRACK0)|bit(PIN_WRITEPROT)|bit(PIN_DSKCHANGE));//LOW "0"
  //inputs
  PORTD |= bit(PIN_MOTORA)|bit(PIN_SELECTA); //Enable Pull Up  
  PORTD |= bit(PIN_STEP)|bit(PIN_STEPDIR)|bit(PIN_SIDE); //Enable Pull Up
  PORTB |= bit(PIN_READDATA); //pullup "1"  
  PORTC |= bit(PIN_WRITEGATE); //pullup
  
  //Setup Interrupts
  EICRA &=~(bit(ISC01)|bit(ISC00)); //clear ISC00&ISC01 bits
  EICRA |= bit(ISC01); //set ISC01 "falling edge"
  EIMSK |= bit(INT0); //External Interrupt Mask Register enable INT0  
  
  PCMSK2 = bit(PIN_SELECTA)| bit(PIN_MOTORA); // Pin Change Mask Register 2 enable SELECTA&MOTORA
  PCICR |= bit(PCIE2); // Pin Change Interrupt Control Register enable port D    
  pinsInitialized = true; //done
  sei(); //Turn interrupts on
}

void FloppyDrive::ejectDisk()
{
  DiskFile::ejectDisk();
  SET_DSKCHANGE_LOW();
}

int FloppyDrive::loadDisk(char *filename)
{
  int res = DiskFile::loadDisk(filename);
  if (res == FR_OK)
  {
    (fAttr & AM_RDO) ? SET_WRITEPROT_LOW() : SET_WRITEPROT_HIGH();  //check readonly
    SET_DSKCHANGE_HIGH();    
  }
  else
    ejectDisk();
  return (res);
}

void FloppyDrive::init() 
{ 
  bitLength=16; //  bit length for 3.5" HD floppy is 16
}

FloppyDrive::FloppyDrive(void)
{  
  if (!pinsInitialized) initFDDpins();
  init();
}
