#include <avr/io.h>
;#include "avrFlux.h"

.global fdcReadData
.global fdcWriteData
.global fdcWriteHeader
.global fdcWriteGap

#if defined(__AVR_ATmega328P__)
.equ TIFR,    0x16  ; timer 1 flag register
.equ TOV,     0     ; overflow flag
.equ OCF,     1     ; output compare flag
.equ ICF,     5     ; input capture flag
.equ TCCRC,   0x82  ; timer 1 control register C
.equ FOC,     0x80  ; force output compare flag
.equ TCNTL,   0x84  ; timer 1 counter (low byte)
.equ ICRL,    0x86  ; timer 1 input capture register (low byte)
.equ OCRL,    0x88  ; timer 1 output compare register (low byte)
.equ IDXDDR,  0x0A ; DDRD 
.equ IDXBIT,  7 ; INDEX pin bit (digital pin 7, register PORTD)     
.equ WGPORT,  0x06 ; PINC
.equ WGBIT,   0 ; A0

#elif defined(__AVR_ATmega32U4__)
.equ TIFR,    0x16  ; timer 1 flag register
.equ TOV,     0     ; overflow flag
.equ OCF,     1     ; output compare flag
.equ ICF,     5     ; input capture flag
.equ TCCRC,   0x82  ; timer 1 control register C
.equ FOC,     0x80  ; force output compare flag
.equ TCNTL,   0x84  ; timer 1 counter (low byte)
.equ ICRL,    0x86  ; timer 1 input capture register (low byte)
.equ OCRL,    0x88  ; timer 1 output compare register (low byte)
.equ IDXDDR,  0x0A ; DDRD 
.equ IDXBIT,  7     ; INDEX pin bit (digital pin 6, register PORTD)     
.equ WGPORT,  0x0C  ; PINE accessed via SBIS instruction
.equ WGBIT,   6   ; pin 7

#elif defined(__AVR_ATmega2560__)
.equ TIFR,    0x1A  ; timer 5 flag register
.equ TOV,     0     ; overflow flag
.equ OCF,     1     ; output compare flag
.equ ICF,     5     ; input capture flag
.equ TCCRC,   0x122 ; timer 5 control register C
.equ FOC,     0x80  ; force output compare flag
.equ TCNTL,   0x124 ; timer 5 counter (low byte)
.equ ICRL,    0x126 ; timer 5 input capture register (low byte)
.equ OCRL,    0x128 ; timer 5 output compare register (low byte)
; /TODO/.equ IDXDDR, 0x109 ; INDEX pin register (digital pin 47, register PL2)
; /TODO/.equ IDXBIT,  2     ; INDEX pin bit (digital pin 47, register PL2)
; /TODO/.equ WGPORT,  0x06  ; PINC
; /TODO/.equ WGBIT,   0x0   ; pin 1
#endif

; *FDC_READ_DATA*
; expect at least 10 bytes of 0x00 followed by three sync marks (0xA1 with one missing clock bit)
; Data bits :     0 0 ...0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
; In MFM    : (0)1010...10 0100010010001001 0100010010001001 0100010010001001

