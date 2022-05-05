// -----------------------------------------------------------------------------
// Modified for use with fddEMU "Floppy Disk Drive Emulator" 
// 2021 Acemi Elektronikci
// -----------------------------------------------------------------------------
// 3.5"/5.25" DD/HD Disk controller for Arduino
// Copyright (C) 2021 David Hansel
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

#include "avrFlux.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

#if defined(__AVR_ATmega328P__)

asm ("   .equ TIFR,    0x16\n"  // timer 1 flag register
     "   .equ TOV,     0\n"     // overflow flag
     "   .equ OCF,     1\n"     // output compare flag
     "   .equ ICF,     5\n"     // input capture flag
     "   .equ TCCRC,   0x82\n"  // timer 1 control register C
     "   .equ FOC,     0x80\n"  // force output compare flag
     "   .equ TCNTL,   0x84\n"  // timer 1 counter (low byte)
     "   .equ ICRL,    0x86\n"  // timer 1 input capture register (low byte)
     "   .equ OCRL,    0x88\n"  // timer 1 output compare register (low byte)
     "   .equ IDXDDR,  0x0A\n" // DDRD 
     "   .equ IDXBIT,  7\n" // INDEX pin bit (digital pin 7, register PORTD)     
     "   .equ WGPORT,  0x06\n" // PINC
     "   .equ WGBIT,   0\n" // A0
     );

#define TIFR    TIFR1   // timer 1 flag register
#define TOV     TOV1    // overflow flag
#define OCF     OCF1A   // output compare flag
#define ICF     ICF1    // input capture flag
#define TCCRA   TCCR1A  // timer 1 control register A
#define COMA1   COM1A1  // timer 1 output compare mode bit 1
#define COMA0   COM1A0  // timer 1 output compare mode bit 0
#define TCCRB   TCCR1B  // timer 1 control register B
#define CS1     CS11    // timer 1 clock select bit 1
#define CS0     CS10    // timer 1 clock select bit 0
#define WGM2    WGM12   // timer 1 waveform mode bit 2
#define TCCRC   TCCR1C  // timer 1 control register C
#define FOC     FOC1A   // force output compare flag
#define OCR     OCR1A   // timer 1 output compare register
#define TCNT    TCNT1   // timer 1 counter
#define OCDDR   DDRB    // DDR controlling WRITEDATA pin
#define OCBIT   1       // bit for WRITEDATA pin

#elif defined(__AVR_ATmega32U4__)

// -----------------------  Pin assignments for Arduino Leonardo/Micro (Atmega32U4)  --------------------------

asm ("   .equ TIFR,    0x16\n"  // timer 1 flag register
     "   .equ TOV,     0\n"     // overflow flag
     "   .equ OCF,     1\n"     // output compare flag
     "   .equ ICF,     5\n"     // input capture flag
     "   .equ TCCRC,   0x82\n"  // timer 1 control register C
     "   .equ FOC,     0x80\n"  // force output compare flag
     "   .equ TCNTL,   0x84\n"  // timer 1 counter (low byte)
     "   .equ ICRL,    0x86\n"  // timer 1 input capture register (low byte)
     "   .equ OCRL,    0x88\n"  // timer 1 output compare register (low byte)
     "   .equ IDXDDR,  0x0A\n" // DDRD 
     "   .equ IDXBIT,  7\n"     // INDEX pin bit (digital pin 6, register PORTD)     
     "   .equ WGPORT,  0x0C\n"  // PINE accessed via SBIS instruction
     "   .equ WGPIN,   6\n"   // pin 7
     );

#define TIFR    TIFR1   // timer 1 flag register
#define TOV     TOV1    // overflow flag
#define OCF     OCF1A   // output compare flag
#define ICF     ICF1    // input capture flag
#define TCCRA   TCCR1A  // timer 1 control register A
#define COMA1   COM1A1  // timer 1 output compare mode bit 1
#define COMA0   COM1A0  // timer 1 output compare mode bit 0
#define TCCRB   TCCR1B  // timer 1 control register B
#define CS1     CS11    // timer 1 clock select bit 1
#define CS0     CS10    // timer 1 clock select bit 0
#define WGM2    WGM12   // timer 1 waveform mode bit 2
#define TCCRC   TCCR1C  // timer 1 control register C
#define FOC     FOC1A   // force output compare flag
#define OCR     OCR1A   // timer 1 output compare register
#define TCNT    TCNT1   // timer 1 counter
#define OCDDR   DDRB    // WRITEDATA pin port (digital pin  9, register PB5)
#define OCBIT   5       // WRITEDATA pin bit  (digital pin  9, register PB5)


#elif defined(__AVR_ATmega2560__)

// ------------------------------  Pin assignments for Arduino Mega (Atmega2560)  -----------------------------

