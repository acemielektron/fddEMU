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
#include "FloppyDrive.h"
#include "pff.h"
#include "diskio.h"
#include "avrFlux.h"
#include "VirtualFloppyFS.h"
#include "simpleUART.h" //DEBUG
#include "UINotice.h" //msg.error

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h> //for strcat,strcpy,...
#include <stdlib.h> //for itoa


//Global variables
bool pinsInitialized = false;
static uint8_t dataBuffer[516];
volatile int iTrack = 0;
volatile uint8_t iFlags = 0;

class FloppyDrive drive[N_DRIVE]; //will be used as extern

//Interrupt routines
#if defined (__AVR_ATmega328P__)
ISR(INT0_vect) //int0 pin 2 of port D
#elif defined (__AVR_ATmega32U4__)
ISR(INT2_vect) //int2
#endif //(__AVR_ATmega32U4__)
{
  if (IS_STEP() ) //debounce
    (STEPDIR()) ? iTrack-- : iTrack++; // pointed out by hachi
  SET_TRACKCHANGED();  
}

//Two drive mode requires SELECT and MOTOR pins combined trough an OR gate
//if two drive mode is enabled SELECTA pin is used for combined SELECTA & MOTORA
//and MOTORA pin is used for combined SELECTB & MOTORB
#if defined (__AVR_ATmega328P__)
ISR(PCINT2_vect) //pin change interrupt of port D
#elif defined (__AVR_ATmega32U4__)
ISR(PCINT0_vect) //pin change interrupt of port B
#endif //(__AVR_ATmega32U4__)
{
#if ENABLE_DRIVE_B
  if ( IS_SELECTA() ) SET_DRIVE0(); //drive A is selected 
  else if ( IS_SELECTB() ) SET_DRIVE1(); //drive B is selected   
#else //Drive B not enabled
  if ( IS_SELECTA() && IS_MOTORA() ) SET_DRIVE0(); //driveA is selected 
#endif  //ENABLE_DRIVE_B
  else CLR_DRVSEL();
}

