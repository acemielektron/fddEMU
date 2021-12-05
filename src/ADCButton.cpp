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

#include <avr/io.h>
#include <util/delay.h>
#include "ADCButton.h"
#include "SerialUI.h" //debug

#if ENABLE_GUI
class ADCButton abtn;
#endif //ENABLE_GUI

ADCButton::ADCButton()
{
  init();
  requestADC(BUTTON_CHANNEL);
}

void ADCButton::init()
{
#if defined(__AVR_ATmega328P__)  
  PRR &= ~(1<<PRADC); //clear PRADC bit in Power Reduction Register
#elif defined(__AVR_ATmega32U4__)
  PRR0 &= ~(1<<PRADC); //clear PRADC bit in Power Reduction Register
#endif  
  ADMUX = (1<<REFS0);     //select AVCC as reference
  ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);  //enable and prescale = 128 (16MHz/128 = 125kHz)
}

void ADCButton::requestADC(uint8_t channel) //request Analog to Digital Conversion on Channel
{
    ADMUX=( (channel&0x0f)|(1<<REFS0) ); //Select channel and voltage reference, if ADLAR is set result is left justified ADCH register is sufficient
    ADCSRA|=(1<<ADSC); // Start the AD conversion
    //sample-and-hold takes 13.5 ADC clock cycles on first conversion 
    //and 1.5 ADC clock cycles on subsequent converisons
    //ADC prescaler is set to clock/128, clock=16mhz so "1.5ADC cycle" = "192 cpu cycle" = 12uS
    //First conversion "13.5 ADC clock cycle" = 108us
    //_delay_us(12);    
    //First conversion takes 25 ADC cycles and subsequent conversions takes 13 ADC clock cycles
}

int16_t ADCButton::readADC() //read ADC result
{
    int result = 0;
    uint8_t low, high;
    low=ADCL;
    high=ADCH;
    result = (high << 8) | low;
    //return ADCW;
    return result;
}



int8_t ADCButton::read()
{
  static uint8_t prevval = 6; 
  static uint8_t lastval = 6;  
  uint8_t newval = 7;
  uint16_t adcval;

  if (adcBusy)  return 0;  
  adcval = readADC();
  requestADC(BUTTON_CHANNEL);
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
