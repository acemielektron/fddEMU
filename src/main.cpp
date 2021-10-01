// -----------------------------------------------------------------------------
// fddEMU "Floppy Disk Drive Emulator"
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


#include "FDDcommon.h"
#include "FloppyDrive.h"
#include "simpleUART.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include "pff.h"
#include "analog_read.h"
#include "FDisplay.h"
#include "constStrings.h"


#define ADC_PIN 7 //ADC7

int16_t nItems = 0;  //required for menu system
extern class FloppyDrive *pDrive;


int scan_files (char* path)
{
  FRESULT res;
  FILINFO fno;
  DIR dir;
  int n_files = 0;
    
  res = pf_opendir(&dir, path);
  if (res) return -1;

  do 
  {
    wdt_reset();
    res = pf_readdir(&dir, &fno);
    if (res != FR_OK) break;
    if (fno.fattrib & AM_DIR) 
    {
      Serial.print(path);
      Serial.write('[');
      Serial.print(fno.fname);                
      Serial.write(']');
      Serial.write('\n');
    } 
    else 
    {
      n_files++;
      Serial.print(path);
      Serial.write('/');
      Serial.print(fno.fname);                
      Serial.write('\n');
    }     
  } while (fno.fname[0] != 0);
  return n_files;
}

 FRESULT get_first_file(DIR *pdir, FILINFO *pfno, char *path)
{
  FRESULT res;
  
  res = pf_opendir(pdir, path);
  if (res == FR_OK)
  {
    res = pf_readdir(pdir, pfno);
    if (pfno->fname[0] == 0) res = FR_NO_FILE;    
  }
  return res;  
}

FRESULT get_next_file(DIR *pdir, FILINFO *pfno)
{
  FRESULT res;
  
  do{
    res = pf_readdir(pdir, pfno);
    }while((pfno->fattrib & AM_DIR) && (pfno->fname[0] != 0)); //if file is directory skip
  if (pfno->fname[0] == 0) res = FR_NO_FILE;    
  return res;  
}

int adcButton()
{
  static uint8_t prevval = 6; 
  static uint8_t lastval = 6;  
  uint8_t newval = 7;
  uint16_t adcval;

  if (adcBusy)  return 0;  
  adcval = readADC();
  reqADC(ADC_PIN);
  if (adcval > 900) newval = 6;
  else if ( (adcval >= 840) && (adcval < 870) ) newval = 5;
  else if ( (adcval >= 630) && (adcval < 660) ) newval = 4;
  else if ( (adcval >= 420) && (adcval < 450) ) newval = 3;
  else if ( (adcval >= 190) && (adcval < 220) ) newval = 2;
  else if (adcval == 0)  newval = 1;
  
  if  ( (prevval == 6) && (lastval == newval) ) //if dropped from max && stable value        
  {
    prevval = lastval;
    if (newval < 6) 
    {      
      #ifdef DEBUG
      Serial.print(F("Button: "));
      Serial.printHEX(newval);
      Serial.print(F(" value: "));
      Serial.printDEC(adcval);
      Serial.write('\n');
      #endif
      return newval;  
    }
  }
  prevval = lastval; 
  lastval = newval;  
  return 0;
}

void loadMenuStrings()
{  
  static int16_t idx_sel = 0; //where were we in the index ?
  DIR dir;
  FILINFO fno;
  int i = 0;

  //Limit menu selection
  if (disp.menu_sel < 0) 
	  {
	  disp.menu_sel = 0;
	  idx_sel--;
	  }
  else if (disp.menu_sel >= MENU_ITEMS) 
	{
	  disp.menu_sel = MENU_ITEMS - 1;
	  idx_sel++;
	}
  else if (disp.menu_sel >= nItems) disp.menu_sel = nItems - 1;
  //Limit index selection
  if (idx_sel < 0) idx_sel = 0;
  else if (idx_sel > (nItems - MENU_ITEMS)) idx_sel = nItems - MENU_ITEMS;
  //load menu strings
  get_first_file(&dir, &fno, (char *)DISK_DIR);
  for (int i=1; i < idx_sel; i++) get_next_file(&dir, &fno); //skip some files
  //for (int i=0; i < MENU_ITEMS && i < nItems; i++)
  while(fno.fname[0] != 0)
  {
    strcpy(disp.menu_strings[i], fno.fname);
    get_next_file(&dir, &fno);
    i++;
  }   
  if (nItems == 0) strcpy_P(disp.menu_strings[0], PSTR("NO FILE"));
  //Output to serial
  Serial.print(F("Selected "));
  Serial.print(disp.menu_strings[disp.menu_sel]);
  Serial.write('\n');
}