void initFDDpins()
{
  //To emulate open collector outputs Output pins are set to LOW "0"
  //To write a "1" to output pin, respective pin is set to input
  //To write a "0" to output pin, respective pin is set to output
  pinsInitialized = false;  
  cli(); //disable interrupts
#if defined (__AVR_ATmega328P__)
  //Setup Input and Output pins as Inputs
  DDRD &= 0b00000011; //D0 and D1 is RX & TX
  DDRB &= 0b11000000; //B6 & B7 is XTAL1 & XTAL2
  DDRC &= 0b11110000; //C7 is nil, C6 is RST, C4 & C5 is SDA & SCL
  //Assign Output pins LOW "0"
  PORTD &= ~bit(PIN_INDEX);
  PORTB &= ~bit(PIN_WRITEDATA);
  PORTC &= ~(bit(PIN_TRACK0)|bit(PIN_WRITEPROT)|bit(PIN_DSKCHANGE));
  //Assign Input pins HIGH "1" (Activate Pullups)
  PORTD |= bit(PIN_MOTORA)|bit(PIN_SELECTA);
  PORTD |= bit(PIN_STEP)|bit(PIN_STEPDIR)|bit(PIN_SIDE);
  PORTB |= bit(PIN_READDATA);
  PORTC |= bit(PIN_WRITEGATE);
  //Setup Pin Change Interrupts
  EICRA &=~(bit(ISC01)|bit(ISC00)); //clear ISC00&ISC01 bits
  EICRA |= bit(ISC01); //set ISC01 "falling edge"
  EIMSK |= bit(INT0); //External Interrupt Mask Register enable INT0  
  //Setup External Interrupt
  PCMSK2 = bit(PIN_SELECTA)| bit(PIN_MOTORA); // Pin Change Mask Register 2 enable SELECTA&MOTORA
  PCICR |= bit(PCIE2); // Pin Change Interrupt Control Register enable port D    
#elif defined (__AVR_ATmega32U4__)
  //Setup Input and Output pins as Inputs
  DDRB |= (1 << PIN_WRITEDATA); //set WRITEDATA as OUTPUT (Not sure it is necessary but datasheet says so)
  DDRB &= ~((1 << PIN_MOTORA)|(1 << PIN_SELECTA)); //PB0 RXLED, PB1 SCK, PB2 MOSI, PB3 MISO, PB4 MOTORA, PB5 OCP1, PB6 SELECTA
  DDRC &= ~(1 << PIN_SIDE); //PC6 SIDE
  DDRD &= ~((1 << PIN_STEP)|(1 << PIN_STEPDIR)|(1 << PIN_READDATA)|(1 << PIN_INDEX)|(1 << PIN_WRITEPROT)); //PD0 SCL, PD1 SDA, PD5 TXLED, PD2 STEP, PD3 STEPDIR, PD4 ICP1, PD7 INDEX
  DDRE &= ~(1 << PIN_WRITEGATE); //PE6 WRITEGATE
  DDRF &= ~((1 << PIN_TRACK0)|(1 << PIN_DSKCHANGE)); //PF4 TRACK0, PF5 WRITEPROT, PF6 DISKCHANGE, PF7 SS
  //Assign Output pins LOW "0"
  PORTB &= ~(1 << PIN_WRITEDATA);
  PORTD &= ~(1 << PIN_INDEX)|(1 << PIN_WRITEPROT);
  PORTF &= ~((1 << PIN_TRACK0)|(1 << PIN_DSKCHANGE));
  //Assign Input pins HIGH "1" (Activate Pullups)
  PORTB |= (1 << PIN_MOTORA)|(1 << PIN_SELECTA); //PB0 RXLED, PB1 SCK, PB2 MOSI, PB3 MISO, PB4 MOTORA, PB5 OCP1, PB6 SELECTA
  PORTC |= (1 << PIN_SIDE); //PC6 SIDE
  PORTD |= (1 << PIN_STEP)|(1 << PIN_STEPDIR)|(1 << PIN_READDATA); //PD0 SCL, PD1 SDA, PD5 TXLED, PD2 STEP, PD3 STEPDIR, PD4 ICP1, PD7 INDEX
  PORTE |= (1 << PIN_WRITEGATE); //PE6 WRITEGATE
  //Setup Pin Change Interrupts
  EICRA &=~(bit(ISC21)|bit(ISC20)); //clear ISC20&ISC21 bits
  EICRA |= bit(ISC21); //set ISC21 "falling edge"
  EIMSK |= bit(INT2); //External Interrupt Mask Register enable INT2
  //Setup External Interrupt
  PCMSK0 = bit(PIN_SELECTA)| bit(PIN_MOTORA); // Pin Change Mask Register 2 enable SELECTA&MOTORA
  PCICR |= bit(PCIE0); // Pin Change Interrupt Control Register enable port B
#endif //defined (__AVR_ATmega32U4__)
  pinsInitialized = true; //done
  sei(); //Turn interrupts on
}

void FloppyDrive::init() 
{ 
  if (!pinsInitialized) initFDDpins();
  bitLength=16; //  bit length for 3.5" HD floppy is 16
}

FloppyDrive::FloppyDrive(void)
{    
  init();
}