; define READPULSE macro (wait for pulse)
; macro arguments: 
;         length: none => just wait for pulse, don't check         ( 9 cycles)
;                 1    => wait for pulse and jump if NOT short  (12/14 cycles)
;                 2    => wait for pulse and jump if NOT medium (14/16 cycles)
;                 3    => wait for pulse and jump if NOT long   (12/14 cycles)
;         dst:    label to jump to if DIFFERENT pulse found
; 
; on entry: r16 contains minimum length of medium pulse
;           r17 contains minimum length of long   pulse
;           r18 contains time of previous pulse
; on exit:  r18 is updated to the time of this pulse
;           r22 contains the pulse length in timer ticks (=processor cycles)     
; CLOBBERS: r19
.macro READPULSE length=0,dst=undefined
             sbis    TIFR, ICF     ; (1/2) skip next instruction if timer input capture seen
             rjmp    .-4           ; (2)   wait more 
             lds     r19, ICRL     ; (2)   get time of input capture (ICR1L, lower 8 bits only)
             sbi     TIFR, ICF     ; (2)   clear input capture flag
             mov     r22, r19      ; (1)   calculate time since previous capture...
             sub     r22, r18      ; (1)   ...into r22
             mov     r18, r19      ; (1)   set r18 to time of current capture
       .if   \length == 1           ;       waiting for short pulse?
             cp      r22, r16      ; (1)   compare r22 to min medium pulse
             brlo   .+2            ; (1/2) skip jump if less
             rjmp   \dst          ; (3)   not the expected pulse => jump to dst
       .else 
         .if   \length == 2         ; waiting for medium pulse?
             cp      r16, r22      ; (1)   min medium pulse < r22? => carry set if so
             brcc    .+2           ; (1/2) skip next instruction if carry is clear
             cp      r22, r17      ; (1)   r22 < min long pulse? => carry set if so
             brcs   .+2            ; (1/2) skip jump if greater
             rjmp   \dst          ; (3)   not the expected pulse => jump to dst
         .else
           .if   \length == 3 
             cp      r22, r17      ; (1)   min long pulse < r22?
             brsh   .+2            ; (1/2) skip jump if greater
             rjmp   \dst          ; (3)   not the expected pulse => jump to dst
           .endif
         .endif
       .endif
.endm

; define STOREBIT macro for storing or verifying data bit 
; storing  data  : 5/14 cycles for 1, 4/13 cycles for 0
; verifying data : 5/15 cycles for 1, 4/14 cycles for 0
.macro STOREBIT data:req,done:req
             lsl     r20           ; (1)   shift received data
     .if \data != 0
             ori     r20, 1        ; (1)   store 1 bit
     .endif
             dec     r21           ; (1)   decrement bit counter
             brne    .+10          ; (1/2) skip if bit counter >0    
             st      Z+, r20       ; (2)   store received data byte
             ldi     r21, 8        ; (1)   re-initialize bit counter
             subi    r26, 1        ; (1)   subtract one from byte counter
             sbci    r27, 0        ; (1) 
             brmi    \done        ; (1/2) done if byte counter<0
.endm
          
; uint8_t fdcReadData(uint8_t bitlen, uint8_t *buffer, unsigned int n)
; bitlen r24:25, buffer r22:23, n 20:21, fourth param r18:r19,  rest: in stack
fdcReadData:					
			push r15		; save r15
			push r16 		; save r16
			movw r30, r22 	; move r22:23(buffer) to Z
			movw r26, r20 	; move r20:21(n) to X
			sbiw r26, 0x01 	; n - 1
; prepare for reading SYNC		 r24:r25(bitlen) -> r24				
             mov         r16, r24   ; (1)   r16 = 2.5 * (MFM bit len) = minimum length of medium pulse
             lsr         r16       ; (1)
             add         r16, r24   ; (1)
             add         r16, r24   ; (1)
             mov         r17, r16  ; (1)   r17 = 3.5 * (MFM bit len) = minimum length of long pulse
             add         r17, r24   ; (1)
             ldi         r24, 0     ; (1)   default return status is S_OK
             mov         r15, r24   ; (1)   initialize timer overflow counter
             sbi         TIFR, TOV ; (2)   reset timer overflow flag

; wait for at least 80x 10 (short) pulse followed by 100 (medium) pulse
ws0:         ldi         r20, 0    ; (1)   initialize short pulse counter
ws1:         sbis        TIFR, TOV ; (1/2) skip next instruction if timer overflow occurred
             rjmp        ws2       ; (2)   continue (no overflow)
             sbi         TIFR, TOV ; (2)   reset timer overflow flag
             dec         r15       ; (1)   overflow happens every 4.096ms, decrement overflow counter
             brne        ws2       ; (1/2) continue if fewer than 256 overflows
             ldi         r24, 3    ; (1)   no sync found in 1.048s => return status is is S_NOSYNC
             rjmp        rdend     ; (2)   done
