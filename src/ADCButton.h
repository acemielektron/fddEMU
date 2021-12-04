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

#ifndef ADCBUTTON_H
#define ADCBUTTON_H

#define adcReady    (!(ADCSRA & (1<<ADSC)) )
#define adcBusy     (ADCSRA & (1<<ADSC))

#if defined (__AVR_ATmega328P__)
    #define BUTTON_PIN 7 //ADC7
#endif //defined (__Atmega328p__)
#if defined (__AVR_ATmega32U4__)
    #define BUTTON_PIN -1 //disabled for now
#endif //defined (__Atmega32U4__)

#define BTN_CANCEL  1
#define BTN_OK      2
#define BTN_UP      3
#define BTN_DOWN    4
#define BTN_EXTRA   5     

class ADCButton{
    private:
    void init();
    void requestADC(uint8_t channel);
    int16_t readADC();
    
    public:
    ADCButton();        
    int8_t read();
};

extern class ADCButton abtn;

#endif //ADCBUTTON_H
