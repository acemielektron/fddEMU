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

#include "SerialUI.h"
#include "constStrings.h"
#include "FloppyDisk.h" //diskinfo
#include "FloppyDrive.h" //drive
#include "DiskFile.h" //sdfile
#include "FDisplay.h" //disp


#if ENABLE_SERIAL || DEBUG
class SerialUI Serial;
#endif //ENABLE_SERIAL || DEBUG

SerialUI::SerialUI(void)
{
    drv_sel=0;
    file_index = 0;    
}

void SerialUI::readRx()
{
  char ch = SerialUI::read();

  switch(ch)
	{
        case 'S': 
        case 's':           
            if ( (drv_sel == 0) || (drv_sel == DRIVE1) ) drv_sel = DRIVE0;
        #if ENABLE_DRIVE_B  
            else if (drv_sel == DRIVE0) drv_sel = DRIVE1;
        #endif //ENABLE_DRIVE_B  
            statusInfo();
            SerialUI::print_P(str_selected);
            SerialUI::print_P(str_drive);
            if (drv_sel == DRIVE0) SerialUI::write('A');
            else if (drv_sel== DRIVE1) SerialUI::write('B');
            SerialUI::write('\n');               
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
            SerialUI::print_P(str_selected);      
            if (sdfile.getFileName()[0] != 0) SerialUI::print(sdfile.getFileName());
            else SerialUI::print_P(str_label);
            SerialUI::write('\n');  
		    break;      
	    case 'l':	//Load
	    case 'L':
            if (drv_sel)
            {
                char filename[13];
                
                memcpy(filename, sdfile.getFileName(), 13);
                SerialUI::print_P(str_loading);
                if (filename[0] != 0) SerialUI::print(filename);
                else SerialUI::print_P(str_label);
                SerialUI::write('\n');              
                if (filename[0] != 0) drive[drv_sel -1].load(filename);
                else drive[drv_sel -1].loadVirtualDisk();
                SerialUI::print(diskinfo(drv_sel -1));
                SerialUI::write('\n');
            }
		    break;		
	    case 'e':	//Cancel
	    case 'E':
            if (drv_sel) 
            {
                SerialUI::print_P(str_eject);
                if (drv_sel == DRIVE0) SerialUI::write('A');
                else SerialUI::write('B');
                SerialUI::write('\n');
                drive[drv_sel -1].eject();
            }
		    break;                  
	    case 'x':
	    case 'X':      
        #if ENABLE_GUI && DEBUG 
		    disp.showNoticeP(errHDR, err_test);
        #endif //ENABLE_GUI && DEBUG 
            break;          
        default:    
            #if ENABLE_GUI    
            //send keypresses 1-5 to button interface      
            if ( (ch > '0') && (ch < '6') ) disp.buttonAction(ch);
        #endif //ENABLE_GUI  
            break;
	}
}

void SerialUI::intro()
{
  SerialUI::write('\n');
  SerialUI::write('\n');
  SerialUI::print_P(str_fddEMU);
  SerialUI::write(' ');
  SerialUI::print_P(str_2021);
  SerialUI::write(' ');
  SerialUI::print_P(str_acemi);
  SerialUI::write(' ');
  SerialUI::print_P(str_elektron);
  SerialUI::write('\n');
  SerialUI::write('\n');
  SerialUI::print_P(str_usage);    
  SerialUI::write('\n');
  SerialUI::write('\n');
}

void SerialUI::statusInfo()
{
  SerialUI::write('\n');    
  SerialUI::write('A');
  SerialUI::print_P(str_colon);
  SerialUI::print(drive[0].fName);
  SerialUI::write('\t');
  SerialUI::print(diskinfo(0));
  SerialUI::write('\n');    
#if ENABLE_DRIVE_B      
  SerialUI::write('B');
  SerialUI::print_P(str_colon);
  SerialUI::print(drive[1].fName);
  SerialUI::write('\t');
  SerialUI::print(diskinfo(1));
  SerialUI::write('\n');    
#endif //ENABLE_DRIVE_B  
}
