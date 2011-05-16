
; I/O Pins
;
;   IDX     LED NUM(S)  PIC PIN(S)  DESCRIPTION
;   0 - 3   L1 - L4     B0 - B3     Buttons 1 to 4
;   4       B1          B4          Bank 1
;   5 - 6   B3 - B4     B5 - B6     Bank 3, Bank 4
;   7 - 10  S4 - S1     B7 - B10    Shifted global banks 4 to 1
;   11      Shift       B11         Shift button
;   12 - 15 G4 - G1     B12 - B15   Global banks 4 to 1
;   16      B2          A4          Bank 2
;
; Pin #    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
;
; PORTA =   -   -   -   -   -   -   -   -   -   -   -  B2 ACK DAT CLK  SS
;           -   -   -   -   -   -   -   -   -   -   - out out  in  in  in
; PORTB =  G1  G2  G3  G4 SHF  S1  S2  S3  S4  B4  B3  B1  L4  L3  L2  L1
;         out out out out out out out out out out out out out out out out
;

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
		mov		#0xffff, W0
		mov		W0, AD1PCFGL

		; Configure Port A
		; All as outputs
		mov 	#0x0, W0
		mov 	W0, TRISA

		; Configure Port B
		; All as outputs except SPI pins CLK, MOSI and SS (0111b)
;		mov		#0x7, W0
		mov		#0xf, W0
		mov		W0, TRISB

		; Configure Interrupts
		bclr	INTCON1, #0xf ; Disable nested interrupts
		bclr	IEC0, #0xa  ; Disable Interrupt IEC0<10>, will re-enable after SPI is configured

		; Configure Reprogrammable Peripheral pins
		; Input CLK (SCK1R), RP0
		mov		#0x0, W0
		; Input MOSI (SDI1R), RP1
		mov		#0x1, W1
		sl		W0, #0x8, W0
		ior		W1, W0, W0
		mov		W0, RPINR20
		; Input SS (SSI1R), RP2
		mov		#0x2, W0
		mov		W0, RPINR21
		; Output MISO, RP3
		mov		#0x700, W0
		mov		W0, RPOR1		

		; Configure SPI1 as slave
		; Setup procedure (1)
		mov		#0x0, W0
		mov		W0, SPI1BUF    ; Clear SPI1BUF
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
		mov		#0x0, SPI_NUM

		; Keep W0 as zero register
		mov		#0x0, W0

		; Enable interrupts
		bclr	IFS0, #0xa     ; Clear Interrupt Flag IFS0<10>
		bset	IEC0, #0xa     ; Enable Interrupt IEC0<10>

		; Loop forever doing nothing, logic handled in SPI data transfer interrupt
idle_loop:
		nop
		;pwrsav	#IDLE_MODE
		bra		idle_loop



; self test routine
self_test:
		mov		#0x0, W1
		mov		W1, PORTA
		mov		W1, PORTB
		call	delay

		mov		#0x1f, W1
		mov		W1, PORTA
		mov		#0xfff0, W1
		mov		W1, PORTB
		call	delay

		mov		#0x0, W1
		mov		W1, PORTA
		mov		W1, PORTB
		call	delay

		; exit self test
		return


; delay routine
delay:	call	delay1
		call 	delay1
		call	delay1
		return
delay1:	mov 	#0xffff, W4
dloop:	dec 	W4, W4
		cp0 	W4
		bra 	NZ, dloop
		return



; SPI1 Transfer Done Interrupt 
__SPI1Interrupt:
		mov		SPI1BUF, W1 ; Read SPI1 data
		mov		#0x0, W0
		mov		W0, SPI1BUF

		cp0		SPI_NUM
		bra 	NZ, second_set

		inc		SPI_NUM, SPI_NUM

		and		W1, #0xf, W0
		mov		W0, PORTA
		mov		#0xf0, W2
		and		W1, W2, W1
		sl		W1, #0x8, W1
		mov		W1, PORTB
		goto	done

second_set:
		dec		SPI_NUM, W2
		cp0		W2
		bra		NZ, third_set

		bclr	PORTA, #0x4
		btsc	W1, #0x1
		bset	PORTA, #0x4

		bclr	W1, #0x1
		sl		W1, #0x3, W3
		mov		PORTB, W2
		ior		W3, W2, W2
		mov		W2, PORTB

		btsc	W1, #0x0
		bset	LATB, #0x4
		btsc	W1, #0x2
		bset	LATB, #0x5
		btsc	W1, #0x3
		bset	LATB, #0x6

		inc		SPI_NUM, SPI_NUM
		goto	done

third_set:
		btsc	W1, #0x0
		bset	LATB, #0xb

		mov		#0x0, SPI_NUM
		clrwdt

done:
		bclr	IFS0, #0xa       ; Clear Interrupt Flag IFS0<10>
		retfie

.end
