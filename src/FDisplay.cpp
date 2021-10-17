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
#include "simpleUART.h" //debug
#include "DiskFile.h"
#include <string.h>
#include <stdlib.h>

class FDISPLAY disp; //will use as extern


char *diskinfo(uint8_t r_drive)	//Generate disk CHS info string in itoabuf (defined in simpleUART)
{
	static char infostring[12]; //drive C/H/S info string
	char convbuf[4];
	
	if (drive[r_drive].fName[0] == 0)
	{
		strcpy_P(infostring, str_nodisk);
		return infostring;
	}		
	infostring[0] = 'C';
	infostring[1] = 0;
	itoa(drive[r_drive].numTrack, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
	strcat(infostring, "H2S");
	itoa(drive[r_drive].numSec, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
	return infostring;
}

//https://github.com/olikraus/u8glib/blob/master/sys/arm/examples/menu/menu.c
void FDISPLAY::drawMenu(void) 
{
	uint8_t i, h;
	u8g_uint_t w, d;	

	//u8g_SetFont(&u8g, u8g_font_6x10);
	u8g_SetFontRefHeightText(&u8g);
	//u8g_SetFontPosTop(&u8g);
  	h = u8g_GetFontAscent(&u8g)-u8g_GetFontDescent(&u8g);
	w = u8g_GetWidth(&u8g);
	for( i = 0; i < menu_max; i++ ) 
	{
    	d = (w-getStrWidth(menuFileNames[i]))/2;
    	u8g_SetDefaultForegroundColor(&u8g);
    	if ( i == menu_sel ) 
    	{			
      		u8g_DrawBox(&u8g, 0, i*h+1, w, h);
      		u8g_SetDefaultBackgroundColor(&u8g);
    	}
    	u8g_DrawStr(&u8g, d, i*h, menuFileNames[i]);	
  	}
  u8g_SetDefaultForegroundColor(&u8g); //set color back to foreground color
}

void FDISPLAY::statusScreen()
{	
#define X_OFS	1
#define Y_OFS_A	3
#define Y_OFS_B 33
	
	u8g_uint_t w = u8g_GetWidth(&u8g);
	//Draw drive0		
	u8g_SetDefaultForegroundColor(&u8g); //set color back to foreground color
	if (isDrive0() ) 	
	{
		drawXBMP(X_OFS, Y_OFS_A, floppym_width, floppym_height, floppya_bits);	
		u8g_DrawFrame(&u8g, X_OFS-1, Y_OFS_A-1, floppym_width+2,floppym_height+2);
		u8g_DrawBox(&u8g, X_OFS+floppym_width+2, Y_OFS_A-1, w-(X_OFS+floppym_width+2),floppym_height+2);
		u8g_SetDefaultBackgroundColor(&u8g);	//set color black
	}
	else drawXBMP(X_OFS, Y_OFS_A, floppym_width, floppym_height, floppya_bits);
	//Disk0 info	
	u8g_uint_t d = (w-floppym_width-getStrWidth(drive[0].fName))/2;
	drawStr(floppym_width+d, Y_OFS_A+1, drive[0].fName);		
	char *infostr = diskinfo(0);
	d = (w-floppym_width-getStrWidth(infostr))/2;
	drawStr(floppym_width+d, Y_OFS_A+14, infostr);
#if ENABLE_DRIVE_B
	//Draw drive1	
	u8g_SetDefaultForegroundColor(&u8g); //set color back to foreground color
	if (isDrive1() ) 
	{
		drawXBMP(X_OFS, Y_OFS_B, floppym_width, floppym_height, floppyb_bits);
		u8g_DrawFrame(&u8g, X_OFS-1, Y_OFS_B-1, floppym_width+2,floppym_height+2);
		u8g_DrawBox(&u8g, X_OFS+floppym_width+2, Y_OFS_B-1, w-(X_OFS+floppym_width+2),floppym_height+2);
		u8g_SetDefaultBackgroundColor(&u8g);	//set color black
	}			
	else drawXBMP(X_OFS, Y_OFS_B, floppym_width, floppym_height, floppyb_bits);
	//Disk1 info
	d = (w-floppym_width-getStrWidth(drive[1].name))/2;	
	drawStr(floppym_width+d, Y_OFS_B+1, drive[1].fName);	
	infostr = diskinfo(1);
	d = (w-floppym_width-getStrWidth(infostr))/2;
	drawStr(floppym_width+d, Y_OFS_B+14, infostr);
	u8g_SetDefaultForegroundColor(&u8g); //set color back to foreground color
#endif //ENABLE_DRIVE_B
}

void FDISPLAY::loadingScreen()
{
	u8g_uint_t w = u8g_GetWidth(&u8g);
	u8g_uint_t d = (w-getStrWidthP(str_loading))/2;

	if (isDrive0() )
		drawXBMP( (w - floppym_width)/2, 0, floppym_width, floppym_height, floppya_bits);
	else if (isDrive1() )	
		drawXBMP( (w - floppym_width)/2, 0, floppym_width, floppym_height, floppyb_bits);

	drawStrP(floppym_height + 5, d, str_loading);	
	d = (w-getStrWidth(menuFileNames[menu_sel]))/2;
	drawStr(floppym_height + 20, d, menuFileNames[menu_sel]);	
}

void FDISPLAY::busyScreen()
{
	u8g_uint_t w = u8g_GetWidth(&u8g);
	u8g_uint_t d = (w-getStrWidthP(str_busy))/2;

	if (isDrive0() )
		drawXBMP( (w - floppym_width)/2, 0, floppym_width, floppym_height, floppya_bits);
	else if (isDrive1() )	
		drawXBMP ( (w - floppym_width)/2, 0, floppym_width, floppym_height, floppyb_bits);

	drawStrP(floppym_height + 5, d, str_busy);	
	d = (w-getStrWidth(drive[getSelectedDrive()-1].fName))/2;
	drawStr(floppym_height + 20, d, drive[getSelectedDrive()-1].fName);	
}

void FDISPLAY::noticeScreen()
{
	u8g_uint_t w = u8g_GetWidth(&u8g);
	u8g_uint_t d = (w-getStrWidthP(notice_header))/2;

	drawXBMP( (w - caution_width)/2, 0, caution_width, caution_height, caution_bits);

	drawStrP(caution_height + 5, d, notice_header);
	d = (w-getStrWidthP(notice_message))/2;
	drawStrP(caution_height + 20, d, notice_message);	
	//Output notice to serial
	Serial.print_P(notice_header); \
	Serial.print_P(str_colon); \
	Serial.print_P(notice_message);
	Serial.write('\n');
}

void FDISPLAY::splashScreen()
{
	u8g_uint_t w = u8g_GetWidth(&u8g);
	u8g_uint_t d = (w-floppy_width-getStrWidthP(str_2021))/2;

	drawXBMP(1, 5, floppy_width, floppy_height, floppy_bits);
	drawStrP(floppy_width+d, 10, str_2021);
	d = (w-floppy_width-getStrWidthP(str_acemi))/2;
	drawStrP(floppy_width+d, 25, str_acemi);
	d = (w-floppy_width-getStrWidthP(str_elektron))/2;
	drawStrP(floppy_width+d, 40, str_elektron);
}

void FDISPLAY::init()
{
	u8g_SetPinInput(PN(2,5)); u8g_SetPinLevel(PN(2,5), 1); u8g_SetPinOutput(PN(2,5));
	u8g_SetPinInput(PN(2,4)); u8g_SetPinLevel(PN(2,4), 1); u8g_SetPinOutput(PN(2,4));
	u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x64_2x_i2c, U8G_I2C_OPT_NONE);
#if FLIP_SCREEN
	u8g_SetRot180(&u8g);
#endif //FLIP_SCREEN
	u8g_SetFont(&u8g, u8g_font_6x10);	//select font
	u8g_SetFontPosTop(&u8g);	//set font position
}

void FDISPLAY::showNoticeP(const char *header, const char *message)
{
	notice_header = header;
	notice_message = message;
	setPage(PAGE_NOTICE);
	notice_timer = NOTICE_TIMEOUT;
	idle_timer = 0; //show ASAP
	update();
}

void FDISPLAY::showDriveIdle()
{	
	selectDrive(0);	
	setPage(PAGE_STATUS);
	idle_timer = 0; //update screen ASAP
	update();
}

void FDISPLAY::showDriveBusy(uint8_t r_drive)
{	
	selectDrive(r_drive);	
	setPage(PAGE_BUSY);
	idle_timer = 0; //update screen ASAP
	update();
}

void FDISPLAY::showDriveLoading()
{		
	setPage(PAGE_LOADING);
	idle_timer = 0; //update screen ASAP
	update();
}

void FDISPLAY::setPage(uint8_t r_page)
{	
	page = r_page; //set requested page
	if (sleep_timer == 0)
	{
		sleep_timer = SLEEP_TIMEOUT; //reset sleep timer
		FDISPLAY::sleepOff();
	#if DEBUG	
		Serial.print(F("Screen wakeup\n"));		
	#endif //DEBUG
		showDriveIdle();
	}	
	sleep_timer = SLEEP_TIMEOUT; //reset sleep timer
}

FDISPLAY::FDISPLAY()
{
init();
notice_timer = NOTICE_TIMEOUT;
sleep_timer = SLEEP_TIMEOUT;
menu_sel = 0;
setPage(PAGE_SPLASH);
}

void FDISPLAY::drawPage()
{
switch(page)
	{
		case PAGE_STATUS:
			statusScreen();
			break;
		case PAGE_SPLASH:		
			splashScreen();
			break;
		case PAGE_BUSY:
			busyScreen();
			break;
		case PAGE_LOADING:
			loadingScreen();
			break;		
		case PAGE_NOTICE:
			noticeScreen();
			break;
		case PAGE_MENU:
			drawMenu();
			break;				
		default:
			statusScreen();	
	}
}

void FDISPLAY::update()
{
	if (idle_timer == 0)
	{

		if (sleep_timer)
		{
			if (notice_timer)
			{
				notice_timer --;
				sleep_timer = SLEEP_TIMEOUT; //reset sleep timer
				if (notice_timer == 0) setPage(PAGE_STATUS);
			}
			//update page
			u8g_FirstPage(&u8g);
    		do	{
    			drawPage();				
    		} while ( u8g_NextPage(&u8g) );

			sleep_timer --;
			if (sleep_timer == 0)
			{
				FDISPLAY::sleepOn();
			#if DEBUG
				Serial.print(F("Screen sleep\n"));
			#endif //DEBUG
			}
		}
	}
	idle_timer++;
}
