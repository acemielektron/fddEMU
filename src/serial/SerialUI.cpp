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

#include "fddEMU.h"
#include "SerialUI.h"
#include "FloppyDrive.h" //drive
#include "DiskFile.h" //sdfile

#if ENABLE_SERIAL
  class SerialUI ser;
#endif //ENABLE_SERIAL

SerialUI::SerialUI(void)
{
    drv_sel=0;
    file_index = 0;    
}

void SerialUI::readRx()
{
  char ch = Serial.read();

  switch(ch)
	{
        case 'S': 
        case 's':           
            if ( (drv_sel == 0) || (drv_sel == DRIVE1) ) drv_sel = DRIVE0;
        #if ENABLE_DRIVE_B  
            else if (drv_sel == DRIVE0) drv_sel = DRIVE1;
        #endif //ENABLE_DRIVE_B  
            statusInfo();
            Serial.print_P(str_selected);
            Serial.print_P(str_drive);
            if (drv_sel == DRIVE0) Serial.write('A');
            else if (drv_sel== DRIVE1) Serial.write('B');
            Serial.write('\n');               
            break;
	    case 'p':
	    case 'P':
            file_index -=2;  		  
	    case 'n':
	    case 'N':
            file_index++;
            if (file_index < 0) file_index = 0;
            if (file_index >= sdfile.nFiles) file_index=sdfile.nFiles;      
            sdfile.openDir((char *)s_RootDir); //rewind directory
            for (int16_t i = 0; i <= file_index; i++) sdfile.getNextFile(); //skip files                        
            Serial.write('>'); 
            Serial.print_P(str_selected);      
            if (sdfile.getFileName()[0] != 0) Serial.print(sdfile.getFileName());
            else Serial.print_P(str_label);                        
            for (int8_t k=0; k < 14; k++) Serial.write(' '); //clear current row in console
            Serial.write('\r'); // return start of the same row  in console    
		    break;      
	    case 'l':	//Load
	    case 'L':
            if (drv_sel)
            {
                char filename[13];
                
                memcpy(filename, sdfile.getFileName(), 13);
                Serial.write('>'); 
                Serial.print_P(str_loading);
                if (filename[0] != 0) Serial.print(filename);
                else Serial.print_P(str_label);
                for (int8_t k=0; k < 14; k++) Serial.write(' '); //clear current row in console
                Serial.write('\n');              
                if (filename[0] != 0) drive[drv_sel -1].load(filename);
                else drive[drv_sel -1].loadVirtualDisk();
                Serial.print(drive[drv_sel -1].diskInfoStr());
                Serial.write('\n');
            }
		    break;		
	    case 'e':	//Cancel
	    case 'E':
            if (drv_sel) 
            {
                Serial.print_P(str_eject);
                (drv_sel == DRIVE0) ? Serial.write('A'):Serial.write('B');
                Serial.write('\n');
                drive[drv_sel -1].eject();
            }
		    break; 
    #if ENABLE_GUI                   
	    case 'x':
	    case 'X':              
		    disp.showNoticeP(str_error, err_test);        
        break;          
      default:    
          
        //send keypresses 1-5 to button interface      
        if ( (ch > '0') && (ch < '6') ) disp.buttonAction(ch - '0');
    #endif //ENABLE_GUI    
	}
}

void SerialUI::intro()
{
  Serial.write('\n');
  Serial.write('\n');
  Serial.print_P(str_fddEMU);
  Serial.write(' ');
  Serial.print_P(str_2021);
  Serial.write(' ');
  Serial.print_P(str_acemi);
  Serial.write(' ');
  Serial.print_P(str_elektron);
  Serial.write('\n');
  Serial.write('\n');
  Serial.print_P(str_usage);    
  Serial.write('\n');
  Serial.write('\n');
}

void SerialUI::statusInfo()
{
  Serial.write('\n');    
  Serial.write('A');
  Serial.print_P(str_colon);
  Serial.print(drive[0].fName);
  Serial.write('\t');
  Serial.print(drive[0].diskInfoStr());
  Serial.write('\n');    
#if ENABLE_DRIVE_B      
  Serial.write('B');
  Serial.print_P(str_colon);
  Serial.print(drive[1].fName);
  Serial.write('\t');
  Serial.print(drive[1].diskInfoStr());
  Serial.write('\n');    
#endif //ENABLE_DRIVE_B  
}
