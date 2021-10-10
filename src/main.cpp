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


#include "FloppyDrive.h"
#include "simpleUART.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include "pff.h"
#include "analog_read.h"
#include "FDisplay.h"
#include "constStrings.h"
#include "FDDpins.h"


#define ADC_PIN 7 //ADC7

int16_t nItems = 0;  //required for menu system
int16_t idx_sel = 0; //where were we in the menu index ?


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
    if (fno.fattrib & (AM_VOL | AM_LFN) );//skip
    else if (fno.fattrib & AM_DIR) //skip
    {
    #if DEBUG  
      Serial.print(path);
      Serial.write('[');
      Serial.print(fno.fname);                
      Serial.write(']');
      Serial.write('\n');
    #endif  
    } 
    else //regular file
    {
      if (fno.fname[0] != 0) n_files++;
    #if DEBUG  
      Serial.print(path);
      Serial.write('/');
      Serial.print(fno.fname);                
      Serial.write('\n');
    #endif  
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
  else if ( (adcval >= 800) && (adcval < 870) ) newval = 5;
  else if ( (adcval >= 600) && (adcval < 660) ) newval = 4;
  else if ( (adcval >= 400) && (adcval < 450) ) newval = 3;
  else if ( (adcval >= 190) && (adcval < 230) ) newval = 2;
  else if (adcval == 0)  newval = 1;
  
  if  ( (prevval == 6) && (lastval == newval) ) //if dropped from max && stable value        
  {
    prevval = lastval;
    if (newval < 6) 
    {      
    #if DEBUG == 1
      Serial.print(F("Button: "));
      Serial.printHEX(newval);
      Serial.print(F(" value: "));
      Serial.printDEC(adcval);
      Serial.write('\n');
    #endif //DEBUG
      return newval;  
    }
  }
  prevval = lastval; 
  lastval = newval;  
  return 0;
}

void loadMenuStrings()
{    
  DIR dir;
  FILINFO fno;
  int8_t menu_max = MENU_ITEMS;

  if (nItems < menu_max) menu_max = nItems;
  //Limit menu selection
  if (disp.menu_sel < 0) 
	  {
	  disp.menu_sel = 0;
	  idx_sel--;
	  }
  else if (disp.menu_sel >= menu_max) 
	{
	  disp.menu_sel = menu_max - 1;
	  idx_sel++;
	}
  //Limit index selection  
  if (idx_sel > (nItems - MENU_ITEMS)) idx_sel = nItems - MENU_ITEMS;
  else if (idx_sel < 0) idx_sel = 0;
  //load menu strings
    get_first_file(&dir, &fno, (char *)s_diskdir);
  for (int16_t i=0; i < idx_sel; i++) get_next_file(&dir, &fno); //skip some files
  for (int16_t i=0; (i < MENU_ITEMS) && (fno.fname[0] != 0); i++)   
  {
    memcpy(disp.menu_strings[i], fno.fname, 13);
    get_next_file(&dir, &fno);
  }   
  if (nItems == 0) strcpy_P(disp.menu_strings[0], str_nofile);  
}

void buttonAction(int button)
{
  if (button <= 0 ) //do nothing
    return;

  switch(button)
  {
    case  5:  //select drive     
      switch (disp.getSelectedDrive())
		  {
		    case 0:
          disp.selectDriveA();			    
			    Serial.print(F("Sel drive: A\n"));	          
			    break;
      #if ENABLE_DRIVE_B
		    case 1:          
			    disp.selectDriveB();          
			    Serial.print(F("Sel drive: B\n"));          
			    break;
      #endif //ENABLE_DRIVE_B  
		    default:
			    disp.setDriveIdle();
			    Serial.print(F("Sel drive: None\n"));          
		  }
      break;
    case  4:  //Next file
      if (disp.getPage() == PAGE_MENU)
      {
        disp.menu_sel++;
        loadMenuStrings();
        //Output to serial  
        Serial.print_P(str_selected);
        Serial.print(disp.menu_strings[disp.menu_sel]);
        Serial.write('\n');  
      }
		  else 
			{
			disp.menu_sel = 0;
      idx_sel = 0;
			if ( disp.getSelectedDrive() ) //if a drive selected
        disp.setPage(PAGE_MENU);
        loadMenuStrings();
			}		
    break;
  case  3:  //Previous file
    if (disp.getPage() == PAGE_MENU)
    {
      disp.menu_sel--;
      loadMenuStrings();
      //Output to serial  
      Serial.print_P(str_selected);
      Serial.print(disp.menu_strings[disp.menu_sel]);
      Serial.write('\n');  
    }
		else 
			{
			disp.menu_sel = 0;
      idx_sel = 0;
			if ( disp.getSelectedDrive() ) //if a drive selected
        disp.setPage(PAGE_MENU);
        loadMenuStrings();
			}		
		break;
  case  2:  //load disk    
    if (disp.getPage() != PAGE_MENU) break; //if we are not in menu disable load
    loadMenuStrings();
		Serial.print_P(str_loading);
		Serial.print(disp.menu_strings[disp.menu_sel]);
		Serial.write('\n');		
    if (disp.getSelectedDrive() == DRIVEA_SELECT) driveA.load(disp.menu_strings[disp.menu_sel]); 
  #if ENABLE_DRIVE_B
    else if (disp.getSelectedDrive() == DRIVEB_SELECT) driveB.load(disp.menu_strings[disp.menu_sel]);   
  #endif //ENABLE_DRIVE_B  
    disp.setDriveIdle();
		break;		
  case  1:  //eject disk
    if (disp.getPage() == PAGE_MENU)  //behave as cancel
    {
      disp.setDriveIdle();
      Serial.print(str_cancel);
    }
    else
    {
      uint8_t s_drive = disp.getSelectedDrive();
      if (s_drive) //behave as eject
      {
        Serial.print_P(str_eject);
        if (s_drive == DRIVEA_SELECT)
        {
          Serial.write('A');
          driveA.eject();
        }
        else if (s_drive == DRIVEB_SELECT)
        {          
          Serial.write('B');
        #if ENABLE_DRIVE_B
          driveB.eject();
        #endif  
        }
        Serial.write('\n');
      }
    }
    disp.setPage(PAGE_STATUS);
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
      buttonAction(3);
		  break;
	  case 'n':
	  case 'N':
      buttonAction(4);
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
  wdt_enable(WDTO_8S);  
  Serial.init(115200);
  Serial.print_P(str_intro);
  Serial.print_P(str_usage);
  driveA.load((char *)s_bootfile);   //if there is "BOOT.IMG" on SD load it
  nItems = scan_files((char *)s_diskdir); //get number of files on SD
  init_ADC(); //prep ADC
  reqADC(ADC_PIN); //request ADC reading on ADC_PIN  
   
  for(;;)
  {   
    wdt_reset();
    if (drvSel == DRIVEA_SELECT) 
    {
      disp.setDriveBusy(drvSel);
      driveA.run();
      disp.setDriveIdle();
    }    
  #if ENABLE_DRIVE_B
    else if (drvSel == DRIVEB_SELECT)
    {
      disp.setDriveBusy(drvSel);
      driveB.run();
      disp.setDriveIdle();    
    }
  #endif //ENABLE_DRIVE_B  
    if (adcReady) buttonAction(adcButton());    
	  if (rxReady) readRx();
	  disp.update();
  }
}