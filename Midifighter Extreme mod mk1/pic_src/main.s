;
; This file is released under the GNU General Public License v3
;   http://www.gnu.org/licenses/gpl.html
;
; Author: Daniel Kersten (dkersten@gmail.com, dublindan.posterous.com)
;


; I/O Pins
;
;   PIC PIN(S)  DESCRIPTION
;   A0 - A3     Buttons 1 to 4
;   B4          Bank 1
;   B5 - B6     Bank 3, Bank 4
;   B7 - B10    Shifted global banks 4 to 1
;   B11         Shift button
;   B12 - B15   Global banks 4 to 1
;   A4          Bank 2
;   B0          CLK
;   B1          MOSI
;   B2          SS
;   B3          MISO
;
; LED states sent over SPI in this order:
;         MSB       LSB
; Byte 1:   GGGG BBBB   G = Global Bank buttons, B = Buttons
; Byte 2:   SSSS EEEE   S = Shifted Global Bank buttons, E = External four banks mode buttons
; Byte 3:   ---- ---X   X = Shift button


.include "p24HJ32GP302.inc"
.text
.global __reset
.global __SPI1Interrupt

.equ LED_STATE_A, W10
.equ LED_STATE_B, W11
.equ SPI_NUM, W12
.equ LED_READY, W13

__reset:
	; Set stack limit
	mov 	#0x0a00, W0
        mov 	W0, SPLIM

        ; Set analog inputs as digital
        mov	#0xffff, W0
        mov	W0, AD1PCFGL

        ; Configure Port A
        ; All as outputs
        mov 	#0x0, W0
        mov 	W0, TRISA

        ; Configure Port B
        ; All as outputs except SPI pins (even MISO is set as output! AFAIK hardware SPI overrides this)
        mov	#0xf, W0
        mov	W0, TRISB

        ; Configure Interrupts
        bclr	INTCON1, #0xf ; Disable nested interrupts
        bclr	IEC0, #0xa  ; Disable Interrupt IEC0<10>, will re-enable after SPI is configured

        ; Configure Reprogrammable Peripheral pins
        ; Input CLK (SCK1R), RP0
        mov	#0x0, W0
        ; Input MOSI (SDI1R), RP1
        mov	#0x1, W1
        sl	W0, #0x8, W0
        ior	W1, W0, W0
        mov	W0, RPINR20
        ; Input SS (SSI1R), RP2
        mov	#0x2, W0
        mov	W0, RPINR21
        ; Output MISO, RP3
        mov	#0x700, W0
        mov	W0, RPOR1

        ; Configure SPI1 as slave
        ; Setup procedure (1)
        mov	#0x0, W0
        mov	W0, SPI1BUF    ; Clear SPI1BUF
        ; Setup procedure (2), disable interrupts
        bclr	IFS0, #0xa     ; Clear Interrupt Flag IFS0<10>
        ; Setup procedure (3)
        bclr	SPI1CON1, #0x5 ; Clear Master Mode Enable bit, SPI1CON1<5>
        bclr	SPI1CON1, #0x9 ; Clear SPI1 Data Input Sample Phase bit, SPI1CON1<9>
        bclr 	SPI1CON1, #0xa ; Clear SPI1 16-bit mode bit, SPI1CON1<10>
        bset	SPI1CON1, #0x7 ; Set Slave Select Enable bit, SPI1CON1<7>
        ; Setup procedure (4)
        bclr	SPI1STAT, #0x6 ; Clear Receive Overflow Flag bit, SPI1STAT<6>
        bset	SPI1STAT, #0xf ; Set SPI1 Enable bit, SPI1STAT<15>

        ; Run self test
        call 	self_test

        ; Set up registers
        mov	#0x0, SPI_NUM

        ; Keep W0 as zero register
        mov	#0x0, W0

        ; Enable interrupts
        bclr	IFS0, #0xa     ; Clear Interrupt Flag IFS0<10>
        bset	IEC0, #0xa     ; Enable Interrupt IEC0<10>

        ; Loop forever doing nothing, logic handled in SPI data transfer interrupt
idle_loop:
        nop
        ;pwrsav	#IDLE_MODE
        bra	idle_loop



; self test routine
self_test:
        ; Turn all LEDs off
        mov	#0x0, W1
        mov	W1, PORTA
        mov	W1, PORTB
        call	delay

        ; Turn all LEDs on
        mov	#0x1f, W1
        mov	W1, PORTA
        mov	#0xfff0, W1
        mov	W1, PORTB
        call	delay

        ; Turn all LEDs off
        mov	#0x0, W1
        mov	W1, PORTA
        mov	W1, PORTB
        call	delay

        ; exit self test
        return


; delay routine
delay:	call	delay1
        call 	delay1
        call	delay1
        return
delay1:	mov 	#0xffff, W4
        ; Idle loop
dloop:	dec 	W4, W4
        cp0 	W4
        bra 	NZ, dloop
        return



; SPI1 Transfer Done Interrupt 
__SPI1Interrupt:
        mov	SPI1BUF, W1 ; Read SPI1 data
        ; Zero the SPI buffer
        mov	#0x0, W0
        mov	W0, SPI1BUF

        ; Test which byte is being received
        cp0	SPI_NUM
        bra 	NZ, second_set ; second or third

        ; Second byte next time
        inc	SPI_NUM, SPI_NUM

        ; Output first byte to correct LED pins
        and	W1, #0xf, W0
        mov	W0, PORTA
        mov	#0xf0, W2
        and	W1, W2, W1
        sl	W1, #0x8, W1
        mov	W1, PORTB
        goto	done

second_set:
        ; Check if the second or third byte is being received
        dec	SPI_NUM, W2
        cp0	W2
        bra	NZ, third_set ; Its the third byte

        ; Output byte to correct LEDs
        ; Code below is retarded because it was late at night and I wanted to get it working asap
        ; turns out the bug was on the midifighter side - I was sending the data wrong!
        ; TODO: rewrite this code in a nicer way
        bclr	PORTA, #0x4
        btsc	W1, #0x1
        bset	PORTA, #0x4

        bclr	W1, #0x1
        sl	W1, #0x3, W3
        mov	PORTB, W2
        ior	W3, W2, W2
        mov	W2, PORTB

        btsc	W1, #0x0
        bset	LATB, #0x4
        btsc	W1, #0x2
        bset	LATB, #0x5
        btsc	W1, #0x3
        bset	LATB, #0x6

        ; Advance to next byte
        inc	SPI_NUM, SPI_NUM
        goto	done

third_set:
        ; Output third byte to correct LEDs
        btsc	W1, #0x0
        bset	LATB, #0xb

        ; Reset the byte counter
        mov	#0x0, SPI_NUM
        ; Clear watchdog timer
        clrwdt

done:
        bclr	IFS0, #0xa       ; Clear Interrupt Flag IFS0<10>
        retfie

.end
