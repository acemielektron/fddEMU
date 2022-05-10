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

#ifndef GRAPHIC_UI_H
#define GRAPHIC_UI_H

#include "FloppyDrive.h" //BIT_DRIVE0 & BIT_DRIVE1
#include "u8g.h"

#define NOTICE_TIMEOUT	20
#define SLEEP_TIMEOUT	255


#define PAGE_STATUS	    0
#define PAGE_NOTICE     1
#define PAGE_MENU	    2
#define PAGE_LOADING    3
#define PAGE_BUSY       4
#define PAGE_SPLASH	    5

#define FNAME_SIZE	13
#define MENU_ITEMS	6

class U8G{  //Encapsulate some u8g functions
    protected:
    u8g_t u8g;

    public:
    void drawStrP(int x, int y, const char *str) {u8g_DrawStrP(&u8g, x, y,(const u8g_pgm_uint8_t*)str);}
    void drawStr(int x, int y, char *str) {u8g_DrawStr(&u8g, x, y,(const char *)str);}
    void drawXBMP(u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h, const u8g_pgm_uint8_t *bitmap) {u8g_DrawXBMP(&u8g, x, y, w, h, bitmap);}
    u8g_uint_t getStrWidthP(const char *str) {return u8g_GetStrWidthP(&u8g, (const u8g_pgm_uint8_t*)str);}
    u8g_uint_t getStrWidth(char *str) {return u8g_GetStrWidth(&u8g, (const char*)str);}
    void sleepOn() {u8g_SleepOn(&u8g);}
    void sleepOff() {u8g_SleepOff(&u8g);}
};

class GraphicUI: public U8G {
private:
struct __attribute__((__packed__)) uiFlags
{
	unsigned int page : 6;
	unsigned int driveSel : 2;
}f;
uint8_t idle_timer;
uint8_t sleep_timer;
uint8_t notice_timer;
const char *notice_header;
const char *notice_message;
void drawPage();
void init();
void splashScreen();
void loadingScreen();
void busyScreen();
void noticeScreen();
void statusScreen();
void drawMenu(void);

public:
int16_t idx_sel; //where were we in the menu index ?
int8_t menu_sel;	//menu index
int8_t menu_max;
char menuFileNames[MENU_ITEMS][FNAME_SIZE];
GraphicUI();
void setPage(uint8_t);
uint8_t getPage() {return f.page;} 
void showNoticeP(const char *, const char *);
void showNotice(const char *, char *);
void update();
void showDriveBusy(uint8_t);
void showDriveIdle();
void showDriveLoading();
void selectDrive(uint8_t r_drive) {f.driveSel = r_drive;}
uint8_t getSelectedDrive() {return f.driveSel;} 
bool isDriveA() {return f.driveSel & (1 << 0) ? true:false;}
bool isDriveB() {return f.driveSel & (1 << 1) ? true:false;}
void loadMenuFiles();
void buttonAction(int8_t button);
};

extern class GraphicUI disp;

#endif //GRAPHIC_UI_H