void buttonAction(int button)
{
  if (button <= 0 ) //do nothing
    return;

  switch(button)
  {
    static uint8_t lastDrive = 0;
    case  5:  //select drive
      switch (disp.getPage())
		  {
		    case PAGE_NSEL:
			    disp.setPage(PAGE_SELA);
			    Serial.print(F("Sel drive: A\n"));	
          lastDrive = 1;		
			    break;
      #ifdef   ENABLE_DRIVE_B
		    case PAGE_SELA:
			    disp.setPage(PAGE_SELB);
			    Serial.print(F("Sel drive: B\n"));
          lastDrive = 2;
			    break;
      #endif //ENABLE_DRIVE_B  
		    default:
			    disp.setPage(PAGE_NSEL);
			    Serial.print(F("Sel drive: None\n"));
          lastDrive = 0;
		  }
      break;
    case  4:  //Previous file
      if (disp.getPage() == PAGE_MENU) disp.menu_sel--;
		  else 
			{
			disp.menu_sel = 0;
			if ( (disp.getPage() == PAGE_SELA) || (disp.getPage() == PAGE_SELB) ) //if a drive selected
        disp.setPage(PAGE_MENU);
			}
		loadMenuStrings();
    break;
  case  3:  //Next file
    if (disp.getPage() == PAGE_MENU) disp.menu_sel++;
		else 
			{
			disp.menu_sel = 0;
			if ( (disp.getPage() == PAGE_SELA) || (disp.getPage() == PAGE_SELB) ) //if a drive selected
        disp.setPage(PAGE_MENU);
			}
		loadMenuStrings();
		break;
  case  2:  //load disk    
    if (disp.getPage() != PAGE_MENU) break; //if we are not in menu disable load
		Serial.print_P(str_loading);
		Serial.print(disp.menu_strings[disp.menu_sel]);
		Serial.write('\n');
		disp.setPage(PAGE_NSEL);
    pDrive[lastDrive - 1].loadDisk(disp.menu_strings[disp.menu_sel]); 
		break;		
  case  1:  //eject disk
    switch(disp.getPage())
    {
      case PAGE_MENU: //behave as cancel
        disp.setPage(PAGE_NSEL);
        Serial.print(str_cancel);
        break;
      case PAGE_SELA: //behave as eject
        pDrive[0].ejectDisk();      
        Serial.print_P(str_eject);
      case PAGE_SELB:
        pDrive[1].ejectDisk();      
        Serial.print_P(str_eject);
    }
    break;
  }  
}

void readRx()
{
char ch = Serial.read();

switch(ch)
	{
    case 'S':
    case 's':
      buttonAction(5);
      break;
	  case 'p':
	  case 'P':
      buttonAction(4);
		  break;
	  case 'n':
	  case 'N':
      buttonAction(3);
		  break;      
	  case 'l':	//OK
	  case 'L':
      buttonAction(2);
		  break;		
	  case 'e':	//Cancel
	  case 'E':
      buttonAction(1);
		  break;
	  case 'x':
	  case 'X':
		  disp.showNoticeP(errHDR, err_test);
	}
}

int main(void)
{ 
  wdt_enable(WDTO_1S);  
  Serial.init(115200);
  Serial.print(F("\nfddEMU (c) 2001 Acemi Elektronikci\n"));
  Serial.print(F("\nS: Select drive\nP: Previous\nN: Next\nL: Load\nE: Eject\n\n"));
  pDrive[0].loadDisk((char *)"BOOT.IMG");   //if there is "BOOT.IMG" on SD load it
  nItems = scan_files((char *)DISK_DIR); //get number of files on SD
  init_ADC(); //prep ADC
  reqADC(ADC_PIN); //request ADC reading on ADC_PIN
   
  for(;;)
  {   
    wdt_reset();
    FDDloop();
    if (adcReady) buttonAction(adcButton());    
	  if (rxReady) readRx();
	  disp.update();
  }
}
