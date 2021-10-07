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
#include "pff.h"
#include "diskio.h"
#include "constStrings.h"
#include "FDisplay.h"
#include "FDDpins.h"

bool sdInitialized = false;
FATFS fs; //petitfs


FRESULT get_file_info(FILINFO *pfno, char *filename)
{
  FRESULT res = FR_NO_FILE;
  DIR dir;

  res = pf_opendir(&dir, (char *)DISK_DIR);
  if (res == FR_OK)
  {
    for (;;) 
    {
      res = pf_readdir(&dir, pfno);
      if (res != FR_OK || pfno->fname[0] == 0) break;
      if (strcmp(pfno->fname, filename) == 0) return res;
    }
  }
  return res;
}

int initSD()
{
  int rVal;

  sdInitialized = false;
  // initialize the SD card
  rVal = pf_mount(&fs);
  if (rVal == FR_OK) sdInitialized = true;
  if (rVal) errorMessage(err_initSD);
  return rVal;    
}

int FloppyDisk::load(char *filename)
{
  FILINFO fno;
  int rVal=FR_OK;
  uint16_t totalSectors;
    
  eject(); 
  if (!sdInitialized) 
  if  ( (rVal = initSD()) ) 
    return rVal;
  // open a file
  rVal = get_file_info(&fno, filename);  
  rVal = pf_open(filename);  
  if (rVal) 
  {
    errorMessage(err_fopen);
    return -1;
  }
  memcpy(fName, fno.fname, 13); 
  if (fno.fattrib & AM_RDO) flags |= FD_READONLY;
  startSector=get_start_sector();  
  if (!isContiguousFile())  //we will use direct sector access so we need a contiguous file
  {
    errorMessage(err_noncontig);
    return -1;
  }
  totalSectors = fno.fsize >> 9;  //convert filesize to 512 byte sectors (filesize / 512)
  switch(totalSectors)  //check filesize in sectors
    { //Standart floppy: C*H*S*512
      uint8_t wbuf[18]; //working buffer;
      case (uint32_t)(80*2*18): //3.5" HD   1.440MB
        numTrack = 80;
        numSec = 18;
        break;
      case (uint32_t)(80*2*9):  //3.5" DD   720KB
        numTrack = 80;
        numSec = 9;
        break;
      case (uint32_t)(80*2*15): //5.25" HD  1.2MB
        numTrack = 80;
        numSec = 15;
        break;
      case (uint32_t)(40*2*9):  //5.25" DD  360KB
        numTrack = 40;
        numSec = 9;
        break;
      default:  //non-standart image - check C/H/S for FAT images
        if (disk_readp(wbuf, startSector, 510, 2)) //Read the boot record         
        {	
          errorMessage(err_diskread);
		      return -1;
        }
        if ( (wbuf[0] != 0x55) || (wbuf[1] != 0xAA) )
        {
          errorMessage(err_invboot);
		      return -1;
        }
        disk_readp(wbuf, startSector, 54, 5); //FileSystemType@54
        if ( (wbuf[0] != 'F') || (wbuf[0] != 'A') || (wbuf[0] != 'T') || (wbuf[0] != '1') || (wbuf[0] != '2') )
        {
          errorMessage(err_notfat12);
		      return -1;
        }
        disk_readp(wbuf, startSector, 11, 17);  //WbytesPerSector@11 WnumHeads@26 WsectorsPerTrack@24 
        if ( (wbuf[0] != 0) || (wbuf[1] != 0x2) || (wbuf[15] != 0x2) || (wbuf[16] != 0) || (wbuf[14] != 0) )
        {
          errorMessage(err_geometry);
		      return -1;        
        }
        totalSectors = (wbuf[9] << 8) & wbuf[8];  //WtotalSectors@19
        numSec = (wbuf[14] << 8) & wbuf[13];     //WsectorsPerTrack@24
        numTrack = totalSectors / (numSec*2);
    } //switch        
  flags |= FD_READY;  
  return rVal;
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
