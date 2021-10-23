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
#include "pff.h"
#include "diskio.h"
#include "simpleUART.h"
#include "avrFlux.h"
#include "constStrings.h"
#include "FDisplay.h"
#include "VirtualFloppyFS.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

//Global variables
bool pinsInitialized = false;
static uint8_t dataBuffer[516];
volatile int iTrack = 0;
volatile uint8_t iFlags = 0;

class FloppyDrive drive[N_DRIVE]; //will be used as extern

//Interrupt routines
ISR(INT0_vect) //int0 pin 2 of port D
{
  if (IS_STEP() ) //debounce
    iTrack = (STEPDIR()) ? --iTrack : ++iTrack;
  SET_TRACKCHANGED();  
}

//Two drive mode requires SELECT and MOTOR pins combined trough an OR gate
//if two drive mode is enabled SELECTA pin is used for combined SELECTA & MOTORA
//and MOTORA pin is used for combined SELECTB & MOTORB
ISR(PCINT2_vect) //pin change interrupt of port D
{
#if ENABLE_DRIVE_B
  if ( IS_SELECTA() ) SET_DRIVE0(); //drive A is selected 
  else if ( IS_SELECTB() ) SET_DRIVE1(); //drive B is selected   
#else //Drive B not enabled
  if ( IS_SELECTA() && IS_MOTORA() ) SET_DRIVE0(); //driveA is selected 
#endif  //ENABLE_DRIVE_B
  else CLR_DRVSEL();
}

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

void FloppyDrive::init() 
{ 
  if (!pinsInitialized) initFDDpins();
  bitLength=16; //  bit length for 3.5" HD floppy is 16
}

FloppyDrive::FloppyDrive(void)
{    
  init();
}



int FloppyDrive::getSectorData(int lba)
{
  int n = FR_DISK_ERR;
  uint8_t *pbuf=dataBuffer+1;  
  
  #if DEBUG
  uint8_t head   = 0;
  uint8_t track  = lba / (numSec*2);
  uint8_t sector = lba % (numSec*2);
  if( sector >= numSec ) { head = 1; sector -= numSec; }

  Serial.write('R');
  Serial.printDEC(track);
  Serial.write('/');
  Serial.printDEC(head);
  Serial.write('/');
  Serial.printDEC(sector+1);
  Serial.write('\n');
  #endif //DEBUG
  
  if (isReady())
  {
    n = disk_read_sector(pbuf, startSector+lba);
    if (n) errorMessage(err_diskread);
  }
#if VFFS_ENABLED  
  else vffs.readSector(pbuf, lba);
#endif //VFFS_ENABLED    
  return n;
}

int FloppyDrive::setSectorData(int lba)
{
  int n = FR_DISK_ERR;
  uint8_t *pbuf=dataBuffer+1;  
  
  if (isReady())
  {
    n = disk_write_sector(pbuf, startSector+lba);
    if (n) errorMessage(err_diskwrite);
  }
#if VFFS_ENABLED    
  else vffs.writeSector(pbuf, lba);
#endif //VFFS_ENABLED    
  
  #if DEBUG
  uint8_t head   = 0;
  uint8_t track  = lba / (numSec*2);
  uint8_t sector = lba % (numSec*2);
  if( sector >= numSec ) { head = 1; sector -= numSec; }

  Serial.write('W');
  Serial.printDEC(track);
  Serial.write('/');
  Serial.printDEC(head);
  Serial.write('/');
  Serial.printDEC(sector+1);
  Serial.write('\n');

  for (int i=0; i<512; i++)
  {    
    Serial.printHEX(pbuf[i]);
    Serial.write(' ');
    if ( (i&0xf) ==0xf ) Serial.write('\n');
  }
  #endif //DEBUG
  return n;
}

bool FloppyDrive::load(char *r_file) 
{
  if (FloppyDisk::load(r_file)) 
    (numSec > 9) ? bitLength = 16 : bitLength = 32; //DD or HD
  else 
    return false;  
  return true;  
}

void FloppyDrive::run()
{
  static int track=0;
  static uint8_t side=0;
  static uint8_t sector=0;
  static int lba;

  if (isChanged()) 
  {
    SET_DSKCHANGE_LOW();
  #if VFFS_ENABLED  
    clrChanged(); //if no disk is present virtual disk is inserted
  #else  
    if (isReady()) clrChanged();//if a disk is loaded clear diskChange flag    
  #endif //VFFS_ENABLED  
  }
  (isReadonly()) ? SET_WRITEPROT_LOW() : SET_WRITEPROT_HIGH();  //check readonly  
  setup_timer1_for_write(); 
  while(GET_DRVSEL()) //PCINT for SELECTA and MOTORA
  {    
    //Start track with index signal
    SET_INDEX_LOW();
    track_start(bitLength);       
    SET_INDEX_HIGH();
    
    for (sector=0; (sector < numSec) && GET_DRVSEL(); sector++)
    {        
    #if WDT_ENABLED                                        
      wdt_reset();
    #endif  //WDT_ENABLED  
      if (!GET_DRVSEL()) break; //if drive unselected exit loop      
      side = (SIDE()) ? 0:1; //check side
      if (IS_TRACKCHANGED()) //if track changed
      {
        CLR_TRACKCHANGED();             
        track = iTrack;
        if (track < 0) track=0; //Check if track valid
        else if (track >= numTrack) track = numTrack-1;          
        (track == 0) ? SET_TRACK0_LOW() : SET_TRACK0_HIGH(); 
        iTrack = track;
      #if VFFS_ENABLED
        SET_DSKCHANGE_HIGH();
      #else          
        (isReady()) ? SET_DSKCHANGE_HIGH() : SET_DSKCHANGE_LOW(); //disk present ?
      #endif //VFFS_ENABLED            
      }
      //start sector
      lba=(track*2+side)*18+sector;//LBA = (C × HPC + H) × SPT + (S − 1)
      getSectorData(lba); //get sector from SD      
      setup_timer1_for_write();
      genSectorID((uint8_t)track,side,sector);
      sector_start(bitLength);          
      if (track != iTrack) continue; //check track change
      //check WriteGate
      for (int i=0; i<20; i++) //wait 20X cycles for WRITEGATE
        if (IS_WRITE() ) break;
      if (IS_WRITE() )  //write gate on               
      {
        setup_timer1_for_read();      
        read_data(bitLength, dataBuffer, 515);
      #if WDT_ENABLED                                
        wdt_reset();
      #endif  //WDT_ENABLED  
        setup_timer1_for_write(); //Write-mode: arduinoFDC immediately tries to verify written sector
        setSectorData(lba); //save sector to SD
        while (!(PINC & bit(PIN_WRITEGATE)));//wait for write to finish
      }
      else  //write gate off                                  
      {
        dataBuffer[0]   = 0xFB; // "data" id
        uint16_t crc = calc_crc(dataBuffer, 513);
        dataBuffer[513] = crc/256;
        dataBuffer[514] = crc&255;
        dataBuffer[515] = 0x4E; // first byte of post-data gap        
        write_data(bitLength, dataBuffer, 516);                                      
      }
    }//sectors             
  }//selected
  SET_DSKCHANGE_HIGH();
  SET_WRITEPROT_HIGH();
}