ws2:         inc         r20       ; (1)   increment short pulse counter
             sbic    WGPORT, WGBIT ; (2) skip next instruction if WRITE_GATE is asserted
             rjmp    rderr         ; exit: read error
             READPULSE             ; (11)   wait for pulse
             cp          r22, r16  ; (1)   pulse length < min medium pulse?
             brlo        ws1       ; (1/2) repeat if so
             cp          r22, r17  ; (1)   pulse length < min long pulse?
             brsh        ws0       ; (1/2) restart if this was a long pulse (expecting medium)
             cpi         r20, 80   ; (1)   did we see at least 80 short pulses?     
             brlo        ws0       ; (1/2) restart if not
    
; expect remaining part of first sync mark (..00010010001001)
             READPULSE   3,ws0     ; (12)  expect long pulse (0001)
             READPULSE   2,ws0     ; (14)  expect medium pulse (001)
             READPULSE   3,ws0     ; (12)  expect long pulse (0001)
             READPULSE   2,ws0     ; (14)  expect medium pulse (001)

; expect second sync mark (0100010010001001)
             READPULSE   1,ws0     ; (12)  expect short pulse (01)
             READPULSE   3,ws0     ; (12)  expect long pulse (0001)
             READPULSE   2,ws0     ; (14)  expect medium pulse (001)
             READPULSE   3,ws0     ; (12)  expect long pulse (0001)
             READPULSE   2,ws0     ; (14)  expect medium pulse (001)

; expect third sync mark (0100010010001001)
             READPULSE   1,ws0     ; (12)  expect short pulse (01)
             READPULSE   3,ws0     ; (12)  expect long pulse (0001)
             READPULSE   2,ws0     ; (14)  expect medium pulse (001)
             READPULSE   3,ws0     ; (12)  expect long pulse (0001)
             READPULSE   2,ws0     ; (14)  expect medium pulse (001)   

; found SYNC => prepare for reading data
             tst     r27           ; (1)   test byte count
             brpl    .+2           ; (1/2) skip following instruction if not negative
             rjmp    rdend         ; (2)   nothing to read (only waiting for sync) => end
             ldi     r21, 8        ; (1)   initialize bit counter (8 bits per byte)

; odd section (previous data bit was 1, no unprocessed MFM bit)
; shortest path: 19 cycles, longest path: 34 cycles
; (longest path only happens when finishing a byte, about every 5-6 pulses)
rdo:         READPULSE             ; (9)   wait for pulse
             cp      r22, r16      ; (1)   pulse length >= min medium pulse?
             brlo    rdos          ; (1/2) jump if not
             cp      r22, r17      ; (1)   pulse length >= min long pulse?
             brlo    rdom          ; (1/2) jump if not

; long pulse (0001) => read 01, still odd
             STOREBIT 0,rddone      ; (4/13) store 0 bit
             STOREBIT 1,rddone      ; (5/14) store 1 bit
             rjmp    rdo            ; (2)    back to start (still odd)

; jump target for relative conditional jumps in STOREBIT macro
rddone:      rjmp    rdend
     
; medium pulse (001) => read 0, now even
rdom:        STOREBIT 0,rddone      ; (4/13) store 0 bit
             rjmp    rde            ; (2)   back to start (now even)

; short pulse (01) => read 1, still odd
rdos:        STOREBIT 1,rddone      ; (5/14) store 1 bit
             rjmp    rdo            ; (2)    back to start (still odd)

; even section (previous data bit was 0, previous MFM 1 bit not yet processed)
; shortest path: 19 cycles, longest path: 31 cycles
rde:         READPULSE             ; (9)   wait for pulse
             cp      r22, r16      ; (1)   pulse length >= min medium pulse?
             brlo    rdes          ; (1/2) jump if not

; either medium pulse (1001) or long pulse (10001) => read 01
; (a long pulse should never occur in this section but it may just be a 
; slightly too long medium pulse so count it as medium)
             STOREBIT 0,rdend      ; (4/13) store 0 bit
             STOREBIT 1,rdend      ; (5/14) store 1 bit
             rjmp    rdo           ; (2)    back to start (now odd)

; short pulse (101) => read 0
rdes:        STOREBIT 0,rdend      ; (5/14) store 0 bit
             rjmp    rde           ; (2)    back to start (still even)

