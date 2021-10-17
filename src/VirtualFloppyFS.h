#ifndef VIRTUALFLOPPYFS_H
#define VIRTUALFLOPPYFS_H

#include <stdint.h>
#include <stdbool.h>

struct __attribute__((__packed__)) PartitionRecord{
uint8_t		status; //bootable?
uint8_t		startCHS[3];
uint8_t		parttype;
uint8_t		endCHS[3];
uint32_t	startSector;	//LBA
uint32_t	nSectors;	//number of sectors
};

struct __attribute__((__packed__)) MasterBootRecord{
uint8_t		bootstrap[446];
struct PartitionRecord part[4];
uint16_t	signature;	//0x55  0xAA
};

//source: http://www.tavi.co.uk/phobos/fat.html
struct __attribute__((__packed__)) FatBootRecord{
uint8_t 	bootStrap_jmp[3];	//jmp instruction of boot strap program
uint8_t		desc[8];
uint16_t 	bytesPerSector;
uint8_t		sectorsPerCluster;
uint16_t	reservedSectors;
uint8_t		nFATs;
uint16_t	nRootEntries;
uint16_t	nSectors; //Total sectors on disk if > 65535 set to 0 and true size at offset 0x20
uint8_t		mediaDescriptor;
uint16_t	sectorsPerFAT;
uint16_t	sectorsPerTrack;
uint16_t	nHeads;
uint32_t	nHiddenBlocks;	//Historical = 0
uint32_t	nSectorsLong;   //Total sectors on disk
uint16_t	driveNumber;
uint8_t		extendedBootBlockSign;
uint32_t	volumeSerial;
uint8_t		volumeLabel[11];
uint8_t		fsID[8];
uint8_t		bootStrap[448]; //boot strap program
uint16_t	signature;	//	0x55 0xAA
};

typedef struct FatBootRecord FatBS;
typedef struct PartitionRecord PART;
typedef struct MasterBootRecord MBR;

class VirtualFloppyFS{
    private:
    uint32_t sdRootSect;
    uint32_t findSDrootSector(uint8_t *buffer);
    void genBootSector(FatBS *pBS);
    void genFatSector(uint8_t *buffer, uint16_t sector);
    void genRootDir(uint8_t *buffer, uint16_t sector);
    void genDataSector(uint8_t *buffer, uint16_t sector);
    public:    
    VirtualFloppyFS();
    void readSector(uint8_t *buffer, uint16_t sector);
    void writeSector(uint8_t *buffer, uint16_t sector);
};

extern class VirtualFloppyFS vffs;

#endif //VIRTUALFLOPPYFS_H
