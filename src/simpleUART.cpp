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

//uses UART 0 (serial 0) on AVR

#include "simpleUART.h"
#include <stdlib.h>

class UART0 Serial;
char itoabuf[12]; //DWORD 2^32 is 10 digits (in decimal) + sign + null terminating char
const uint16_t MIN_2X_BAUD = F_CPU/(4*(2*0XFFF + 1)) + 1;

int putchar_stream(char ch, FILE *stream) 
{    
    if (ch == '\n') //if ch == LF
    {        
        while(((UCSR0A & (1<<UDRE0)) == 0)); //wait for empty tx buffer
        UDR0 = '\r';    //send CR        
    }
    while(((UCSR0A & (1<<UDRE0)) == 0)); //wait for empty tx buffer
    UDR0 = ch;
    return 0;
}

int getchar_stream(FILE *stream) 
{    
    while((UCSR0A & (1 << RXC0)) == 0); //wait for rx buffer
    return UDR0;
}


void init_UART_stdio()
{
#ifdef _STDIO_H_ //setup stdio streams according to "avr-libc/avr-libc/include/stdio.h"
//https://www.avrfreaks.net/comment/1475136#comment-1475136
static FILE UART_stdinout;
UART_stdinout.put = putchar_stream;
UART_stdinout.get = getchar_stream;
UART_stdinout.flags = _FDEV_SETUP_RW;

stdin = stdout = stderr = &UART_stdinout;
#endif //_STDIO_H_ stdio streams
}

void UART0::init(uint32_t baud)
{   //https://forum.arduino.cc/t/over-600-bytes-of-ram-for-serial-on-a-mega-gasp/71429
    uint16_t baud_setting;
    // don't worry, the compiler will squeeze out F_CPU != 16000000UL
	if ((F_CPU != 16000000UL || baud != 57600) && baud > MIN_2X_BAUD) {      
      UCSR0A = 1 << U2X0;   // Double the USART Transmission Speed
      baud_setting = (F_CPU / 4 / baud - 1) / 2;
    } else {
      // hardcoded exception for compatibility with the bootloader shipped
      // with the Duemilanove and previous boards and the firmware on the 8U2
      // on the Uno and Mega 2560.
      UCSR0A = 0;
      baud_setting = (F_CPU / 8 / baud - 1) / 2;
    }	
    UBRR0 = baud_setting;   // assign the baud_setting	
	UCSR0B |= (1 << RXEN0)|(1 << TXEN0);    // Enable reciever and transmitter
	init_UART_stdio();
}

int UART0::write(char ch)
{	
	if (ch == '\n') //  ch == LF
    {        
        while((UCSR0A & (1<<UDRE0)) == 0);  // wait for empty transmit buffer
        UDR0 = '\r';    //  send CR        
    }
    while((UCSR0A & (1<<UDRE0)) == 0);  // wait for empty transmit buffer
    UDR0 = ch;  // send char to output register
    return 0; //returns EOF on error
}

int UART0::read(void)
{	
	while((UCSR0A & (1 << RXC0)) == 0); // wait for data to be received	
	return UDR0;    // return output register
}

void UART::print(char *str)
{	
	while(*str > 0) // iterate through string		
		write(*str++);  // print character
}

void UART::print_P(const char *pstr)
{       
    char ch;
    while ((ch = pgm_read_byte_near(pstr++)) > 0)
        write(ch);
}

void UART::printHEX(uint8_t val)
{   //http://rjhcoding.com/avrc-uart.php
    // extract upper and lower nibbles from input value
    uint8_t upperNibble = (val & 0xF0) >> 4;
    uint8_t lowerNibble = val & 0x0F;

    // convert nibble to its ASCII hex equivalent
    upperNibble += upperNibble > 9 ? 'A' - 10 : '0';
    lowerNibble += lowerNibble > 9 ? 'A' - 10 : '0';

    // print the characters
    write(upperNibble);
    write(lowerNibble);
}

void UART::printDEC(uint32_t i)
{
    itoa(i, itoabuf, 10);
    print(itoabuf);
}