rderr:       ldi     r24, 0XFF         ; return status -1 (write error)
rdend: 
			pop r17 ; restore r17
			pop r16 ; restore r16
			pop r15 ; restore r15
			ret		; return
; outputs: r24:r25(status) -> r24 returns 0(no error), 3(sync error), -1(read error)
; clobbers: r15, r16, r17, r18, r19, r20, r21, r22, X, Z


; *FDC_WRITE_DATA*
; define WRITEPULSE macro (used in write_data and format_track)
.macro WRITEPULSE length=0
       .if \length==1
               sts   OCRL, r16       ; (2)   set OCRxA to short pulse length
       .endif
       .if \length==2
               sts   OCRL, r17       ; (2)   set OCRxA to medium pulse length
       .endif
       .if \length==3
               sts   OCRL, r18       ; (2)   set OCRxA to long pulse length
       .endif
               sbis  TIFR, OCF       ; (1/2) skip next instruction if OCFx is set
               rjmp  .-4             ; (2)   wait more
               ldi   r19,  FOC       ; (1)
               sts   TCCRC, r19      ; (2)   set OCP back HIGH (was set LOW when timer expired)
               sbi   TIFR, OCF       ; (2)   reset OCFx (output compare flag)
.endm
   


; define GETNEXTBIT macro for getting next data bit into carry (4/9 cycles)
; on entry: R20         contains the current byte 
;           R21         contains the bit counter
;           X (R26/R27) contains the byte counter
;           Z (R30/R31) contains pointer to data buffer
.macro GETNEXTBIT
               dec     r21           ; (1)   decrement bit counter
               brpl    .+10          ; (1/2) skip the following if bit counter >=  0
               subi    r26, 1        ; (1)   subtract one from byte counter
               sbci    r27, 0        ; (1) 
               brmi    wdone         ; (1/2) done if byte counter <0
               ld	r20, Z+       ; (2)   get next byte
               ldi     r21, 7        ; (1)   reset bit counter (7 more bits after this first one)
               rol     r20           ; (1)   get next data bit into carry
.endm

; uint8_t fdcWriteData(uint8_t bitlen, uint8_t *buffer, unsigned int n)
; r24:25(bitlen), r22:r23(buffer), r20:r21(n)
fdcWriteData:
			push r16 ; save r16
			push r17 ; save r17
			movw r30, r22 ; move r22:r23(buffer) to z
			movw r26, r20 ; move r20:r21(n) to X
; initialize pulse-length registers (r16, r17, r18)
	           mov   r22, r24 ;save bitlen in r22
               mov   r16, r22         ;       r16 = (2*bitlen)-1 = time for short (01) pulse         
               add   r16, r22
               dec   r16
               mov   r17, r16        ;       r17 = (3*bitlen)-1 = time for medium (001) pulse
               add   r17, r22
               mov   r18, r17        ;       r18 = (4*bitlen)-1 = time for long (0001) pulse
               add   r18, r22
	           ldi   r24, 0x00 ; default return value 0

; write 12 bytes (96 bits) of 0 (i.e. 96 10 sequences, i.e. short pulses)
               ldi     r20, 0        
               sts     TCNTL, r20    ;       reset timer
               ldi     r20, 96       ;       initialize counter
wri:           sbis    WGPORT, WGBIT ; (2) skip next instruction if WRITE_GATE is not asserted
               rjmp    wreq          ; exit: read request							 
               WRITEPULSE 1          ;       write short pulse
               dec     r20           ;       decrement counter
               brne    wri           ;       repeat until 0

; first sync A1: 00100010010001001
               WRITEPULSE 2          ;       write medium pulse
               WRITEPULSE 3          ;       write long pulse
               WRITEPULSE 2          ;       write medium pulse
               WRITEPULSE 3          ;       write long pulse (this is the missing clock bit)
               WRITEPULSE 2          ;       write medium pulse
     
; second sync A1: 0100010010001001
               WRITEPULSE 1          ;       write short pulse
               WRITEPULSE 3          ;       write long pulse
               WRITEPULSE 2          ;       write medium pulse
               WRITEPULSE 3          ;       write long pulse (this is the missing clock bit)
               WRITEPULSE 2          ;       write medium pulse

