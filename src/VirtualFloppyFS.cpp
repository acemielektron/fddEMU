#include "VirtualFloppyFS.h"
#include "FloppyDrive.h"
#include "constStrings.h"
#include "diskio.h" //direct SD access
#include "simpleUART.h" //debug
#include "pff.h" //file attributes
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
            drive[0].load((char *)buffer);
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
            drive[1].load((char *)buffer);
        }
    #endif //ENABLE_DRIVE_B
    }        
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
    memset(pBS->volumeLabel, ' ', 11);
    memcpy_P(pBS->volumeLabel, str_label, 6);
    memcpy_P(pBS->fsID, bs_fsid, 5);
    memcpy_P(pBS->bootStrap, bs_bootStrap, 448); //448 bytes
    pBS->signature = 0xAA55;
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

    if ( (r_sector >= DISKS_START) && (r_sector < DISKS_END) )
    {                
        DIRE *pDir = (DIRE *) buffer;
        r_sector -= DISKS_START;         
        if  (sdRootSect == 0) sdRootSect = findSDrootSector(buffer);        
        if (sdRootSect)
        {               
            disk_read_sector(buffer, sdRootSect + r_sector); //return SD card root sector            
            for (uint8_t i=0; i< 512/sizeof(DIRE); i++)
            {
                pDir[i]. clustHI = 0;
                pDir[i]. clustLO = 0;
                //pDir[i].fileSize = 0;
            }
        }       
        else memset(buffer, 0, 512);
        if (r_sector == 0)
        {   //overwrite fitst 2 entries with . and .. entries 
            //volume label and first LFN record are owerwritten
            makeDirEntryP(str_dot, NULL, AM_DIR, DISKS_START+FIRST_CLUSTER, 0, &pDir[0]);
            makeDirEntryP(str_2dot, NULL, AM_DIR, 0, 0, &pDir[1]);
        }
    }
    else if (r_sector == DRV_A_SECTOR)
        memcpy(buffer, drive[0].fName, 13);
#if ENABLE_DRIVE_B        
    else if (r_sector == DRV_B_SECTOR)
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