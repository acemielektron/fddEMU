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

#include "u8g.h"

#define NOTICE_TIMEOUT	40
#define SLEEP_TIMEOUT	255


//Higher nibble of page
#define BIT_DRIVEB  7   //drive B is selected
#define BIT_DRIVEA  6   //drive A is selected
#define BIT_BUSY    5   //drive in use

//Lower nibble of page
#define PAGE_SPLASH	0
#define PAGE_STATUS	1
#define PAGE_NOTICE 2
#define PAGE_MENU	3

#define FNAME_SIZE	13
#define MENU_ITEMS	5

class FDISPLAY{
private:
u8g_t u8g;
uint8_t idle_timer;
uint8_t sleep_timer;
uint8_t notice_timer;
uint8_t page;
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
char menu_strings[MENU_ITEMS][FNAME_SIZE];
int8_t menu_sel;	//menu index
FDISPLAY();
void setDriveActive(uint8_t);
void setDriveIdle() {page &= 0x0F; setPage(PAGE_STATUS);}
void selectDriveA() {page &= 0x0F; page |= (1 << BIT_DRIVEA);}
void selectDriveB() {page &= 0x0F; page |= (1 << BIT_DRIVEB);}
uint8_t getSelectedDrive() {return (page >> 6);} //2 MSB bits are selected drive
void setPage(uint8_t);
uint8_t getPage() {return (page & 0x0F);}   //return lower nibble
void showNoticeP(const char *, const char *);
void update();
};

extern class FDISPLAY disp;

#endif //FDISPLAY_H