char *FloppyDrive::diskInfoStr()	//Generate disk CHS info string
{
	static char infostring[12]; //drive C/H/S info string
	char convbuf[4];
	
	if (fName[0] == 0)
	{
		strcpy_P(infostring, str_nodisk);
		return infostring;
	}		
	infostring[0] = 'C';
	infostring[1] = 0;
	itoa(numTrack, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
	strcat(infostring, "H2S");
	itoa(numSec, convbuf, 10); //max 255 -> 3 digits
	strcat(infostring, convbuf);
	return infostring;
}

int FloppyDrive::getSectorData(int lba)
{
  int n = FR_DISK_ERR;
  uint8_t *pbuf=dataBuffer+1;  
  
#if DEBUG
  uint8_t head   = 0;
  uint8_t track  = lba / (numSec*2);
  uint8_t sector = lba % (numSec*2);
  if( sector >= numSec ) { head = 1; sector -= numSec; }

  Serial.write('R');
  Serial.printDEC(track);
  Serial.write('/');
  Serial.printDEC(head);
  Serial.write('/');
  Serial.printDEC(sector+1);
  Serial.write('\n');
#endif //DEBUG
  
  if (isReady())
  {
    n = disk_read_sector(pbuf, startSector+lba);
    if (n) msg.error(err_diskread);
  }
#if ENABLE_VFFS
  else if (isVirtual()) 
    n = vffs.readSector(pbuf, lba);
#endif //ENABLE_VFFS
  return n;
}

int FloppyDrive::setSectorData(int lba)
{
  int n = FR_DISK_ERR;
  uint8_t *pbuf=dataBuffer+1;  
  
  if (isReady())
  {
    n = disk_write_sector(pbuf, startSector+lba);
    if (n) msg.error(err_diskwrite);
  }
#if ENABLE_VFFS
  else if (isVirtual()) 
    n = vffs.readSector(pbuf, lba);
#endif //ENABLE_VFFS
  
#if DEBUG
  uint8_t head   = 0;
  uint8_t track  = lba / (numSec*2);
  uint8_t sector = lba % (numSec*2);
  if( sector >= numSec ) { head = 1; sector -= numSec; }

  Serial.write('W');
  Serial.printDEC(track);
  Serial.write('/');
  Serial.printDEC(head);
  Serial.write('/');
  Serial.printDEC(sector+1);
  Serial.write('\n');

  for (int i=0; i<512; i++)
  {    
    Serial.printHEX(pbuf[i]);
    Serial.write(' ');
    if ( (i&0xf) ==0xf ) Serial.write('\n');
  }
#endif //DEBUG
  return n;
}

bool FloppyDrive::load(char *r_file) 
{
  if (FloppyDisk::load(r_file)) 
    (numSec > 9) ? bitLength = 16 : bitLength = 32; //DD or HD
  else 
    return false;  
  return true;  
}

void FloppyDrive::run()
{
  static int track=0;
  static uint8_t side=0;
  static uint8_t sector=0;
  static int lba;

  if (isChanged()) 
  {
    SET_DSKCHANGE_LOW();
    if (isReady() || isVirtual()) clrChanged();//if a disk is loaded clear diskChange flag    
  }
  (isReadonly()) ? SET_WRITEPROT_LOW() : SET_WRITEPROT_HIGH();  //check readonly  
  setup_timer1_for_write(); 
  while(GET_DRVSEL()) //PCINT for SELECTA and MOTORA
  {    
    //Start track with index signal
    SET_INDEX_LOW();
    track_start(bitLength);       
    SET_INDEX_HIGH();
    
    for (sector=0; (sector < numSec) && GET_DRVSEL(); sector++)
    {        
    #if ENABLE_WDT
      wdt_reset();
    #endif  //ENABLE_WDT
    #if defined (__AVR_ATmega32U4__)
      Serial.rcvRdy(); //service usb
    #endif //defined (__AVR_ATmega32U4__)
      if (!GET_DRVSEL()) break; //if drive unselected exit loop      
      side = (SIDE()) ? 0:1; //check side
      if (IS_TRACKCHANGED()) //if track changed
      {
        CLR_TRACKCHANGED();             
        track = iTrack;
        if (track < 0) track=0; //Check if track valid
        else if (track >= numTrack) track = numTrack-1;          
        (track == 0) ? SET_TRACK0_LOW() : SET_TRACK0_HIGH(); 
        iTrack = track;
        (isReady()) || isVirtual() ? SET_DSKCHANGE_HIGH() : SET_DSKCHANGE_LOW(); //disk present ?      
      }
      //start sector
      lba=(track*2+side)*numSec+sector;//LBA = (C × HPC + H) × SPT + (S − 1) //pointed by ikonko
      getSectorData(lba); //get sector from SD      
      setup_timer1_for_write();
      genSectorID((uint8_t)track,side,sector);
      sector_start(bitLength);          
      if (IS_TRACKCHANGED()) continue; //if track changed skip rest of the loop
      //check WriteGate
      for (int i=0; i<20; i++) //wait and check for WRITEGATE
        if (IS_WRITE() ) break;
      if (IS_WRITE() )  //write gate on               
      {
        setup_timer1_for_read();      
        read_data(bitLength, dataBuffer, 515);
      #if ENABLE_WDT  
        wdt_reset();
      #endif  //ENABLE_WDT
        setup_timer1_for_write(); //Write-mode: arduinoFDC immediately tries to verify written sector
        setSectorData(lba); //save sector to SD
        while (IS_WRITE());//wait for write to finish
      }
      else  //write gate off                                  
      {
        dataBuffer[0]   = 0xFB; // "data" id
        uint16_t crc = calc_crc(dataBuffer, 513);
        dataBuffer[513] = crc/256;
        dataBuffer[514] = crc&255;
        dataBuffer[515] = 0x4E; // first byte of post-data gap        
        write_data(bitLength, dataBuffer, 516);                                      
      }
    }//sectors             
  }//selected
  SET_DSKCHANGE_HIGH();
  SET_WRITEPROT_HIGH();
}