; third sync A1: 0100010010001001
               WRITEPULSE 1          ;       write short pulse
               WRITEPULSE 3          ;       write long pulse
               WRITEPULSE 2          ;       write medium pulse
               WRITEPULSE 3          ;       write long pulse (this is the missing clock bit)
               WRITEPULSE 2          ;       write medium pulse

; start writing data
               sts     OCRL, r16     ; (2)   set up timer for 01 sequence
               ldi     r21, 0        ; (1)   initialize bit counter to fetch next byte

; just wrote a 1 bit => must be followed by either 01 (for 1 bit) or 00 (for 0 bit)
; (have time to fetch next bit during the leading 0)
wro:           GETNEXTBIT            ; (4/9) fetch next data bit into carry
               brcs    wro1          ; (1/2) jump if 1
; next bit is 0 => write 00
               lds     r19,  OCRL    ; (2)   get current OCRxAL value
               add     r19,  r22     ; (2)   add one-bit time
               sts     OCRL, r19     ; (2)   set new OCRxAL value
               rjmp    wre           ; (2)   now even
; next bit is 1 => write 01
wro1:          WRITEPULSE            ; (7)   wait and write pulse
               sts     OCRL, r16     ; (2)   set up timer for another 01 sequence
               rjmp    wro           ; (2)   still odd

; just wrote a 0 bit, (i.e. either 10 or 00) where time for the trailing 0 was already added
; to the pulse length (have time to fetch next bit during the already-added 0)
wre:           GETNEXTBIT            ; (4/9) fetch next data bit into carry
               brcs    wre1          ; (1/2) jump if 1
; next bit is 0 => write 10
               WRITEPULSE            ; (7)   wait and write pulse
               sts     OCRL, r16     ; (2)   set up timer for another 10 sequence
               rjmp    wre           ; (2)   still even
; next bit is 1 => write 01
wre1:          lds     r19,  OCRL    ; (2)   get current OCRxAL value
               add     r19,  r22     ; (2)   add one-bit time
               sts     OCRL, r19     ; (2)   set new OCRxAL value
               WRITEPULSE            ; (7)   wait and write pulse
               sts     OCRL, r16     ; (2)   set up timer for 01 sequence
               rjmp    wro           ; (2)   now odd

wreq:          ldi r24, 0xFF   ; return -1
; done writing
wdone:         WRITEPULSE            ; (9)   wait for and write final pulse
			pop r17 ; restore r17
			pop r16 ; restore r16
			ret     ; return
; outputs r24:25(status) -> r24 0(ok), -1(read requested)
; clobbers r16, r17, r18, r19, r20, r21, r22, X, Z


; *FDC_WRITE_HEADER*
; 3.5 DD disk:
;   writing 95 + 1 + 65 + (7 + 37 + 515 + 69) * 8 + (7 + 37 + 515) bytes
;   => 5744 bytes per track = 45952 bits
;   data rate 250 kbit/second, rotation rate 300 RPM (0.2s per rotation)
;   => 50000 bits unformatted capacity per track

; 3.5 HD disk:
;   writing 95 + 1 + 65 + (7 + 37 + 515 + 69) * 17 + (7 + 37 + 515) bytes
;   => 5744 bytes per track = 45952 bits
;   data rate 500 kbit/second, rotation rate 300 RPM (0.2s per rotation)
;   => 100000 bits unformatted capacity per track
  
.macro    WRTPS
               sts   OCRL, r16     
               call  waitp
.endm
.macro    WRTPM
               sts   OCRL, r17     
               call  waitp
.endm
.macro    WRTPL
               sts   OCRL, r18     
               call  waitp
.endm

; void fdcWriteHeader(uint8_t bitlen, uint8_t *buffer)
; inputs: r24:r25(bitlen), r22:r23(buffer)
fdcWriteHeader:
			push r16 ; save r16
			push r17 ; save r17
			movw r30, r22 ; move r22:r23(buffer) to Z
