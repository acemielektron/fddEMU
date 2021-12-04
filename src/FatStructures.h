#ifndef FATSTRUCTURES_H
#define FATSTRUCTURES_H

#include <stdint.h>

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
uint8_t 	bootStrap_jmp[3];	//@0 jmp instruction of boot strap program
uint8_t		description[8];		//@3
uint16_t 	bytesPerSector;		//@11
uint8_t		sectorsPerCluster;	//@13
uint16_t	reservedSectors;	//@14
uint8_t		nFATs;				//@16
uint16_t	nRootEntries;		//@17
uint16_t	nSectors;           //@19 Total sectors on disk if > 65535 set to 0 and true size at offset 0x20
uint8_t		mediaDescriptor;	//@21
uint16_t	sectorsPerFAT;		//@22
uint16_t	sectorsPerTrack;	//@24
uint16_t	nHeads;				//@26
uint32_t	nHiddenBlocks;	    //@28 Historical = 0
uint32_t	nSectorsLong;		//@32
uint8_t		driveNumber;	    //@36
uint8_t		ntFlags;		    //@37
uint8_t		signature;	        //@38
uint32_t	volumeSerial;	    //@39
uint8_t		volumeLabel[11];	//@43
uint8_t		fsID[8];			//@54
uint8_t		bootStrap[448];     //@62 boot strap program
uint16_t	bootSign;		    //@510 0x55 0xAA
};

//https://wiki.osdev.org/FAT#FAT_32
struct __attribute__((__packed__)) Fat32BootRecord{
uint8_t 	bootStrap_jmp[3];	//@0 jmp instruction of boot strap program
uint8_t		description[8];		//@3
uint16_t 	bytesPerSector;		//@11
uint8_t		sectorsPerCluster;	//@13
uint16_t	reservedSectors;	//@14
uint8_t		nFATs;				//@16
uint16_t	nRootEntries_Old;	//@17 fat12-fat16 only
uint16_t	nSectors_Old;		//@19 fat12-fat16 only
uint8_t		mediaDescriptor;	//@21
uint16_t	sectorsPerFAT_Old;	//@22 fat12-fat16 only
uint16_t	sectorsPerTrack;	//@24
uint16_t	nHeads;				//@26
uint32_t	nHiddenBlocks;		//@28 LBA of the beginning of partition
uint32_t	nSectors;			//@32
uint32_t	sectorsPerFAT;		//@36
uint16_t	flags;				//@40
uint16_t	version;			//@42
uint32_t	rootCluster;		//@44
uint16_t	FSInfoSector;		//@48
uint16_t	backupBootSector;   //@50
uint8_t		reserved[12];		//@52
uint8_t		driveNumber;		//@64
uint8_t		ntFlags;			//@65
uint8_t		signature;			//@66
uint32_t	volumeSerial;		//@67
char		volumeLabel[11];	//@71
char		fsID[8];			//@82 Always "FAT32   "
uint8_t		bootStrap[420];		//@90
uint16_t	bootSign;			//@510 0x55 0xAA
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
typedef struct Fat32BootRecord Fat32BS;
typedef struct DirEntry DIRE;

#endif //FATSTRUCTURES_H