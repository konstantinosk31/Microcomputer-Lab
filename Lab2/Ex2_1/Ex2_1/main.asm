;
; Ex2_1.asm
;
; Created: 10/16/2024 1:10:26 PM
; Author : User
;

.include "m328PBdef.inc"	; ATmega328pB microcontroller definitions

.equ FOSC_MHZ=16			; Microcontroller operating frequency in MHz

.equ DEL_mS=500				; Delay in mS (valid number from 1 to 4095)

.equ DEL_NU=FOSC_MHZ*DEL_mS	; delay_mS routine: (1000*DEL_NU+6) cycles

.def int1counter=r16
.def pin=r17

.org 0x0
	rjmp reset

.org 0x4
	rjmp isr1

reset:
; Init Stack Pointer
	ldi r24, LOW(RAMEND)
	out SPL, r24
	ldi r24, HIGH(RAMEND)
	out SPH, r24

; Init PORTB as output
	ser r26
	out DDRB, r26

; Init PORTC as output
	ldi r26, 0b00111111
	out DDRC, r26

; Init PORTD as input 
	clr r26
	out DDRD, r26

; Interrupt on rising edge of INT1 pin
	ldi r24, (1<<ISC11) | (1<<ISC10)
	sts EICRA, r24

; Enable the INT1 interrupt
	ldi r24, (1<<INT1)
	out EIMSK, r24

	clr int1counter
	sei ; Enable general flag of interrupts

loop1:
	clr r26
loop2:
	out PORTB, r26

	ldi r24, LOW(DEL_NU)	;
	ldi r25, HIGH(DEL_NU)	; Set delay (number of cycles)
	rcall delay_mS			;

	inc r26

	cpi r26, 16				; compare r26 with 16
	breq loop1
	rjmp loop2

; External interrupt 1 service routine
isr1:
	push r24
	push r25
	in r24, SREG
	push r24
isrloop:
	ldi r24, (1<<INTF1)
	out EIFR, r24
	ldi r24, LOW(5*16)	;
	ldi r25, HIGH(5*16)	; Set delay (number of cycles)
	rcall delay_mS			;
	in r24, EIFR
	sbrc r24, INTF1			; skip next instruction if EIFR & (1<<INTF1) == 0
	rjmp isrloop			; don't continue until EIFR & (1<<INTF1) == 0 to avoid debouncing
	in pin, PIND
	sbrs pin, 5				; skip next instruction if PB5=1 (not pressed)
	rjmp end				; skip interrupt if PB5=0 (pressed)
	inc int1counter
	sbrc int1counter, 6		; skip next instruction if int1counter & (1<<6) == 0
	clr int1counter			; reached 2^6 = 64, return to 0
	out PORTC, int1counter
end:
	pop r24
	out SREG, r24
	pop r25
	pop r24
	reti					; Return from interrupt

; delay of 1000*F1+6 cycles (almost equal to 1000*F1 cycles)
delay_mS:
; total delay of next 4 instruction group = 1+(249*4-1) = 996 cycles
	ldi r23, 249			; (1 cycle)
loop_inn:
	dec r23					; 1 cycle
	nop						; 1 cycle
	brne loop_inn			; 1 or 2 cycles

	sbiw r24, 1				; 2 cycles
	brne delay_mS			; 1 or 2 cycles

	ret						; 4 cycles