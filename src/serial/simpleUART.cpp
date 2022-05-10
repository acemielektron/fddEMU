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
#include <stdlib.h> //itoa

#if defined (__AVR_ATmega328P__)
    #define UCSRA   UCSR0A
    #define UCSRB   UCSR0B
    #define UDRE    UDRE0
    #define UDR     UDR0
    #define RXC     RXC0
    #define UBRR    UBRR0
    #define RXEN    RXEN0
    #define TXEN    TXEN0
    #define U2X     U2X0
#endif //defined (__ATmega328P__)
#if defined (__AVR_ATmega32U4__)
    #define UCSRA   UCSR1A
    #define UCSRB   UCSR1B
    #define UDRE    UDRE1
    #define UDR     UDR1
    #define RXC     RXC1
    #define UBRR    UBRR1
    #define RXEN    RXEN1
    #define TXEN    TXEN1
    #define U2X     U2X1
#endif //defined (__ATmega32U4__)

#if ENABLE_SERIAL
class UART0 Serial;
#endif //ENABLE_SERIAL

const uint16_t MIN_2X_BAUD = F_CPU/(4*(2*0XFFF + 1)) + 1;

int putchar_stream(char ch, FILE *stream) 
{    
    return Serial.write(ch);
}

int getchar_stream(FILE *stream) 
{    
    return Serial.read();
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

#if defined (__AVR_ATmega328P__)

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


#elif defined (__AVR_ATmega32U4__)

void UART0::init(uint32_t baud)
{
    SetupHardware();
}

int UART0::write(char ch)
{	    
    if (ch == '\n') //  ch == LF
    {
        if (RingBuffer_IsFull(&USARTtoUSB_Buffer))
            return -1;
        RingBuffer_Insert(&USARTtoUSB_Buffer, '\r'); //send CR
        usb_cdc_loop();
    }    
    if (RingBuffer_IsFull(&USARTtoUSB_Buffer))
        return -1;
    RingBuffer_Insert(&USARTtoUSB_Buffer, ch);
    usb_cdc_loop();
    return 0;
}

int UART0::read(void)
{	
    usb_cdc_loop();
    if (RingBuffer_IsEmpty(&USBtoUSART_Buffer))
        return -1;
    return RingBuffer_Remove(&USBtoUSART_Buffer);
}

#endif  //defined (__AVR_ATmega32U4__)

void UART::print(char *str)
{	
	while(*str > 0) // iterate through string		
		write(*str++);  // print character
}

void UART::print(int32_t n, uint8_t base)
{
    char itoabuf[12]; //DWORD 2^32 is 10 digits (in decimal) + sign + null terminating char
    itoa(n, itoabuf, base);
    print(itoabuf);
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
