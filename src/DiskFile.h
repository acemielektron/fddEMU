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

#include <stdint.h>
#include <stdbool.h>
#include "pff.h"

class DiskFile
{
    private:    
    FATFS fs; //petitfs
    FILINFO fno;
    DIR dir;
    bool sdInitialized = false;
    bool initSD();

    public:
    FRESULT res;
    int16_t nFiles;
    DiskFile();   
    void printFileName();
    int16_t scanFiles(char *path);
    bool openDir(char *path);
    bool getNextEntry();
    bool getNextFile();
    bool getFileInfo(char *path, char *filename);
    uint32_t getStartSector();
    bool getReadOnly() {return (fno.fattrib & AM_RDO);}
    char *getFileName() {return fno.fname;}
    uint32_t getFileSize() {return fno.fsize;}
};

extern DiskFile sdfile;

#endif //DISKFILE_H