; initialize pulse-length registers (r16, r17, r18)
               mov    r16, r24         ; r16 = (2*bitlen)-1 = time for short (01) pulse         
               add    r16, r24
               dec    r16
               mov    r17, r16        ;       r17 = (3*bitlen)-1 = time for medium (001) pulse
               add    r17, r24
               mov    r18, r17        ;       r18 = (4*bitlen)-1 = time for long (0001) pulse
               add    r18, r24
; copy Z register to X register     
               movw    X, Z    
; point X register to sector (buffer+3) in buffer 
               adiw   X, 3 
               ld     r19, X ; load sector to r19
; check if we are sending the first sector, if so track start  
               dec     r19 ; if sector was 1, r19 is now 0 zero flag is set
               breq    trkstart ; Branch if Zero Flag is set   
               rjmp    secstart ; else jump to sector start

; 1) ---------- 56x 0x4E (pre-index gap)
;
; 0x4E             0x4E             ...
;  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
; 1001001001010100 1001001001010100 ...
; M  M  M  M S S   M  M  M  M S S   ...     
; => (MMMMSS)x56

trkstart:      sbi    IDXDDR, IDXBIT ; (2) set index pin to output = low    
               ldi    r20, 56          ; (1) write 56 gap bytes
               call   wrtgap           ;     returns 20 cycles after final pulse was written

; 2) ---------- 12x 0x00
;
; 0x4E             0x00             0x00             ...
;  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
; 1001001001010100 1010101010101010 1010101010101010 ...
; S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
; => MSx95
               WRTPM                   ; write medium pulse
               ldi    r20, 95          ; write 95 short pulses
               call   wrtshort         ; returns 20 cycles after final pulse was written
     
; 3) ---------- 3x SYNC 0xC2
;
; 0x00              0xC2            0xC2             0xC2
;  0 0 0 0 0 0 0 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0
; 1010101010101010 0101001000100100 0101001000100100 0101001000100100
; S S S S S S S S   M S  M   L  M    L S  M   L  M    L S  M   L  M
; => MSMLM(LSMLM)x2
               ldi    r20, 3
               WRTPM                   ; write medium pulse (returns 14 cycles after pulse)
               rjmp   iskip            ; (2)
iloop:         WRTPL                   ; write long   pulse
iskip:         WRTPS                   ; write short  pulse
               WRTPM                   ; write medium pulse
               WRTPL                   ; write long   pulse
               WRTPM                   ; write medium pulse
               dec    r20              ; (1)
               brne   iloop            ; (1/2)

; 4) ---------- index record (0xFC)
;
; 0xC2             0xFC            
;  1 1 0 0*0 0 1 0  1 1 1 1 1 1 0 0
; 0101001000100100 0101010101010010
;  L S  M   L  M    L S S S S S  M 
; => LSSSSSM
               WRTPL                   ; write long pulse (returns 14 cycles after pulse)
               ldi    r20, 5           ; (1) write 5 short pulses
               call   wrtshort         ; 6 cycles until timer update, 20 cycles after pulse
               WRTPM                   ; write medium pulse

; 5) ---------- 50x 0x4E (post-index gap)
;
; 0xFC             0x4E             0x4E             ...
;  1 1 1 1 1 1 0 0  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
; 0101010101010010 1001001001010100 1001001001010100 ...
;  L S S S S S  M  S  M  M  M S S   M  M  M  M S S   ...
; => SMMMSS (MMMMSS)x49
               ldi    r20, 49           ; (1) write 49 gap bytes
               WRTPS                    ; write short  pulse
               call   wrtgap2           ;     returns 20 cycles after final pulse was written
               cbi    IDXDDR, IDXBIT ; (2) set index pin to 0 input = high
     
; 6) ---------- 12x 0x00
;
; 0x4E             0x00             0x00             ...
;  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
; 1001001001010100 1010101010101010 1010101010101010 ...
; S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
; => MSx95     
secstart:      WRTPM                   ; write medium pulse
               ldi    r20, 95          ; write 95 short pulses
               call   wrtshort         ; returns 20 cycles after final pulse was written
     
