// -----------------------------------------------------------------------------
// simple analog read
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


void init_ADC()
{
  PRR &= ~(1<<PRADC); //clear PRADC bit in Power Reduction Register
  ADMUX = (1<<REFS0);     //select AVCC as reference
  ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);  //enable and prescale = 128 (16MHz/128 = 125kHz)
}

int waitReadADC(uint8_t channel) //request and read ADC - blocking function
{  
  uint8_t low, high;
  
  ADMUX=( (channel&0x0f)|(1<<REFS0) ); //Select channel and voltage reference, ADLAR is set so result is left justified ADCH register is sufficient
  _delay_us(150);  
  ADCSRA|=(1<<ADSC); // Start the AD conversion
  while ( ADCSRA & (1<<ADSC) ); //wait for conversion to finish
  low=ADCL;
  high=ADCH;
  //return ADCW;
  return ( (high << 8) | low );
}

void reqADC(uint8_t channel) //request Analog to Digital Conversion on Channel
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

int readADC() //read ADC result
{
    int result = 0;
    uint8_t low, high;
    low=ADCL;
    high=ADCH;
    result = (high << 8) | low;
    //return ADCW;
    return result;
}