#include "DiskFile.h"
#include "simpleUART.h"
#include "FDisplay.h"
#include "constStrings.h"
#include <string.h>
#if WDT_ENABLED  
  #include <avr/wdt.h>
#endif //WDT_ENABLED  

DiskFile sdfile; //we will use as extern

bool DiskFile::initSD()
{  
  // initialize the SD card
  res = pf_mount(&fs);
  if (res == FR_OK) sdInitialized = true;
  else
  {
    errorMessage(err_initSD);
    return false;
  }
  return true;    
}

DiskFile::DiskFile()
{
    sdInitialized = false;
    initSD();
}

void DiskFile::printFileName()
{
    if (fno.fattrib & AM_DIR) Serial.write('[');
    Serial.print(fno.fname);                
    if (fno.fattrib & AM_DIR) Serial.write(']');    
}

int16_t DiskFile::scanFiles(char *path)
{      
    nFiles = 0;    
    if ( !openDir(path) ) 
      return 0;
    do 
    {
    #if WDT_ENABLED  
        wdt_reset();
    #endif //WDT_ENABLED            
        if ( !getNextFile() ) break;
        if (fno.fattrib & (AM_VOL | AM_LFN | AM_DIR) ); //skip DIR, LFN, VOL entries
        else
        {
            nFiles++;
        #if DEBUG
            printFileName();
            Serial.write('\n');
        #endif //DEBUG
        }
    } while (fno.fname[0] != 0);
    return nFiles;
}

bool DiskFile::openDir(char *path)
{
    if ( (res = pf_opendir(&dir, path)) == FR_OK) 
        return true;
    else if (res & (FR_DISK_ERR | FR_NOT_READY))
    {
      if (initSD()) //reinitialize SD            
        if ( (res = pf_opendir(&dir, path)) == FR_OK) //retry
          return true;
    }
    return false;  
}

bool DiskFile::getNextFile()
{
  res = pf_readdir(&dir, &fno);  
  if (res != FR_OK) 
    return false;
  if (fno.fname[0] == 0) res = FR_NO_FILE;    
  return true;  
}

bool DiskFile::getFileInfo(char *path, char *filename)
{ 
  if ( !openDir(path) ) 
    return false;
  while ( getNextFile() )
  {
      if (strcmp(fno.fname, filename) == 0) 
        return true;
  }
  return false;
}

uint32_t DiskFile::getStartSector()
{
    res = pf_open(fno.fname);  
    if (res != FR_OK) 
    {
        errorMessage(err_fopen);
        return 0;
    }   
    if (!isContiguousFile())  //we will use direct sector access so we need a contiguous file
    {
        errorMessage(err_noncontig);
        return 0;
    }
    return get_start_sector();  
}