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

#include "FloppyDisk.h"
#include "simpleUART.h"
#include "diskio.h"
#include "constStrings.h"
#include "FDisplay.h"
#include "FDDpins.h"
#include "DiskFile.h"


bool FloppyDisk::load(char *filename)
{  
  uint16_t totalSectors;
  uint8_t wbuf[18]; //working buffer;
    
  eject(); 
  // open requested file
  if ( !sdfile.getFileInfo((char *)s_RootDir, filename) )
    return false;
  if ( ( startSector = sdfile.getStartSector() ) == 0)
    return false;
  if (sdfile.getReadOnly()) flags |= FD_READONLY;  
  if (disk_readp(wbuf, startSector, 510, 2)) //Read the boot record         
  {	
    errorMessage(err_diskread);
	  return false;
  }
  if ( (wbuf[0] != 0x55) || (wbuf[1] != 0xAA) ) //Invalid boot sector -> Raw disk image ?
  {
    totalSectors = sdfile.getFileSize() >> 9;  //convert filesize to 512 byte sectors (filesize / 512)
    switch(totalSectors)  //check filesize in sectors
    { //Standart floppy: C*H*S*512      
      case (uint16_t)(80*2*18): //3.5" HD   1.440MB
        numTrack = 80;
        numSec = 18;
        break;
      case (uint16_t)(80*2*9):  //3.5" DD   720KB
        numTrack = 80;
        numSec = 9;
        break;
      case (uint16_t)(80*2*15): //5.25" HD  1.2MB
        numTrack = 80;
        numSec = 15;
        break;
      case (uint16_t)(40*2*9):  //5.25" DD  360KB
        numTrack = 40;
        numSec = 9;
        break;
      default:  //not a standart raw floppy image
        errorMessage(err_invboot);
	      return false;
    } //switch        
  }
  else  //Valid boot signature
  {    
    disk_readp(wbuf, startSector, 54, 18); //FileSystemType@54
  #if DEBUG
    Serial.print(F("FS: "));
    for (int i=0; i < 5; i++) Serial.write(wbuf[i]);
    Serial.write('\n');
  #endif  
    if ( (wbuf[0] != 'F') || (wbuf[1] != 'A') || (wbuf[2] != 'T') || (wbuf[3] != '1') )
    {
      errorMessage(err_notfat12);
	    return false;
    }
    disk_readp(wbuf, startSector, 11, 18);  //WbytesPerSector@11 WnumHeads@26 WsectorsPerTrack@24        
  #if DEBUG
    Serial.print(F("BPS: "));
    Serial.printDEC(*(int16_t *)wbuf);
    Serial.print(F(" Heads: "));
    Serial.printDEC(*(int16_t *)(wbuf+15));
    Serial.print(F(" SPT: "));
    Serial.printDEC(*(int16_t *)(wbuf+13));
    Serial.print(F(" Total Sec: "));
    Serial.printDEC( *(uint16_t *)(wbuf+8));
    Serial.write('\n');
  #endif  
    if ( (*(int16_t *)wbuf != 512) || (*(int16_t *)(wbuf+15) > 2) || (*(int16_t *)(wbuf+13) > 255) )           
    {
      errorMessage(err_geometry);
	    return false;        
    }
    totalSectors = (uint16_t) *(int16_t *)(wbuf+8);  //WtotalSectors@19
    if (totalSectors > (sdfile.getFileSize() >> 9))  
    {
      errorMessage(err_geombig);
	    return false;        
    }
    numSec = (uint8_t) *(int16_t *)(wbuf+13);     //WsectorsPerTrack@24
    numTrack = (uint8_t) ( totalSectors / (numSec*2) );
    //Print info
    Serial.print(F("Geom: "));
    Serial.printDEC(numTrack);
    Serial.print(F("/2/"));
    Serial.printDEC(numSec);
    Serial.write('\n');  
  }
  //After all the checks load image file
  memcpy(fName, filename, 13);   
  flags |= FD_READY;  
  return true;
}

void FloppyDisk::eject(void)
{
  startSector = 0;  
  fName[0] = '\0';  //clear disk file name
  numTrack = 80; //default for 3.5" HD Floppy
  numSec = 18; //default for 3.5" HD Floppy
  flags = FD_CHANGED; //clear flags & set DISK CHANGED
}

FloppyDisk::FloppyDisk(void)
{
  eject();
}
