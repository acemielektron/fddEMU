#include "VirtualFloppyFS.h"
#include "constStrings.h"
#include "diskio.h" //direct SD access
#include "simpleUART.h" //debug
#include <string.h>

class VirtualFloppyFS vffs;

#define BPS     512     //bytes per sector
#define SPT     18      //sectors per track
#define SPC     1       //sectors per cluster
#define NRSV    1       //number of reserved sectors
#define NFAT    2       //number of FATs
#define SPFAT   9       //sectors per fat
#define NROOT   224     //number of root directory entries
#define NSEC    2880    //number of total sectors
#define DIRE    32      //size of directory entry


#define FAT_SECTOR     0 + NRSV                    //boot_sector + reserved_sectors
#define ROOT_SECTOR    FAT_SECTOR+(NFAT*SPFAT)    //fat_sector + (number_of_fats*sectors_per_fat)
#define DATA_SECTOR    ROOT_SECTOR+(((NROOT*DIRE)+511)/512)  //root_sector + ((n_root_entries*dir_entry_size +511)/512)

#if FAT_SECTOR != 1
    #error FAT
#endif
#if ROOT_SECTOR != 19
    #error ROOT
#endif
#if DATA_SECTOR !=33
    #error DATA
#endif        

VirtualFloppyFS::VirtualFloppyFS()
{
    sdRootSect = 0;
}

void VirtualFloppyFS::readSector(uint8_t *buffer, uint16_t sector)
{
    if (sector == 0) 
        genBootSector((FatBS *)buffer);
    else if (sector >= FAT_SECTOR && sector < ROOT_SECTOR)
        genFatSector(buffer, sector);
    else if (sector >= ROOT_SECTOR && sector < DATA_SECTOR)    
        genRootDir(buffer, sector);
    else 
        genDataSector(buffer, sector);    
}

void VirtualFloppyFS::writeSector(uint8_t *buffer, uint16_t sector)
{
    
}

void VirtualFloppyFS::genBootSector(FatBS *pBS)
{
    memset(pBS, 0, 512); //fill boot sector buffer with zeroes
    memcpy_P(pBS->bootStrap_jmp, bs_jump, 3);    
    memcpy_P(pBS->desc, str_fddEMU, 6);    
    pBS->bytesPerSector = BPS;
    pBS->sectorsPerCluster = SPC;
    pBS->reservedSectors = NRSV;
    pBS->nFATs = NFAT;
    pBS->nRootEntries = NROOT;
    pBS->nSectors = NSEC;
    pBS->mediaDescriptor = 0xF0;
    pBS->sectorsPerFAT = SPFAT;
    pBS->sectorsPerTrack = SPT;
    pBS->nHeads = 2;
    pBS->extendedBootBlockSign = 0x29;
    pBS->volumeSerial = 0xF0D0D0E0;
    memcpy_P(pBS->volumeLabel, str_fddEMU, 6);
    memcpy_P(pBS->fsID, bs_fsid, 5);
    memcpy_P(pBS->bootStrap, bs_bootStrap, 448); //448 bytes
    pBS->signature = 0xAA55;
}

void VirtualFloppyFS::genFatSector(uint8_t *buffer, uint16_t sector)
{
    memset(buffer, 0, 512); //fill buffer with zeroes
}

void VirtualFloppyFS::genRootDir(uint8_t *buffer, uint16_t sector)
{       
    if  (sdRootSect == 0) sdRootSect = findSDrootSector(buffer); 
    uint16_t root_sector = ROOT_SECTOR; //?subtraction doesn't work without subtrahend in a variable

    if (sdRootSect)
        disk_read_sector(buffer, sdRootSect + sector - root_sector); //return SD card root sector            
    else memset(buffer, 0, 512);
}

void VirtualFloppyFS::genDataSector(uint8_t *buffer, uint16_t sector)
{
    memset(buffer, 0, 512); //fill buffer with zeroes
}

uint32_t VirtualFloppyFS::findSDrootSector(uint8_t *buffer)
{
    uint32_t bootSect = 0; 
    uint32_t rootSect = 0;

    MBR *pMBR = (MBR *)buffer;
    FatBS *pBS = (FatBS *)buffer;
    if (disk_read_sector(buffer, 0)) 
        return 0;
    bootSect =  pMBR->part[0].startSector;
    disk_read_sector(buffer, bootSect);
    rootSect = bootSect + pBS->reservedSectors + (pBS->nFATs*pBS->sectorsPerFAT);
#if DEBUG    
    Serial.print(F("SD boot sect: "));
    Serial.printDEC(bootSect);
    Serial.print(F(" root sect: "));
    Serial.printDEC(rootSect);
    Serial.write('\n');
#endif    
    return rootSect;
}