; 7) ---------- 3x SYNC 0xA1
;
;  0x00             0xA1             0xA1             0xA1
;  0 0 0 0 0 0 0 0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
; 1010101010101010 0100010010001001 0100010010001001 0100010010001001
; S S S S S S S S   M   L  M   L  M  S   L  M   L  M  S   L  M   L  M
; => MLMLM(SLMLM)x2

; do not have sufficient time after final pulse from wrtsync call
; => only write two bytes in wrtsync, write final pulses directly to save time
               ldi   r20, 2             ; only write first two bytes of sync
               call  wrtsync            ; returns 20 cycles after final pulse was written
               WRTPS
               WRTPL
               WRTPM
               WRTPL
               WRITEPULSE 2             ; write medium pulse, returns 10 cycles after pulse was written

; 8) ---------- ID record plus first 0x4E: 0xFE (cylinder) (side) (sector) (length) (CRC1) (CRC2) 0x4E)
;
; 0xA1               ...  0x4E
;  1 0 1 0 0*0 0 1   ...   0 1 0 0 1 1 1 0
; 0100010010001001   ...  ??01001001010100
;  S   L  M   L  M   ...  ?  ?  M  M S S
; => (write pre-calculated bytes, starting odd)
; worst case needs 20 cycles before timer is initialized
               sts     OCRL, r16     ; (2)   set up timer for 01 sequence
               ldi     r21, 0        ; (1)   initialize bit counter to fetch next byte
               ldi     r26, 8        ; (1)   initialize byte counter (8 bytes to write)
; just wrote a 1 bit => must be followed by either 01 (for 1 bit) or 00 (for 0 bit)
; (have time to fetch next bit during the leading 0)
fio:           dec     r21           ; (1)   decrement bit counter
               brpl    fio0          ; (1/2) skip the following if bit counter >=  0
               subi    r26, 1        ; (1)   subtract one from byte counter
               brmi    fidone        ; (1/2) done if byte counter <0
               ld	    r20, Z+       ; (2)   get next byte
               ldi     r21, 7        ; (1)   reset bit counter (7 more bits after this first one)
fio0:          rol     r20           ; (1)   get next data bit into carry
               brcs    fio1          ; (1/2) jump if 1
; next bit is 0 => write 00
               lds     r19,  OCRL    ; (2)   get current OCRxAL value
               add     r19,  r24     ; (2)   add one-bit time
               sts     OCRL, r19     ; (2)   set new OCRxAL value
               rjmp    fie           ; (2)   now even
; next bit is 1 => write 01
fio1:          WRITEPULSE            ; (7)   wait and write pulse
               sts     OCRL, r16     ; (2)   set up timer for another 01 sequence
               rjmp    fio           ; (2)   still odd
; just wrote a 0 bit, (i.e. either 10 or 00) where time for the trailing 0 was already added
; to the pulse length (have time to fetch next bit during the already-added 0)
fie:           dec     r21           ; (1)   decrement bit counter
               brpl    fie0          ; (1/2) skip the following if bit counter >=  0
               subi    r26, 1        ; (1)   subtract one from byte counter
               brmi    fidone        ; (1/2) done if byte counter <0
               ld	    r20, Z+       ; (2)   get next byte
               ldi     r21, 7        ; (1)   reset bit counter (7 more bits after this first one)
fie0:          rol     r20           ; (1)   get next data bit into carry
               brcs    fie1          ; (1/2) jump if 1
; next bit is 0 => write 10
               WRITEPULSE            ; (7)   wait and write pulse
               sts     OCRL, r16     ; (2)   set up timer for another 10 sequence
               rjmp    fie           ; (2)   still even
; next bit is 1 => write 01
fie1:          lds     r19,  OCRL    ; (2)   get current OCRxAL value
               add     r19,  r24     ; (2)   add one-bit time
               sts     OCRL, r19     ; (2)   set new OCRxAL value
               WRITEPULSE            ; (7)   wait and write pulse
               sts     OCRL, r16     ; (2)   set up timer for 01 sequence
               rjmp    fio           ; (2)   now odd
fidone:   