asm ("   .equ TIFR,    0x1A\n"  // timer 5 flag register
     "   .equ TOV,     0\n"     // overflow flag
     "   .equ OCF,     1\n"     // output compare flag
     "   .equ ICF,     5\n"     // input capture flag
     "   .equ TCCRC,   0x122\n" // timer 5 control register C
     "   .equ FOC,     0x80\n"  // force output compare flag
     "   .equ TCNTL,   0x124\n" // timer 5 counter (low byte)
     "   .equ ICRL,    0x126\n" // timer 5 input capture register (low byte)
     "   .equ OCRL,    0x128\n" // timer 5 output compare register (low byte)
     "   .equ IDXPORT, 0x109\n" // INDEX pin register (digital pin 47, register PL2)
     "   .equ IDXBIT,  2\n"     // INDEX pin bit (digital pin 47, register PL2)
     "   /TODO/.equ WGPORT,  0x06\n"  // PINC
     "   /TODO/.equ WGPIN,   0x0\n"   // pin 1
     );

#define TIFR    TIFR5   // timer 5 flag register
#define TOV     TOV5    // overflow flag
#define OCF     OCF5A   // output compare flag
#define ICF     ICF5    // input capture flag
#define TCCRA   TCCR5A  // timer 5 control register A
#define COMA1   COM5A1  // timer 5 output compare mode bit 1
#define COMA0   COM5A0  // timer 5 output compare mode bit 0
#define TCCRB   TCCR5B  // timer 5 control register B
#define CS1     CS51    // timer 5 clock select bit 1
#define CS0     CS50    // timer 5 clock select bit 0
#define WGM2    WGM52   // timer 5 waveform mode bit 2
#define TCCRC   TCCR5C  // timer 5 control register C
#define FOC     FOC5A   // force output compare flag
#define OCR     OCR5A   // timer 5 output compare register
#define TCNT    TCNT5   // timer 5 counter
#define OCDDR   DDRL    // DDR controlling WRITEDATA pin
#define OCBIT   3       // bit for WRITEDATA pin

#else // not __AVR_ATmega328P__ or __AVR_ATmega32U4__ or __AVR_ATmega2560__

#error "ArduinoFDC library requires either an ATMega328P, Atmega32U4 or ATMega2560 processor (Arduino UNO, Leonardo or MEGA)"

#endif  //__AVR_ATmega328P__

#if F_CPU != 16000000
  #error "avrFlux library requires 16MHz clock speed"
#endif  //F_CPU != 16000000

