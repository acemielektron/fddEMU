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

#ifndef FLOPPYDRIVE_H
#define PLOPPYDRIVE_H

#include "FloppyDisk.h"
#include <stdint.h>

#define DRIVEA_SELECT   1
#define DRIVEB_SELECT   2

class FloppyDrive : public FloppyDisk
{
    public:      
    uint8_t bitLength;
    FloppyDrive();
    void init();
    int getSectorData(int lba);
    int setSectorData(int lba);
    void run();
};

extern volatile uint8_t drvSel;

extern class FloppyDrive driveA;
#ifdef ENABLE_DRIVE_B
extern class FloppyDrive driveB;
#endif //ENABLE_DRIVE_B


#endif //PLOPPYDRIVE_H
