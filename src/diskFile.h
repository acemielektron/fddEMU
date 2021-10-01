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

#ifndef DISKFILE_H
#define DISKFILE_H

#include "FDDcommon.h"

class DiskFile{
    public:
    uint8_t diskChange;
    uint8_t numTrack;   //number of tracks
    uint8_t numSec;     //sectors per track
    long startSector;
    char fName[13];
    uint8_t fAttr;    
    DiskFile();
    int loadDisk(char *);
    void ejectDisk();
};

#endif