static const uint16_t PROGMEM crc16_table[256] =
{
 0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
 0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
 0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
 0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
 0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
 0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

uint16_t calc_crc(uint8_t *buf, int n)
{
  // already includes sync marks (0xA1, 0xA1, 0xA1)
  uint16_t crc = 0xCDB4;

  // compute CRC of remaining data
  while( n-- > 0 )
    crc = pgm_read_word_near(crc16_table + (((crc >> 8) ^ *buf++) & 0xff)) ^ (crc << 8);

  return crc;
}

void setup_timer1_for_write()
{
  // set up timer
  TCCRB = (1 << CS0); // prescaler 1
  // make sure OC1A is high before we enable WRITE_GATE
  OCDDR  &= ~(1 << OCBIT);             // disable OC1A pin
  TCCRA  = (1 << COMA1) | (1 <<COMA0);  // set OC1A on compare match
  TCCRC |= (1 << FOC);                 // force compare match
  TCCRA  = 0;                        // disable OC1A control by timer
  OCDDR |= (1 << OCBIT);               // enable OC1A pin

  // reset timer and overrun flags
  TIFR = (1 << TOV);
  TCCRB |= (1 << WGM2);   // WGMx2:10 = 010 => clear-timer-on-compare (CTC) mode 
  TCNT = 0;             // reset timer
  OCR = 32;             // clear OCRxH byte (we only modify OCRxL below)
  TIFR = (1 << OCF);      // clear OCFx

  // enable OC1A output pin control by timer (WRITE_DATA), initially high
  TCCRA  = (1 << COMA0); // COMxA1:0 =  01 => toggle OC1A on compare match  
}

void setup_timer1_for_read()
{
  // reset timer and capture/overrun flags
  TCNT = 0;
  TIFR = (1 << ICF) | (1 << TOV);           
  //setup_timer_for_read     
  TCCRA = 0;
  TCCRB = (1 << CS0); // falling edge input capture, prescaler 1, no output compare
  TCCRC = 0;      
  TIMSK0 &= ~(1 << TOIE0); //Disable Timer0 Overflow - noInterrupts();
}
   
//TODO: implement timer overflow check, cpu hangs up if read pin disconnected or has no external pullup
uint8_t read_data(uint8_t bitlen, uint8_t *buffer, unsigned int n)
{
  uint8_t status = 0; //timer overflow counter will be set to this value

  // expect at least 10 bytes of 0x00 followed by three sync marks (0xA1 with one missing clock bit)
  // Data bits :     0 0 ...0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
  // In MFM    : (0)1010...10 0100010010001001 0100010010001001 0100010010001001

  asm volatile 
    (// define READPULSE macro (wait for pulse)
     // macro arguments: 
     //         length: none => just wait for pulse, don't check         ( 9 cycles)
     //                 1    => wait for pulse and jump if NOT short  (12/14 cycles)
     //                 2    => wait for pulse and jump if NOT medium (14/16 cycles)
     //                 3    => wait for pulse and jump if NOT long   (12/14 cycles)
     //         dst:    label to jump to if DIFFERENT pulse found
     // 
     // on entry: r16 contains minimum length of medium pulse
     //           r17 contains minimum length of long   pulse
     //           r18 contains time of previous pulse
     // on exit:  r18 is updated to the time of this pulse
     //           r22 contains the pulse length in timer ticks (=processor cycles)     
     // CLOBBERS: r19
     ".macro READPULSE length=0,dst=undefined\n"
     "        sbis    TIFR, ICF\n"     // (1/2) skip next instruction if timer input capture seen
     "        rjmp    .-4\n"           // (2)   wait more 
     "        lds     r19, ICRL\n"     // (2)   get time of input capture (ICR1L, lower 8 bits only)
     "        sbi     TIFR, ICF\n "    // (2)   clear input capture flag
     "        mov     r22, r19\n"      // (1)   calculate time since previous capture...
     "        sub     r22, r18\n"      // (1)   ...into r22
     "        mov     r18, r19\n"      // (1)   set r18 to time of current capture
     "  .if \\length == 1\n"           //       waiting for short pulse?
     "        cp      r22, r16\n"      // (1)   compare r22 to min medium pulse
     "        brlo   .+2\n"            // (1/2) skip jump if less
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "  .else \n"
     "    .if \\length == 2\n"         // waiting for medium pulse?
     "        cp      r16, r22\n"      // (1)   min medium pulse < r22? => carry set if so
     "        brcc    .+2\n"           // (1/2) skip next instruction if carry is clear
     "        cp      r22, r17\n"      // (1)   r22 < min long pulse? => carry set if so
     "        brcs   .+2\n"            // (1/2) skip jump if greater
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "    .else\n"
     "      .if \\length == 3\n" 
     "        cp      r22, r17\n"      // (1)   min long pulse < r22?
     "        brsh   .+2\n"            // (1/2) skip jump if greater
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "      .endif\n"
     "    .endif\n"
     "  .endif\n"
     ".endm\n"
     //add check WRITE_GATE to READPULSE
     ".macro READPULSE_CHK length=0,dst=undefined\n"
     "        sbic    WGPORT, WGBIT\n" // (2) skip next instruction if WRITE_GATE is asserted
     "        rjmp    rd_err\n"        // exit: read error
     "        sbis    TIFR, ICF\n"     // (1/2) skip next instruction if timer input capture seen
     "        rjmp    .-4\n"           // (2)   wait more 
     "        lds     r19, ICRL\n"     // (2)   get time of input capture (ICR1L, lower 8 bits only)
     "        sbi     TIFR, ICF\n "    // (2)   clear input capture flag
     "        mov     r22, r19\n"      // (1)   calculate time since previous capture...
     "        sub     r22, r18\n"      // (1)   ...into r22
     "        mov     r18, r19\n"      // (1)   set r18 to time of current capture
     "  .if \\length == 1\n"           //       waiting for short pulse?
     "        cp      r22, r16\n"      // (1)   compare r22 to min medium pulse
     "        brlo   .+2\n"            // (1/2) skip jump if less
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "  .else \n"
     "    .if \\length == 2\n"         // waiting for medium pulse?
     "        cp      r16, r22\n"      // (1)   min medium pulse < r22? => carry set if so
     "        brcc    .+2\n"           // (1/2) skip next instruction if carry is clear
     "        cp      r22, r17\n"      // (1)   r22 < min long pulse? => carry set if so
     "        brcs   .+2\n"            // (1/2) skip jump if greater
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "    .else\n"
     "      .if \\length == 3\n" 
     "        cp      r22, r17\n"      // (1)   min long pulse < r22?
     "        brsh   .+2\n"            // (1/2) skip jump if greater
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "      .endif\n"
     "    .endif\n"
     "  .endif\n"
     ".endm\n"

     // define STOREBIT macro for storing or verifying data bit 
     // storing  data  : 5/14 cycles for "1", 4/13 cycles for "0"
     // verifying data : 5/15 cycles for "1", 4/14 cycles for "0"
     ".macro STOREBIT data:req,done:req\n"
     "        lsl     r20\n"           // (1)   shift received data
     ".if \\data != 0\n"
     "        ori     r20, 1\n"        // (1)   store "1" bit
     ".endif\n"
     "        dec     r21\n"           // (1)   decrement bit counter
     "        brne    .+10\n"          // (1/2) skip if bit counter >0    
     "        st      Z+, r20\n"       // (2)   store received data byte
     "        ldi     r21, 8\n"        // (1)   re-initialize bit counter
     "        subi    r26, 1\n"        // (1)   subtract one from byte counter
     "        sbci    r27, 0\n"        // (1) 
     "        brmi    \\done\n"        // (1/2) done if byte counter<0
     ".endm\n"
          
     // prepare for reading SYNC
     "        mov         r16, %1\n"   // (1)   r16 = 2.5 * (MFM bit len) = minimum length of medium pulse
     "        lsr         r16\n"       // (1)
     "        add         r16, %1\n"   // (1)
     "        add         r16, %1\n"   // (1)
     "        mov         r17, r16\n"  // (1)   r17 = 3.5 * (MFM bit len) = minimum length of long pulse
     "        add         r17, %1\n"   // (1)
     "        ldi         %0, 0\n"     // (1)   default return status is S_OK
     "        mov         r15, %0\n"   // (1)   initialize timer overflow counter
     "        sbi         TIFR, TOV\n" // (2)   reset timer overflow flag

     // wait for at least 80x "10" (short) pulse followed by "100" (medium) pulse
     "ws0:    ldi         r20, 0\n"    // (1)   initialize "short pulse" counter
     "ws1:    sbis        TIFR, TOV\n" // (1/2) skip next instruction if timer overflow occurred
     "        rjmp        ws2\n"       // (2)   continue (no overflow)
     "        sbi         TIFR, TOV\n" // (2)   reset timer overflow flag
     "        dec         r15\n"       // (1)   overflow happens every 4.096ms, decrement overflow counter
     "        brne        ws2\n"       // (1/2) continue if fewer than 256 overflows
     "        ldi         %0, 3\n"     // (1)   no sync found in 1.048s => return status is is S_NOSYNC
     "        rjmp        rdend\n"     // (2)   done
     "ws2:    inc         r20\n"       // (1)   increment "short pulse" counter
     "        READPULSE_CHK\n"         // (11)   wait for pulse and check WRITE_GATE
     "        cp          r22, r16\n"  // (1)   pulse length < min medium pulse?
     "        brlo        ws1\n"       // (1/2) repeat if so
     "        cp          r22, r17\n"  // (1)   pulse length < min long pulse?
     "        brsh        ws0\n"       // (1/2) restart if this was a long pulse (expecting medium)
     "        cpi         r20, 80\n"   // (1)   did we see at least 80 short pulses?     
     "        brlo        ws0\n"       // (1/2) restart if not
    
     // expect remaining part of first sync mark (..00010010001001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)

     // expect second sync mark (0100010010001001)
     "        READPULSE   1,ws0\n"     // (12)  expect short pulse (01)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)

     // expect third sync mark (0100010010001001)
     "        READPULSE   1,ws0\n"     // (12)  expect short pulse (01)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)   

     // found SYNC => prepare for reading data
     "        tst     r27\n"           // (1)   test byte count
     "        brpl    .+2\n"           // (1/2) skip following instruction if not negative
     "        rjmp    rdend\n"         // (2)   nothing to read (only waiting for sync) => end
     "        ldi     r21, 8\n"        // (1)   initialize bit counter (8 bits per byte)

     // odd section (previous data bit was "1", no unprocessed MFM bit)
     // shortest path: 19 cycles, longest path: 34 cycles
     // (longest path only happens when finishing a byte, about every 5-6 pulses)
     "rdo:    READPULSE\n"             // (9)   wait for pulse
     "        cp      r22, r16\n"      // (1)   pulse length >= min medium pulse?
     "        brlo    rdos\n"          // (1/2) jump if not
     "        cp      r22, r17\n"      // (1)   pulse length >= min long pulse?
     "        brlo    rdom\n"          // (1/2) jump if not

     // long pulse (0001) => read "01", still odd
     "        STOREBIT 0,rddone\n"      // (4/13) store "0" bit
     "        STOREBIT 1,rddone\n"      // (5/14) store "1" bit
     "        rjmp    rdo\n"            // (2)    back to start (still odd)

     // jump target for relative conditional jumps in STOREBIT macro
     "rddone:  rjmp    rdend\n"
     
     // medium pulse (001) => read "0", now even
     "rdom:   STOREBIT 0,rddone\n"      // (4/13) store "0" bit
     "        rjmp    rde\n"            // (2)   back to start (now even)

     // short pulse (01) => read "1", still odd
     "rdos:   STOREBIT 1,rddone\n"      // (5/14) store "1" bit
     "        rjmp    rdo\n"            // (2)    back to start (still odd)

     // even section (previous data bit was "0", previous MFM "1" bit not yet processed)
     // shortest path: 19 cycles, longest path: 31 cycles
     "rde:    READPULSE\n"             // (9)   wait for pulse
     "        cp      r22, r16\n"      // (1)   pulse length >= min medium pulse?
     "        brlo    rdes\n"          // (1/2) jump if not

     // either medium pulse (1001) or long pulse (10001) => read "01"
     // (a long pulse should never occur in this section but it may just be a 
     //  slightly too long medium pulse so count it as medium)
     "        STOREBIT 0,rdend\n"      // (4/13) store "0" bit
     "        STOREBIT 1,rdend\n"      // (5/14) store "1" bit
     "        rjmp    rdo\n"           // (2)    back to start (now odd)

     // short pulse (101) => read "0"
     "rdes:   STOREBIT 0,rdend\n"      // (5/14) store "0" bit
     "        rjmp    rde\n"           // (2)    back to start (still even)

     "rd_err: ldi     %0, 1\n"         // return status is 1 (write error)
     "rdend: \n"
     
     : "=r"(status)                         // outputs
     : "r"(bitlen), "x"(n-1), "z"(buffer)   // inputs  (x=r26/r27, z=r30/r31)
     : "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22");  // clobbers

  return status;
}

asm (// define WRITEPULSE macro (used in write_data and format_track)
     ".macro WRITEPULSE length=0\n"
     "  .if \\length==1\n"
     "          sts   OCRL, r16\n"       // (2)   set OCRxA to short pulse length
     "  .endif\n"
     "  .if \\length==2\n"
     "          sts   OCRL, r17\n"       // (2)   set OCRxA to medium pulse length
     "  .endif\n"
     "  .if \\length==3\n"
     "          sts   OCRL, r18\n"       // (2)   set OCRxA to long pulse length
     "  .endif\n"
     "          sbis  TIFR, OCF\n"       // (1/2) skip next instruction if OCFx is set
     "          rjmp  .-4\n"             // (2)   wait more
     "          ldi   r19,  FOC\n"       // (1)
     "          sts   TCCRC, r19\n"      // (2)   set OCP back HIGH (was set LOW when timer expired)
     "          sbi   TIFR, OCF\n"       // (2)   reset OCFx (output compare flag)
     ".endm\n");
     
asm( // define GETNEXTBIT macro for getting next data bit into carry (4/9 cycles)
     // on entry: R20         contains the current byte 
     //           R21         contains the bit counter
     //           X (R26/R27) contains the byte counter
     //           Z (R30/R31) contains pointer to data buffer
     ".macro GETNEXTBIT\n"
     "          dec     r21\n"           // (1)   decrement bit counter
     "          brpl    .+10\n"          // (1/2) skip the following if bit counter >=  0
     "          subi    r26, 1\n"        // (1)   subtract one from byte counter
     "          sbci    r27, 0\n"        // (1) 
     "          brmi    wdone\n"         // (1/2) done if byte counter <0
     "          ld      r20, Z+\n"       // (2)   get next byte
     "          ldi     r21, 7\n"        // (1)   reset bit counter (7 more bits after this first one)
     "          rol     r20\n"           // (1)   get next data bit into carry
     ".endm\n");     


int16_t write_sector(uint8_t *buffer, uint8_t bitlen)
{
  // 3.5" DD disk:
  //   writing 95 + 1 + 65 + (7 + 37 + 515 + 69) * 8 + (7 + 37 + 515) bytes
  //   => 5744 bytes per track = 45952 bits
  //   data rate 250 kbit/second, rotation rate 300 RPM (0.2s per rotation)
  //   => 50000 bits unformatted capacity per track

  // 3.5" HD disk:
  //   writing 95 + 1 + 65 + (7 + 37 + 515 + 69) * 17 + (7 + 37 + 515) bytes
  //   => 5744 bytes per track = 45952 bits
  //   data rate 500 kbit/second, rotation rate 300 RPM (0.2s per rotation)
  //   => 100000 bits unformatted capacity per track
  uint16_t result;
  uint8_t datagaplen = 54;

  asm volatile
    (".macro    WRTPS_CHK\n" // wtite short pulse & check write gate (max 12 cycles)
     "          sts   OCRL, r16\n"     
     "          sbis  WGPORT, WGBIT\n"     // (2) skip next instruction if WGBIT is set (not asserted)
     "          rjmp  wr_exit\n"           // (2) WG asserted jump to write exit
     "          sbis  TIFR, OCF\n"         // (1/2) skip next instruction if OCFx is set
     "          rjmp  .-4\n"               // (2)   wait more
     "          ldi   r19,  FOC\n"         // (1)
     "          sts   TCCRC, r19\n"        // (2)   set OCP back HIGH (was set LOW when timer expired)
     "          sbi   TIFR, OCF\n"         // (2)   reset OCFx (output compare flag)     
     ".endm\n"
     ".macro    WRTPS\n"
     "          sts   OCRL, r16\n"     
     "          call  waitp\n"
     ".endm\n"
     ".macro    WRTPM\n"
     "          sts   OCRL, r17\n"     
     "          call  waitp\n"
     ".endm\n"
     ".macro    WRTPL\n"
     "          sts   OCRL, r18\n"     
     "          call  waitp\n"
     ".endm\n"

     // save SPH:SPL
     "          push    r3\n" // save r3 to stack
     "          push    r2\n" // save r2 to stack
     "          in      r2, __SP_L__\n" // save SPL to r2
     "          in      r3, __SP_H__\n" // save SPH to r3          
     // initialize pulse-length registers (r16, r17, r18)
     "          mov    r16, %[bitlen]\n"         // r16 = (2*bitlen)-1 = time for short ("01") pulse         
     "          add    r16, %[bitlen]\n"
     "          dec    r16\n"
     "          mov    r17, r16\n"        //       r17 = (3*bitlen)-1 = time for medium ("001") pulse
     "          add    r17, %[bitlen]\n"
     "          mov    r18, r17\n"        //       r18 = (4*bitlen)-1 = time for long ("0001") pulse
     "          add    r18, %[bitlen]\n"
     // copy Z register to X register     
     "          movw    X, Z\n"    
     // point X register to "sector" (buffer+3) in buffer 
     "          adiw   X, 3\n" 
     "          ld     r19, X+\n" // load sector to r19
     "          ld     r20, X\n" // load length to r20     
     "          cpi    r20, 3\n" // if absolute "value=3" > r20 "carry" is set, if equal "zero" is set
     "          brcs   continue\n" // if "carry" is set branch
     "          rjmp   err_exit\n" // we don't support sector size > 512
     "continue: eor    r27, r27\n" // clear X high byte
     "          ldi    r26, 64\n" //put "64" in X low byte
     "          inc    r20\n" // increment r20 "length" by 1 (minimum length is 128)
     "calc_len: add    r26, r26\n" // shift left r26
     "          adc    r27, r27\n" // shift left with carry r27
     "          dec    r20\n"
     "          brne   calc_len\n"
     // we need to put number of bytes we want to write into X
     "          adiw   X, 4\n" // add 4 to X "length" id(1)+crc(2)+gap(1)
     //check if we are sending the first sector, if so track start
     "          ldi     r20, 0\n"        
     "          sts     TCNTL, r20\n" // reset timer     
     "          dec     r19\n" // if sector was "1", r19 is now "0" zero flag is set
     "          breq    trkstart\n" // Branch if "Zero Flag" is set     
     "          rjmp    secstart\n" // else jump to sector start

     // 1) ---------- 56x 0x4E (pre-index gap)
     //
     // 0x4E             0x4E             ...
     //  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 1001001001010100 1001001001010100 ...
     // M  M  M  M S S   M  M  M  M S S   ...     
     // => (MMMMSS)x56
    
     "trkstart: sbi    IDXDDR, IDXBIT\n" // (2) set index pin to output = low    
     "          ldi    r20, 56\n"          // (1) write 56 gap bytes
     "          call   wrtgap\n"           //     returns 20 cycles after final pulse was written

     // 2) ---------- 12x 0x00
     //
     // 0x4E             0x00             0x00             ...
     //  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
     // 1001001001010100 1010101010101010 1010101010101010 ...
     // S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
     // => MSx95
     "          WRTPM\n"                   // write medium pulse
     "          ldi    r20, 95\n"          // write 95 short pulses
     "          call   wrtshort\n"         // returns 20 cycles after final pulse was written
     
     // 3) ---------- 3x SYNC 0xC2
     //
     // 0x00              0xC2            0xC2             0xC2
     //  0 0 0 0 0 0 0 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0
     // 1010101010101010 0101001000100100 0101001000100100 0101001000100100
     // S S S S S S S S   M S  M   L  M    L S  M   L  M    L S  M   L  M
     // => MSMLM(LSMLM)x2
     "          ldi    r20, 3\n"
     "          WRTPM\n"                   // write medium pulse (returns 14 cycles after pulse)
     "          rjmp   iskip\n"            // (2)
     "iloop:    WRTPL\n"                   // write long   pulse
     "iskip:    WRTPS\n"                   // write short  pulse
     "          WRTPM\n"                   // write medium pulse
     "          WRTPL\n"                   // write long   pulse
     "          WRTPM\n"                   // write medium pulse
     "          dec    r20\n"              // (1)
     "          brne   iloop\n"            // (1/2)

     // 4) ---------- index record (0xFC)
     //
     // 0xC2             0xFC            
     //  1 1 0 0*0 0 1 0  1 1 1 1 1 1 0 0
     // 0101001000100100 0101010101010010
     //  L S  M   L  M    L S S S S S  M 
     // => LSSSSSM
     "          WRTPL\n"                   // write long pulse (returns 14 cycles after pulse)
     "          ldi    r20, 5\n"           // (1) write 5 short pulses
     "          call   wrtshort\n"         // 6 cycles until timer update, 20 cycles after pulse
     "          WRTPM\n"                   // write medium pulse

     // 5) ---------- 50x 0x4E (post-index gap)
     //
     // 0xFC             0x4E             0x4E             ...
     //  1 1 1 1 1 1 0 0  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 0101010101010010 1001001001010100 1001001001010100 ...
     //  L S S S S S  M  S  M  M  M S S   M  M  M  M S S   ...
     // => SMMMSS (MMMMSS)x49
     "          ldi    r20, 49\n"           // (1) write 49 gap bytes
     "          WRTPS\n"                    // write short  pulse
     "          call   wrtgap2\n"           //     returns 20 cycles after final pulse was written
     "          cbi    IDXDDR, IDXBIT\n" // (2) set index pin to "0" input = high
     
     // 6) ---------- 12x 0x00
     //
     // 0x4E             0x00             0x00             ...
     //  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
     // 1001001001010100 1010101010101010 1010101010101010 ...
     // S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
     // => MSx95     
     "secstart: WRTPM\n"                   // write medium pulse
     "          WRTPM\n"                   // write medium pulse
     "          ldi    r20, 95\n"          // write 95 short pulses
     "          call   wrtshort\n"         // returns 20 cycles after final pulse was written
     
     // 7) ---------- 3x SYNC 0xA1
     //
     //  0x00             0xA1             0xA1             0xA1
     //  0 0 0 0 0 0 0 0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
     // 1010101010101010 0100010010001001 0100010010001001 0100010010001001
     // S S S S S S S S   M   L  M   L  M  S   L  M   L  M  S   L  M   L  M
     // => MLMLM(SLMLM)x2

     // do not have sufficient time after final pulse from "wrtsync" call
     // => only write two bytes in "wrtsync", write final pulses directly to save time
     "          ldi   r20, 2\n"             // only write first two bytes of sync
     "          call  wrtsync\n"            // returns 20 cycles after final pulse was written
     "          WRTPS\n"
     "          WRTPL\n"
     "          WRTPM\n"
     "          WRTPL\n"
     "          WRITEPULSE 2\n"             // write medium pulse, returns 10 cycles after pulse was written

     // 8) ---------- ID record plus first 0x4E: 0xFE (cylinder) (side) (sector) (length) (CRC1) (CRC2) 0x4E)
     //
     // 0xA1               ...  0x4E
     //  1 0 1 0 0*0 0 1   ...   0 1 0 0 1 1 1 0
     // 0100010010001001   ...  ??01001001010100
     //  S   L  M   L  M   ...  ?  ?  M  M S S
     // => (write pre-calculated bytes, starting odd)
     // worst case needs 20 cycles before timer is initialized
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          ldi     r21, 0\n"        // (1)   initialize bit counter to fetch next byte
     "          ldi     r26, 8\n"        // (1)   initialize byte counter (8 bytes to write)
     // just wrote a "1" bit => must be followed by either "01" (for "1" bit) or "00" (for "0" bit)
     // (have time to fetch next bit during the leading "0")
     "fio:      dec     r21\n"           // (1)   decrement bit counter
     "          brpl    fio0\n"          // (1/2) skip the following if bit counter >=  0
     "          subi    r26, 1\n"        // (1)   subtract one from byte counter
     "          brmi    fidone\n"        // (1/2) done if byte counter <0
     "          ld	    r20, Z+\n"       // (2)   get next byte
     "          ldi     r21, 7\n"        // (1)   reset bit counter (7 more bits after this first one)
     "fio0:     rol     r20\n"           // (1)   get next data bit into carry
     "          brcs    fio1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "00"
     "          lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %[bitlen]\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          rjmp    fie\n"           // (2)   now even
     // next bit is "1" => write "01"
     "fio1:     WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "01" sequence
     "          rjmp    fio\n"           // (2)   still odd
     // just wrote a "0" bit, (i.e. either "10" or "00") where time for the trailing "0" was already added
     // to the pulse length (have time to fetch next bit during the already-added "0")
     "fie:      dec     r21\n"           // (1)   decrement bit counter
     "          brpl    fie0\n"          // (1/2) skip the following if bit counter >=  0
     "          subi    r26, 1\n"        // (1)   subtract one from byte counter
     "          brmi    fidone\n"        // (1/2) done if byte counter <0
     "          ld	    r20, Z+\n"       // (2)   get next byte
     "          ldi     r21, 7\n"        // (1)   reset bit counter (7 more bits after this first one)
     "fie0:     rol     r20\n"           // (1)   get next data bit into carry
     "          brcs    fie1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "10"
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "10" sequence
     "          rjmp    fie\n"           // (2)   still even
     // next bit is "1" => write "01"
     "fie1:     lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %[bitlen]\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          rjmp    fio\n"           // (2)   now odd
     "fidone:   \n"

     // 9) ---------- 21x 0x4E (post-ID gap)
     //
     // 0x4E             0x4E             ...
     //  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 1001001001010100 1001001001010100 ...
     // S  M  M  M S S   M  M  M  M S S   ...
     // => (MMMMSS)x21
     "          ldi    r20, 21\n"          // (1) write 21 gap bytes
     "          call   wrtgap\n"           //     returns 20 cycles after final pulse was written

     // 10) ---------- 12x 0x00 (96 bits) of "0" (i.e. 96 "10" sequences, i.e. short pulses)
     //
     // 0x4E             0x00             0x00             ...
     //  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
     // 1001001001010100 1010101010101010 1010101010101010 ...
     // S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
     // => MSx95
     "          WRTPM\n"                   // write medium pulse     
     "          ldi     r20, 0\n"        
     "          sts     TCNTL, r20\n"    //       reset timer
     "          ldi     r20, 95\n"       //       initialize counter
     //"wri:      WRITEPULSE 1\n"          //       write short pulse
     "wri:      WRTPS_CHK\n"          //       write short pulse
     "          dec     r20\n"           //       decrement counter
     "          brne    wri\n"           //       repeat until 0

     // first sync "A1": 00100010010001001
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse (this is the missing clock bit)
     "          WRITEPULSE 2\n"          //       write medium pulse
     
     // second sync "A1": 0100010010001001
     "          WRITEPULSE 1\n"          //       write short pulse
     "          WRITEPULSE 3\n"          //       write long pulse
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse (this is the missing clock bit)
     "          WRITEPULSE 2\n"          //       write medium pulse

     // third sync "A1": 0100010010001001
     "          WRITEPULSE 1\n"          //       write short pulse
     "          WRITEPULSE 3\n"          //       write long pulse
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse (this is the missing clock bit)
     "          WRITEPULSE 2\n"          //       write medium pulse

     // start writing data
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          ldi     r21, 0\n"        // (1)   initialize bit counter to fetch next byte

     // just wrote a "1" bit => must be followed by either "01" (for "1" bit) or "00" (for "0" bit)
     // (have time to fetch next bit during the leading "0")
     "wro:      GETNEXTBIT\n"            // (4/9) fetch next data bit into carry
     "          brcs    wro1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "00"
     "          lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %[bitlen]\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          rjmp    wre\n"           // (2)   now even
     // next bit is "1" => write "01"
     "wro1:     WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "01" sequence
     "          rjmp    wro\n"           // (2)   still odd

     // just wrote a "0" bit, (i.e. either "10" or "00") where time for the trailing "0" was already added
     // to the pulse length (have time to fetch next bit during the already-added "0")
     "wre:      GETNEXTBIT\n"            // (4/9) fetch next data bit into carry
     "          brcs    wre1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "10"
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "10" sequence
     "          rjmp    wre\n"           // (2)   still even
     // next bit is "1" => write "01"
     "wre1:     lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %[bitlen]\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          rjmp    wro\n"           // (2)   now odd

     // done writing
     "wdone:    WRITEPULSE\n"            // (9)   wait for and write final pulse

     // 15) ---------- 54/102x 0x4E (post-data gap)
     //
     //  0xF6             0x4E             0x4E             ...
     //  1 1 1 1 0 1 1 0  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 0101010100010100 1001001001010100 1001001001010100 ...
     //  S S S S   L S   M  M  M  M S S   ...
     // => (MMMMSS) x datagaplen
     "          mov    r20, %[gaplen]\n"         // (1) write "datagaplen" gap bytes
     "          call   wrtgap\n"          //     returns 20 cycles after final pulse was written
     "          clr    r26\n" //clear X register for return value
     "          clr    r27\n" //clear X register for return value
     "          rjmp   sec_end\n"         //       done

     // -------------- subroutines

     // write short pulses
     //  r20 contains number of short pulses to write
     //  => takes 6 cycles until timer is initialized (including call)
     //  => returns 20 cycles (max) after final pulse is written (including return statement)
     "wrtshort: WRTPS\n"
     "          dec r20\n"                 // (1)
     "          brne wrtshort\n"           // (1/2)
     "          ret\n"                     // (4)

     // write gap (0x4E) => (MMMMSS) x r20
     //  r20 contains number of gap bytes to write
     //  => takes 6 cycles until timer is initialized (including call)
     //  => returns 20 cycles (max) after final pulse is written (including return statement)
     "wrtgap:   WRTPM\n"
     "wrtgap2:  WRTPM\n"
     "          WRTPM\n"
     "          WRTPM\n"
     "          WRTPS\n"
     "          WRTPS\n"                
     "          dec r20\n"                 // (1)
     "          brne wrtgap\n"             // (1/2)
     "          ret\n"                     // (4)

     // write SYNC (0xA1 with missing clock bit) => MLMLM (SLMLM) x r20
     //  r20 contains nyumber of SYNC bytes to write
     //  => takes 7 cycles until timer is initialized (including call)
     //  => returns 20 cycles (max) after final pulse is written (including return statement)
     "wrtsync:  WRTPM\n"                   // write medium pulse (returns 14 cycles after pulse)
     "          rjmp   sskip\n"            // (2)
     "sloop:    WRTPS\n"                   // write short  pulse
     "sskip:    WRTPL\n"                   // write long   pulse
     "          WRTPM\n"                   // write medium pulse
     "          WRTPL\n"                   // write long   pulse
     "          WRTPM\n"                   // write medium pulse
     "          dec    r20\n"              // (1)
     "          brne   sloop\n"            // (1/2)
     "          ret\n"                     // (4)   return
     
     // wait for pulse to be written
     // => returns 14 cycles (max) after pulse is written (including return statement)
     "waitp:    sbis  TIFR, OCF\n"         // (1/2) skip next instruction if OCFx is set
     "          rjmp  .-4\n"               // (2)   wait more
     "          ldi   r19,  FOC\n"         // (1)
     "          sts   TCCRC, r19\n"        // (2)   set OCP back HIGH (was set LOW when timer expired)
     "          sbi   TIFR, OCF\n"         // (2)   reset OCFx (output compare flag)
     "          ret\n"                     // (4)   return

     "err_exit: ldi   r27, 0xFF\n" //load -1 to X
     "          ldi   r26, 0xFF\n"
     "wr_exit:  in    r19, __SREG__\n" // save status register (interrrupt status)
     "          cli\n" // disable interrupts
     "          out   __SP_L__, r2\n" //restore SPL from r2
     "          out   __SP_H__, r3\n" //restore SPH from r3
     "          out   __SREG__, r19\n" // restore status register (interrupt status)
     "sec_end:  pop   r2\n" //restore r2 from stack
     "          pop   r3\n" //restore r3 from stack
     
     : [retval]"=x"(result)                   // outputs (x=r26/r27)
     : [bitlen]"r"(bitlen), [gaplen]"r"(datagaplen), [buffer]"z"(buffer)      // inputs  (z=r30/r31)
     : "r16", "r17", "r18", "r19", "r20", "r21"); // clobbers

  return result;
}
