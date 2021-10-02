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

#include "FDDcommon.h"
#include "FloppyDrive.h"
#include "pff.h"
#include "diskio.h"
#include "simpleUART.h"
#include "avrFlux.h"
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "constStrings.h"
#include "FDisplay.h"

//Global variables
#ifdef ENABLE_DRIVE_B
  #warning Two drive mode is not ready yet
  #define N_DRIVES 2
#else 
  #define N_DRIVES 1
#endif //ENABLE_DRIVE_B

static class FloppyDrive drive[N_DRIVES];
class FloppyDrive *pDrive=drive;

static uint8_t dataBuffer[516];
volatile int iTrack = 0;
volatile uint8_t drvSel = 0;

#ifdef DEBUG
uint8_t debugSectors = 0;
#endif

//Interrupt routines
ISR(INT0_vect) //int0 pin 2 of port D
{
  if (!(PIND & bit(PIN_STEP))) //debounce
    iTrack = (PIND & bit(PIN_STEPDIR)) ? --iTrack : ++iTrack;
}

//Two drive mode requires SELECT and MOTOR pins combined trough an OR gate
//if two drive mode is enabled SELECTA pin is used for combined SELECTA & MOTORA
//and MOTORA pin is used for combined SELECTB & MOTORB
ISR(PCINT2_vect) //pin change interrupt of port D
{
#ifdef ENABLE_DRIVE_B
  if (!(PIND & (1 << PIN_SELECTA))) drvSel = 1; //drive A is selected 
  else if (!(PIND & (1 << PIN_MOTORA))) drvSel = 2; //drive B is selected   
#else //Drive B not enabled
  if ( (!(PIND & bit(PIN_SELECTA))) && (!(PIND & bit(PIN_MOTORA))) ) drvSel = 1; //driveA is selected 
#endif  //ENABLE_DRIVE_B
  else drvSel = 0;
}

int getSectorData(long start_sector, int lba)
{
  int n = FR_DISK_ERR;
  uint8_t *pbuf=dataBuffer+1;  
  
  #ifdef DEBUG 
  uint8_t numSec = debugSectors;
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
  
  if (start_sector)
  {
    n = disk_read_sector(pbuf, start_sector+lba);
    if (n) errorMessage(err_diskread);
  }
  else errorMessage(err_fnopen);
  return n;
}

int setSectorData(long start_sector, int lba)
{
  int n = FR_DISK_ERR;
  uint8_t *pbuf=dataBuffer+1;  
  
  if (start_sector)
  {
    n = disk_write_sector(pbuf, start_sector+lba);
    if (n) errorMessage(err_diskwrite);
  }
  else errorMessage(err_fnopen);
  
  #ifdef DEBUG
  uint8_t numSec = debugSectors; 
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

void FDDloop()
{
  static int track=0;
  static uint8_t side=0;
  static uint8_t sector=0;
  static int lba;
  static uint8_t cDrv; //current drive

  if (drvSel) cDrv = drvSel - 1;
  else return; 
  disp.setDriveActive(drvSel);  
  if (drive[cDrv].diskChange) 
  {
    SET_DSKCHANGE_LOW();
    if ((drive[cDrv].fName[0])) drive[cDrv].diskChange = 0; //if a disk is loaded clear diskChange flag
  }
  else (drive[cDrv].fName[0]) ? SET_DSKCHANGE_HIGH() : SET_DSKCHANGE_LOW(); //disk present ?  
  (drive[cDrv].fAttr & AM_RDO) ? SET_WRITEPROT_LOW() : SET_WRITEPROT_HIGH();  //check readonly  
  setup_timer1_for_write(); 
  while(drvSel) //PCINT for SELECTA and MOTORA
  {    
    //Start track with index signal
    SET_INDEX_LOW();
    track_start(drive[cDrv].bitLength);       
    SET_INDEX_HIGH();
    
    for (sector=0; (sector < drive[cDrv].numSec) && drvSel; sector++)
    {                                              
      wdt_reset();
      if (!drvSel) break; //if drive unselected exit loop      
      side = (PIND & bit(PIN_SIDE)) ? 0:1; //check side
      if (track != iTrack) //if track changed
      {                     
        track = iTrack;
        if (track < 0) track=0; //Check if track valid
        else if (track >= drive[cDrv].numTrack) track=drive[cDrv].numTrack-1;          
        if (track == 0) SET_TRACK0_LOW();
        else  SET_TRACK0_HIGH(); 
        iTrack = track;
        (drive[cDrv].fName[0]) ? SET_DSKCHANGE_HIGH() : SET_DSKCHANGE_LOW(); //disk present ?
      }
      //start sector
      lba=(track*2+side)*18+sector;//LBA = (C × HPC + H) × SPT + (S − 1)
      getSectorData(drive[cDrv].startSector, lba); //get sector from SD
      setup_timer1_for_write();
      genSectorID((uint8_t)track,side,sector);
      sector_start(drive[cDrv].bitLength);          
      if (track != iTrack) continue; //check track change
      //check WriteGate
      for (int i=0; i<20; i++) //wait 20X cycles for WRITEGATE
        if (!(PINC & bit(PIN_WRITEGATE))) break;
      if (PINC & bit(PIN_WRITEGATE))
      {//write gate off                                  
        dataBuffer[0]   = 0xFB; // "data" id
        uint16_t crc = calc_crc(dataBuffer, 513);
        dataBuffer[513] = crc/256;
        dataBuffer[514] = crc&255;
        dataBuffer[515] = 0x4E; // first byte of post-data gap        
        write_data(drive[cDrv].bitLength, dataBuffer, 516);                                      
      }
      else
      {//write gate on               
      setup_timer1_for_read();      
      read_data(drive[cDrv].bitLength, dataBuffer, 515);                              
      wdt_reset();
      setup_timer1_for_write(); //Write-mode: arduinoFDC immediately tries to verify written sector
      setSectorData(drive[cDrv].startSector, lba); //save sector to SD
      }      
    }//sectors             
  }//selected
  disp.setDriveIdle();
  SET_DSKCHANGE_HIGH();
  SET_WRITEPROT_HIGH();
}