; 9) ---------- 21x 0x4E (post-ID gap)
;
; 0x4E             0x4E             ...
;  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
; 1001001001010100 1001001001010100 ...
; S  M  M  M S S   M  M  M  M S S   ...
; => (MMMMSS)x21
               ldi    r20, 21          ; (1) write 21 gap bytes
               call   wrtgap           ;     returns 20 cycles after final pulse was written
               rjmp   secend ; jmp end

; -------------- subroutines

; write short pulses
;  r20 contains number of short pulses to write
;  => takes 6 cycles until timer is initialized (including call)
;  => returns 20 cycles (max) after final pulse is written (including return statement)
wrtshort:      WRTPS
               dec r20                 ; (1)
               brne wrtshort           ; (1/2)
               ret                     ; (4)

; write gap (0x4E) => (MMMMSS) x r20
;  r20 contains number of gap bytes to write
;  => takes 6 cycles until timer is initialized (including call)
;  => returns 20+2 cycles (max) after final pulse is written (including return statement)
wrtgap:        WRTPM
wrtgap2:       WRTPM
               WRTPM
               WRTPM
               WRTPS
               WRTPS       
               sbis    WGPORT, WGBIT ; (2) skip next instruction if WRITE_GATE is not asserted
               rjmp    secend          ; exit: read request								          
               dec r20                 ; (1)
               brne wrtgap             ; (1/2)
               ret                     ; (4)

; write SYNC (0xA1 with missing clock bit) => MLMLM (SLMLM) x r20
;  r20 contains nyumber of SYNC bytes to write
;  => takes 7 cycles until timer is initialized (including call)
;  => returns 20 cycles (max) after final pulse is written (including return statement)
wrtsync:       WRTPM                   ; write medium pulse (returns 14 cycles after pulse)
               rjmp   sskip            ; (2)
sloop:         WRTPS                   ; write short  pulse
sskip:         WRTPL                   ; write long   pulse
               WRTPM                   ; write medium pulse
               WRTPL                   ; write long   pulse
               WRTPM                   ; write medium pulse
               dec    r20              ; (1)
               brne   sloop            ; (1/2)
               ret                     ; (4)   return
     
; wait for pulse to be written
; => returns 14 cycles (max) after pulse is written (including return statement)
waitp:         sbis  TIFR, OCF         ; (1/2) skip next instruction if OCFx is set
               rjmp  .-4               ; (2)   wait more
               ldi   r19,  FOC         ; (1)
               sts   TCCRC, r19        ; (2)   set OCP back HIGH (was set LOW when timer expired)
               sbi   TIFR, OCF         ; (2)   reset OCFx (output compare flag)
               ret                     ; (4)   return

secend:
              pop r17 ; restore r17
              pop r16 ; restore r16
              ret     ; return
     
; no outputs
; clobbers r16, r17, r18, r19, r20, r21, r26, r27, X, Z


; *FDC_WRITE_GAP*
; void fdcWriteGap(uint8_t bitlen, uint8_t gaplen)
; inputs: r24:r25(bitlen), r22:r23(gaplen)
fdcWriteGap:
              push r16 ; save r16
              push r17 ; save r17              
; initialize pulse-length registers (r16, r17, r18)
               mov    r16, r24         ; r16 = (2*bitlen)-1 = time for short (01) pulse         
               add    r16, r24
               dec    r16
               mov    r17, r16        ;       r17 = (3*bitlen)-1 = time for medium (001) pulse
               add    r17, r24
               mov    r18, r17        ;       r18 = (4*bitlen)-1 = time for long (0001) pulse
               add    r18, r24
    
; 15) ---------- 54/102x 0x4E (post-data gap)
;
;  0xF6             0x4E             0x4E             ...
;  1 1 1 1 0 1 1 0  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
; 0101010100010100 1001001001010100 1001001001010100 ...
;  S S S S   L S   M  M  M  M S S   ...
; => (MMMMSS) x datagaplen
               mov    r20, r22        ; (1) write datagaplen gap bytes
               call   wrtgap          ;     returns 20 cycles after final pulse was written
      
; no outputs
; clobbers r16, r17, r18, r19, r20, Z
