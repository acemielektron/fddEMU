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
    void loadImage(); 
};

extern class VirtualFloppyFS vffs;

#endif //VIRTUALFLOPPYFS_H
