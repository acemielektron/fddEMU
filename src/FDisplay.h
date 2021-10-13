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

#ifndef FDISPLAY_H
#define FDISPLAY_H

#include "FloppyDrive.h" //BIT_DRIVE0 & BIT_DRIVE1
#include "u8g.h"

#define NOTICE_TIMEOUT	20
#define SLEEP_TIMEOUT	255


#define BIT_BUSY    7   //drive in use

//Lower nibble of page
#define PAGE_STATUS	0
#define PAGE_NOTICE 1
#define PAGE_MENU	2
#define PAGE_SPLASH	3

#define FNAME_SIZE	13
#define MENU_ITEMS	6

class FDISPLAY{
private:
u8g_t u8g;
uint8_t idle_timer;
uint8_t sleep_timer;
uint8_t notice_timer;
uint8_t page;
uint8_t drive_sel;
const char *notice_header;
const char *notice_message;
void drawPage();
void init();
void splashScreen();
void noticeScreen();
void statusScreen();
void drawMenu(void);
void drawStrP(int x, int y, const char *str) {u8g_DrawStrP(&u8g, x, y,(const u8g_pgm_uint8_t*)str);}
void drawStr(int x, int y, char *str) {u8g_DrawStr(&u8g, x, y,(const char *)str);}
void sleep() {u8g_SleepOn(&u8g);}
void wakeup() {u8g_SleepOff(&u8g);}

public:
int8_t menu_sel;	//menu index
int8_t menu_max;
char menuFileNames[MENU_ITEMS][FNAME_SIZE];
FDISPLAY();
void setPage(uint8_t);
uint8_t getPage() {return page;} 
void showNoticeP(const char *, const char *);
void update();
void setDriveBusy(uint8_t);
void setDriveIdle() {setDriveBusy(0);}
void selectDrive(uint8_t r_drive) {drive_sel = r_drive & ((1 << BIT_DRIVE0)|(1 << BIT_DRIVE1));}
uint8_t getSelectedDrive() {return (drive_sel & ((1 << BIT_DRIVE0)|(1 << BIT_DRIVE1)) );} 
uint8_t isDrive0() {return drive_sel & (1 << BIT_DRIVE0);}
uint8_t isDrive1() {return drive_sel & (1 << BIT_DRIVE1);}
uint8_t isBusy() {return drive_sel & (1 << BIT_BUSY);}
};

extern class FDISPLAY disp;

#endif //FDISPLAY_H
