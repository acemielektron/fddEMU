// -----------------------------------------------------------------------------
// simple UART
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

#ifndef SIMPLE_UART_H
#define SIMPLE_UART_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
//if stdio is included stdio streams will be initialized for printf/scanf functions
#include <stdio.h> //sprintf & printf
#include <string.h> //strlen

#if defined (__AVR_ATmega328P__)
    #define rxReady (UCSR0A & (1 << RXC0))
#elif defined (__AVR_ATmega32U4__)
    #include "USBtoSerial.h"
    #define rxReady Serial.rcvRdy()
#endif //defined (__ATmega32U4__)
    
//https://forum.arduino.cc/t/what-does-the-f-do-exactly/89384
class __PGMSTR;
#define F(string_literal) (reinterpret_cast<const __PGMSTR *>(PSTR(string_literal)))

//define bases for printing numbers
#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

class UART{
    public:
    virtual int write(char) = 0;
    virtual int read(void) = 0;
    void print(char *); // print from ram
    void print(const char *str) {print((char *)str);} // print from ram const 
    void print_P(const char *); //print from flash
    void print(const __PGMSTR *fstr) {print_P((const char *)fstr);} //print from flash F() macro
    void print(int32_t n, uint8_t base); // print number, base = ? BIN/OCT/DEC/HEX
    void print(int32_t n) {print(n, DEC);} // print number, base = 10    
    void printHEX(uint8_t); // print hex byte  
};

class UART0 : public UART{
    public:
    void init(uint32_t);
    int write(char);
    int read(void);
#if defined (__AVR_ATmega32U4__)
    bool rcvRdy() {usb_cdc_loop(); return !RingBuffer_IsEmpty(&USBtoUSART_Buffer);};
#endif //defined (__ATmega32U4__)    
};

extern class UART0 Serial;

#endif //SIMPLE_UART_H