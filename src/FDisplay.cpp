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

#include "FDisplay.h"
#include "FloppyDrive.h"
#include "constStrings.h"
#include <string.h>
#include <stdlib.h>
#include "simpleUART.h" //debug

class FDISPLAY disp; //will use as exter
extern class FloppyDrive *pDrive;
char infostring[12]; //drive C/H/S info string


void diskinfo(class FloppyDrive *pdrv)	//Generate disk CHS info string in itoabuf (defined in simpleUART)
{
	char convbuf[4];
	
	if (pdrv->fName[0] == 0)
	{
		strcpy_P(infostring, str_nodisk);
		return;
	}		
	infostring[0] = 'C';
	infostring[1] = 0;
	itoa(pdrv->numTrack, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
	strcat(infostring, "H2S");
	itoa(pdrv->numSec, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
}

//https://github.com/olikraus/u8glib/blob/master/sys/arm/examples/menu/menu.c
void FDISPLAY::drawMenu(void) 
{
  uint8_t i, h;
  u8g_uint_t w, d;

  u8g_SetFont(&u8g, u8g_font_6x10);
  u8g_SetFontRefHeightText(&u8g);
  u8g_SetFontPosTop(&u8g);
  
  h = u8g_GetFontAscent(&u8g)-u8g_GetFontDescent(&u8g);
  w = u8g_GetWidth(&u8g);
  for( i = 0; i < MENU_ITEMS; i++ ) 
  {
    d = (w-u8g_GetStrWidth(&u8g, menu_strings[i]))/2;
    u8g_SetDefaultForegroundColor(&u8g);
    if ( i == menu_sel ) 
    {
      u8g_DrawBox(&u8g, 0, i*h+1, w, h);
      u8g_SetDefaultBackgroundColor(&u8g);
    }
    u8g_DrawStr(&u8g, d, i*h, menu_strings[i]);
  }
}

void FDISPLAY::statusScreen()
{
#define X_OFS	1
#define Y_OFS_A	4
#define Y_OFS_B 35

	if (page == PAGE_ACTA)
		u8g_DrawXBMP(&u8g, X_OFS, Y_OFS_A, floppyar_width, floppyar_height, floppyar_bits);
	else if (page == PAGE_SELA)
		{
		u8g_DrawXBMP(&u8g, X_OFS, Y_OFS_A, floppya_width, floppya_height, floppya_bits);
		u8g_DrawFrame(&u8g, X_OFS-1, Y_OFS_A-1, X_OFS+floppya_width+1,Y_OFS_A+floppya_height+1);
		}
	else
		u8g_DrawXBMP(&u8g, X_OFS, Y_OFS_A, floppya_width, floppya_height, floppya_bits);

	u8g_SetFont(&u8g, u8g_font_6x10);
	u8g_SetFontPosTop(&u8g);	
	drawStr(40, Y_OFS_A+1, pDrive[0].fName);
	diskinfo(&pDrive[0]); //generate disk CHS info
	drawStr(40, Y_OFS_A+14, infostring); //use itoabuf (defined in simpleUART)

#ifdef ENABLE_DRIVE_B
	if (page == PAGE_ACTB)
		u8g_DrawXBMP(&u8g, X_OFS, Y_OFS_B, floppybr_width, floppybr_height, floppybr_bits);
	else if (page == PAGE_SELB)
		{
		u8g_DrawXBMP(&u8g, X_OFS, Y_OFS_B, floppyb_width, floppyb_height, floppyb_bits);
		u8g_DrawFrame(&u8g, X_OFS-1, Y_OFS_B-1, X_OFS+floppyb_width+1,Y_OFS_B+floppyb_height+1);
		}
	else
		u8g_DrawXBMP(&u8g, X_OFS, Y_OFS_B, floppyb_width, floppyb_height, floppyb_bits);

	drawStr(40, Y_OFS_B+1, pDrive[1].fName);
	diskinfo(&pDrive[1]); //generate disk CHS info
	drawStr(40, Y_OFS_B+14, infostring);
#endif //ENABLE_DRIVE_B
}

void FDISPLAY::noticeScreen()
{
	u8g_DrawXBMP(&u8g, 0, 0, caution_width, caution_height, caution_bits);
	u8g_SetFont(&u8g, u8g_font_6x10);
	u8g_SetFontPosTop(&u8g);	
	drawStrP(50, 10, notice_header);
	drawStrP(0+0, 30, notice_message);
	//drawStrP(0+0, 45, PSTR("on fdd emulation"));
}

void FDISPLAY::splashScreen()
{
	u8g_DrawXBMP(&u8g, 1, 5, floppy_width, floppy_height, floppy_bits);
	u8g_SetFont(&u8g, u8g_font_6x10);
	u8g_SetFontPosTop(&u8g);	
	drawStrP(50+10, 10, PSTR("(c) 2021"));
	drawStrP(50+20, 25, PSTR("Acemi"));
	drawStrP(50+5, 40, PSTR("Elektronikci"));
}

void FDISPLAY::init()
{
	u8g_SetPinInput(PN(2,5)); u8g_SetPinLevel(PN(2,5), 1); u8g_SetPinOutput(PN(2,5));
	u8g_SetPinInput(PN(2,4)); u8g_SetPinLevel(PN(2,4), 1); u8g_SetPinOutput(PN(2,4));
	u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x64_2x_i2c, U8G_I2C_OPT_NONE);
#ifdef FLIP_SCREEN
	u8g_SetRot180(&u8g);
#endif //FLIP_SCREEN	
}

FDISPLAY::FDISPLAY()
{
init();
notice_timer = NOTICE_TIMEOUT;
sleep_timer = SLEEP_TIMEOUT;
menu_sel = 0;
}

void FDISPLAY::drawPage()
{
switch(page)
	{
	case PAGE_SPLASH:		
		splashScreen();
		break;
	case PAGE_NSEL:
	case PAGE_SELA:
	case PAGE_SELB:
	case PAGE_ACTA:
	case PAGE_ACTB: 
		statusScreen();
		break;
	case PAGE_NOTC:
		noticeScreen();
		break;
	case PAGE_MENU:
		drawMenu();
		break;
	}
}

void FDISPLAY::setPage(uint8_t r_page)
{
	page = r_page;
	Serial.print("Page timer: ");
	Serial.printDEC(sleep_timer);
	Serial.write('\n');
	if (sleep_timer == 0) FDISPLAY::wakeup();
	sleep_timer = SLEEP_TIMEOUT; //reset sleep timer
}

void FDISPLAY::showNoticeP(const char *header, const char *message)
{
	notice_header = header;
	notice_message = message;
	FDISPLAY::setPage(PAGE_NOTC);
	notice_timer = NOTICE_TIMEOUT;
	idle_timer = 0; //show ASAP
}

void FDISPLAY::setDriveActive(uint8_t drive)
{
	if (drive == 0) return;
#ifdef ENABLE_DRIVE_B	
	else if (drive == 2) 
		setPage(PAGE_ACTB);
	else if (drive == 1)
#endif //ENABLE_DRIVE_B	
		setPage(PAGE_ACTA);
	idle_timer = 0; //update screen ASAP
	update();
}

void FDISPLAY::update()
{
if (idle_timer == 0)
	{
	if (notice_timer)
	{
		notice_timer --;
		if (notice_timer == 0) FDISPLAY::setPage(PAGE_NSEL); //return to status page
		sleep_timer = SLEEP_TIMEOUT; //reset sleep timer
	}
	if (sleep_timer)
	{
		sleep_timer --;
		if (sleep_timer == 0) FDISPLAY::sleep();
	}
	//update page
	u8g_FirstPage(&u8g);
    do	{
      	drawPage();
    	} while ( u8g_NextPage(&u8g) );
	}
	idle_timer++;
}
