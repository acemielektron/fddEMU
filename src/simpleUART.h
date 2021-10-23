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
#include<util/delay.h>
//if stdio is included stdio streams will be initialized for printf/scanf functions
#include <stdio.h> //sprintf & printf
#include <string.h> //strlen

#define rxReady (UCSR0A & (1 << RXC0))

//https://forum.arduino.cc/t/what-does-the-f-do-exactly/89384
class __PGMSTR;
#define F(string_literal) (reinterpret_cast<const __PGMSTR *>(PSTR(string_literal)))

class UART{
    public:
    virtual int write(char) = 0;
    virtual int read(void) = 0;
    void print(char *);
    void print(const char *str) {print((char *)str);}
    void print_P(const char *);
    void print(const __PGMSTR *fstr) {print_P((const char *)fstr);}
    void printHEX(uint8_t);
    void printHEX(uint16_t val) {printHEX((uint8_t) (val >> 8)); printHEX((uint8_t) val);}
    void printHEX(uint32_t val) {printHEX((uint16_t) (val >> 16)); printHEX((uint16_t) val);}
    void printDEC(uint32_t);
};

class UART0 : public UART{
    public:
    void init(uint32_t);
    int write(char);
    int read(void);
};

#endif //SIMPLE_UART_H