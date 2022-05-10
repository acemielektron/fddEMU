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

#include "fddEMU.h"
#include "VirtualFloppyFS.h"
#include "FatStructures.h"
#include "FloppyDrive.h"
#include "petitfs/diskio.h" //direct SD access
#include "petitfs/pff.h" //file attributes
#include <string.h>
 

//Virtual Floppy Boot sector
#define BPS     512     //bytes per sector
#define SPT     18      //sectors per track
#define SPC     1       //sectors per cluster
#define NRSV    1       //number of reserved sectors
#define NFAT    2       //number of FATs
#define SPFAT   9       //sectors per fat
#define NROOT   224     //number of root directory entries
#define NSEC    2880    //number of total sectors

const FatBS vfBootSector    PROGMEM = {
    {0xEB, 0x3C, 0x90},//jmp instruction
    {'f','d','d','E','M','U',' ',' '},//description
    BPS,        //bytesPerSector
    SPC,        //sectorsPerCluster
    NRSV,       //reservedSectors
    NFAT,       //nFATs
    NROOT,      //nRootEntries
    NSEC,       //nSectors 
    0xF0,       //mediaDescriptor
    SPFAT,      //sectorsPerFAT
    SPT,        //sectorsPerTrack
    2,          //nHeads
    0,          //nHiddenBlocks
    0,          //nSectorsLong
    0,          //driveNumber
    0,          //ntFlags
    0x29,       //signature
    0xFDDE0000, //volumeSerial
    {'F','D','E','M','U',' ',' ',' ',' ',' '},//volumeLabel
    {'F','A','T','1','2',' ',' ',' '},//fsID
    //bootStrap code
    {0x0E,0x1F,0xBE,0x5B,0x7C,0xAC,0x22,0xC0,0x74,0x0B,0x56,0xB4,0x0E,0xBB,0x07,0x00,
     0xCD,0x10,0x5E,0xEB,0xF0,0x32,0xE4,0xCD,0x16,0xCD,0x19,0xEB,0xFE,0x54,0x68,0x69,
     0x73,0x20,0x69,0x73,0x20,0x6E,0x6F,0x74,0x20,0x61,0x20,0x62,0x6F,0x6F,0x74,0x61,
     0x62,0x6C,0x65,0x20,0x64,0x69,0x73,0x6B,0x2E,0x20,0x20,0x50,0x6C,0x65,0x61,0x73,
     0x65,0x20,0x69,0x6E,0x73,0x65,0x72,0x74,0x20,0x61,0x20,0x62,0x6F,0x6F,0x74,0x61,
     0x62,0x6C,0x65,0x20,0x66,0x6C,0x6F,0x70,0x70,0x79,0x20,0x61,0x6E,0x64,0x0D,0x0A,
     0x70,0x72,0x65,0x73,0x73,0x20,0x61,0x6E,0x79,0x20,0x6B,0x65,0x79,0x20,0x74,0x6F,
     0x20,0x74,0x72,0x79,0x20,0x61,0x67,0x61,0x69,0x6E,0x20,0x2E,0x2E,0x2E,0x20,0x0D,
     0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    0xAA55      //bootSign
};



#if ENABLE_VFFS
class VirtualFloppyFS vffs;
#endif //ENABLE_VFFS

//Disks folder
#define FIRST_CLUSTER   2
#define NDISKS          1024    //max number of disks in DISKS folder
#define DISKS_START     1 //start of disks
#define DISKS_SECTORS  ((NDISKS * sizeof(DIRE) + 511)/512)
#define DISKS_END       DISKS_START + DISKS_SECTORS
#define DRV_A_SECTOR   DISKS_END
#define DRV_B_SECTOR   DRV_A_SECTOR + 1


#define FAT_SECTOR      0 + NRSV                    //boot_sector + reserved_sectors
#define ROOT_SECTOR     FAT_SECTOR+(NFAT*SPFAT)    //fat_sector + (number_of_fats*sectors_per_fat)
#define DATA_SECTOR     ROOT_SECTOR+(((NROOT*sizeof(DIRE))+511)/512)  //root_sector + ((n_root_entries*dir_entry_size +511)/512)
     

uint32_t findSDrootSector(uint8_t *buffer)
{
    uint32_t bootSect = 0; 
    uint32_t rootSect = 0;

    MBR *pMBR = (MBR *)buffer;
    FatBS *pBS = (FatBS *)buffer;
    Fat32BS *p32BS = (Fat32BS *)buffer;

    if (disk_read_sector(buffer, 0)) 
        return 0;
    bootSect =  pMBR->part[0].startSector;
    disk_read_sector(buffer, bootSect);
    if ( (pBS->fsID[0 == 'F']) && (pBS->fsID[0 == 'A']) )
        rootSect = bootSect + pBS->reservedSectors + (pBS->nFATs*pBS->sectorsPerFAT);
    else if ( (p32BS->fsID[0 == 'F']) && (p32BS->fsID[0 == 'A']) )
        rootSect = bootSect + p32BS->reservedSectors + (p32BS->nFATs*p32BS->sectorsPerFAT) +( (p32BS->rootCluster -2)*p32BS->sectorsPerCluster );
#if DEBUG    
    Serial.print(F("SD boot sect: "));
    Serial.print(bootSect);
    Serial.print(F(" root sect: "));
    Serial.print(rootSect);
    Serial.write('\n');
#endif    
    return rootSect;
}

void makeDirEntryP(const char *fname, const char *fext, uint8_t fattr, uint32_t cluster, uint32_t fsize, DIRE *entry)
{
//memset(entry, 0, sizeof(DIRE));
memset(entry->name, ' ', 11); //clear name & ext
memcpy_P((char *)entry->name, fname, strlen_P(fname));
if (!(fattr & (AM_DIR | AM_VOL)) )
    memcpy_P((char *)entry->ext, fext, strlen_P(fext));
entry->attr = fattr;
entry->clustHI = cluster >> 16;
entry->clustLO = cluster & 0x0000FFFF;
entry->fileSize = fsize;
}

VirtualFloppyFS::VirtualFloppyFS()
{
    sdRootSect = 0;
    memset(filename, 0, 13);
    flags = 0;
}

void VirtualFloppyFS::loadImage()
{
    if (flags & (DRIVE0|DRIVE1)) //check if a drive is flagged for VFFS load
      {
        if (flags & DRIVE0) drive[0].load(filename);
      #if ENABLE_DRIVE_B
        else if (flags & DRIVE1) drive[1].load(filename);
      #endif //ENABLE_DRIVE_B  
      flags = 0; //reset virtual floppy flags
      }
}

uint16_t VirtualFloppyFS::readSector(uint8_t *buffer, uint16_t sector)
{
    if (sector == 0) 
        genBootSector(buffer);
    else if (sector >= FAT_SECTOR && sector < ROOT_SECTOR)
        genFatSector(buffer, sector);
    else if (sector >= ROOT_SECTOR && sector < DATA_SECTOR)    
        genRootDir(buffer, sector);
    else 
        genDataSector(buffer, sector);    
    return 0; //on error return != 0    
}

uint16_t VirtualFloppyFS::writeSector(uint8_t *buffer, uint16_t sector)
{    
    if (sector >= DATA_SECTOR) 
    {
        uint16_t r_sector = DATA_SECTOR;
        r_sector = sector - r_sector;

        if (r_sector == DRV_A_SECTOR)
        {
            buffer[12] = 0;      
        #if DEBUG          
            Serial.print(F("Req image: "));
            Serial.print((char *)buffer);
            Serial.write('\n');
        #endif //DEBUG   
            memcpy(filename, buffer, 13); //store filename
            flags |= DRIVE0;
        }
    #if ENABLE_DRIVE_B    
        else if (r_sector == DRV_B_SECTOR)
        {
            buffer[12] = 0;            
        #if DEBUG    
            Serial.print(F("Req image: "));
            Serial.print((char *)buffer);
            Serial.write('\n');
        #endif //DEBUG    
            memcpy(filename, buffer, 13); //store filename
            flags |= DRIVE1;
        }
    #endif //ENABLE_DRIVE_B
    }        
    return 0; //on error return != 0    
}

void VirtualFloppyFS::genBootSector(uint8_t *buffer)
{
    //find root directory of SD card
    if (sdRootSect == 0) 
        sdRootSect = findSDrootSector(buffer);
    //prepare virtual floppy boot sector
    memcpy_P(buffer, &vfBootSector, 512);
}

void VirtualFloppyFS::genFatSector(uint8_t *buffer, uint16_t sector)
{
    uint16_t r_sector = FAT_SECTOR;
    uint16_t f_sectors = SPFAT;
    r_sector = sector - r_sector;

    memset(buffer, 0, 512); //fill buffer with zeroes
    if (r_sector >= f_sectors) //if second copy of FAT
        r_sector -= f_sectors;
    if (r_sector == 0)
    {
        setCluster(0, 0x0FF0, buffer);
        setCluster(1, 0x0FFF, buffer);
        genClusters(DISKS_START+FIRST_CLUSTER, DISKS_SECTORS, buffer);
        setCluster(DRV_A_SECTOR+FIRST_CLUSTER, 0x0FFF, buffer);
        setCluster(DRV_B_SECTOR+FIRST_CLUSTER, 0x0FFF, buffer);
    }
}

void VirtualFloppyFS::genRootDir(uint8_t *buffer, uint16_t sector)
{   
    uint16_t r_sector = ROOT_SECTOR;
    r_sector = sector - r_sector;

    memset(buffer, 0, 512);
    if (r_sector == 0)
    {
        DIRE *pDir = (DIRE *) buffer;    
                
        //Set up volume label
        makeDirEntryP(str_label, NULL, AM_VOL, 0, 0, &pDir[0]);
        //Set up disks folder        
        makeDirEntryP(str_disks, NULL, AM_DIR, DISKS_START+FIRST_CLUSTER, 0, &pDir[1]);
        //Set up Drive 0 file
        makeDirEntryP(str_load0name, str_loadext, 0, DRV_A_SECTOR+FIRST_CLUSTER, 13, &pDir[2]);
    #if ENABLE_DRIVE_B    
        //Set up Drive 1 file
        makeDirEntryP(str_load1name, str_loadext, 0, DRV_A_SECTOR+FIRST_CLUSTER, 13, &pDir[3]);
    #endif //ENABLE_DRIVE_B    
    }    
}

void VirtualFloppyFS::genDataSector(uint8_t *buffer, uint16_t sector)
{
    uint16_t r_sector = DATA_SECTOR;
    r_sector = sector - r_sector;

    //Transfer SD root directory entries in DISKS directory of Virtual Floppy
    if ( (r_sector >= DISKS_START) && (r_sector < DISKS_END) )
    {                
        DIRE *pDir = (DIRE *) buffer;
        r_sector -= DISKS_START;                 

        if (r_sector == 0) flags &= ~F_EOFLIST; //clear EOF flag;
        if (sdRootSect)
        {                    
            if (flags & F_EOFLIST) memset(buffer, 0, 512);
            else disk_read_sector(buffer, sdRootSect + r_sector); //return SD card root sector            
            for (uint8_t i=0; i< 512/sizeof(DIRE); i++)
            {
                if (pDir[i].name[0] == 0) flags |= F_EOFLIST;
                pDir[i]. clustHI = 0;
                pDir[i]. clustLO = 0;
                //pDir[i].fileSize = 0;
            }
        }       
        else memset(buffer, 0, 512);
        
        if (r_sector == 0)
        {   //overwrite first 2 entries with . and .. entries 
            //Probably volume label and first LFN record are overwritten
            makeDirEntryP(str_dot, NULL, AM_DIR, DISKS_START+FIRST_CLUSTER, 0, &pDir[0]);
            makeDirEntryP(str_2dot, NULL, AM_DIR, 0, 0, &pDir[1]);
        }
    }    
    else if (r_sector == DRV_A_SECTOR)  //Drive0 currently loaded file
        memcpy(buffer, drive[0].fName, 13);
#if ENABLE_DRIVE_B        
    else if (r_sector == DRV_B_SECTOR)  //Drive1 currently loaded file
        memcpy(buffer, drive[1].fName, 13);
#endif //ENABLE_DRIVE_B        
    else memset(buffer, 0, 512); //fill buffer with zeroes
}

void Fat12::setCluster(uint16_t cluster, uint16_t next_cluster, uint8_t *buffer)
{
	int16_t offset = cluster + cluster/2; //position of cluster in FAT

	if (cluster & 1)	//odd offset
		{
		*(uint16_t *)(buffer + offset) |= next_cluster << 4;
		}
	else	//even offset
		{
		*(uint16_t *)(buffer + offset) |= next_cluster &0xFFF;
		}
}

void Fat12::genClusters(uint16_t firstCluster, int16_t n, uint8_t *buffer)
{
for (uint16_t cluster=firstCluster; cluster < firstCluster+n; cluster++)
	{
	int16_t next_cluster = cluster+1;
	if (cluster == firstCluster+n-1) next_cluster = 0x0FFF;

	setCluster(cluster, next_cluster, buffer);
	}
}