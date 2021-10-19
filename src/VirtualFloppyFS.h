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

struct __attribute__((__packed__)) DirEntry{
uint8_t		name[8];//File name
uint8_t		ext[3];	//File extension
uint8_t		attr;	//File attributes
uint16_t	rsvd1;	//NT reserved
uint16_t	cTime;	//time created
uint16_t	cDate;	//date created
uint16_t	rsvd2;	//reserved
uint16_t	clustHI;//First cluster high word
uint16_t	mTime;	//Time Modified
uint16_t	mDate;	//Date Modified
uint16_t	clustLO;//First cluster low word
uint32_t	fileSize;
};

typedef struct PartitionRecord PART;
typedef struct MasterBootRecord MBR;
typedef struct FatBootRecord FatBS;
typedef struct DirEntry DIRE;

class Fat12{
    public:
    void setCluster(uint16_t cluster, uint16_t next_cluster, uint8_t *buffer);
    void genClusters(uint16_t firstCluster, int16_t n, uint8_t *buffer);
};

class VirtualFloppyFS: public Fat12{
    private:
    uint32_t sdRootSect;    
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
