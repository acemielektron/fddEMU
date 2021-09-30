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

#ifndef ANALOG_READ_H
#define ANALOG_READ_H

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#define adcReady    (!(ADCSRA & (1<<ADSC)) )
#define adcBusy     (ADCSRA & (1<<ADSC))

void init_ADC();
int waitReadADC(uint8_t channel); //request and read ADC - blocking function
void reqADC(uint8_t channel); //request Analog to Digital Conversion on Channel
int readADC(); //read ADC result

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus

#endif //ANALOG_READ_H
