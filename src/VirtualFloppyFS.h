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
struct __attribute__((__packed__)) fatbootinfo{
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
struct __attribute__((__packed__)) fat32info{
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
typedef struct fatbootinfo FatBS;
typedef struct fat32info Fat32BS;
typedef struct DirEntry DIRE;

class Fat12{
    public:
    void setCluster(uint16_t cluster, uint16_t next_cluster, uint8_t *buffer);
    void genClusters(uint16_t firstCluster, int16_t n, uint8_t *buffer);
};

//define VirtualFloppy Flags bits
//bit 0 and 1 are drive bits
#define BIT_EOFLIST 2
//define virtual floppy flags
#define F_EOFLIST   (1 << BIT_EOFLIST)

class VirtualFloppyFS: public Fat12{
    private:
    uint32_t sdRootSect;        
    void genBootSector(uint8_t *buffer);
    void genFatSector(uint8_t *buffer, uint16_t sector);
    void genRootDir(uint8_t *buffer, uint16_t sector);
    void genDataSector(uint8_t *buffer, uint16_t sector);    
    public:
    uint8_t flags;
    char filename[13];    
    VirtualFloppyFS();
    uint16_t readSector(uint8_t *buffer, uint16_t sector);
    uint16_t writeSector(uint8_t *buffer, uint16_t sector);    
};

extern class VirtualFloppyFS vffs;

#endif //VIRTUALFLOPPYFS_H
