;
; Ex2_3_asm.asm
;
; Created: 10/16/2024 2:28:35 PM
; Author : User
;

.include "m328PBdef.inc"

.equ FOSC_MHZ=16
.equ FLASH_MS=500
.equ DEL_MS=4500

.def CNT=r24 ; count number of delays that have happened
.def CNT_HI=r25
.def TMP=r26
.def TMP_HI=r27
.def FLG=r28 ; 0 --> Closed, 1 --> PB0, 2 --> All

; ---- INTERRUPTS ----
.org 0x0
	rjmp reset
.org 0x4
	rjmp ISR1

ISR1:
	push TMP
	in TMP, SREG
	push TMP

	ldi FLG, 2
	ldi CNT   , low (FLASH_MS)
	ldi CNT_HI, high(FLASH_MS)

	pop TMP
	out SREG, TMP
	pop TMP

	reti
	
; ---- MAIN CODE ----
delay_mS:	; Given procedure, altered so it runs for 1ms (no args)
	ldi TMP, low(3999)
	ldi TMP_HI, high(3999)
loop_inn:
	sbiw TMP, 1
	brne loop_inn
	ret

reset: ; configure eternal Interrupts
	ldi TMP, (1 << ISC11) | (1 << ISC10)	; set rising edge
	sts EICRA, TMP				;
	ldi TMP, (1 << INT1)			; enable INT1
	out EIMSK, TMP				;
	sei					; enable Interrupts

; init SP
ldi TMP, LOW(RAMEND)
out SPL, TMP
ldi TMP, HIGH(RAMEND)
out SPH, TMP

; set PORTB as OUTPUT
ser TMP
out DDRB, TMP
; out DDRC, TMP
; set PORTD as INPUT
clr TMP
out DDRD, TMP

init:
	clr FLG			;
	ldi CNT   , low(DEL_MS)	; load 4.5s (no matter if FLG = 0 or 1)
	ldi CNT_HI, high(DEL_MS);

loop_:
	sei
	cpi FLG, 1		; Check FLG:
	breq one_bit		;   if FLG == 1, open 1 bit
	brlo no_bits		;   else if FLG < 1, open 0 bits
all_bits:			;   else open all bits
	ser TMP
	out PORTB, TMP
	jmp cont
no_bits:
	clr TMP
	out PORTB, TMP
	jmp cont
one_bit:
	ldi TMP, 1
	out PORTB, TMP
	jmp cont
cont:
	rcall delay_mS		; run delay
	sbiw CNT, 1			; decrease counter
	brne loop_		; if counter != 0, skip FLG updates
	dec FLG			; decrease flag
	brpl foo		; if N == 1 (FLG < 0)
	ldi FLG, 0		;   then FLG = 0
foo:
	cli
	cpi FLG, 2
	breq loop_
	ldi CNT   , low(DEL_MS)	; load 4.5s (no matter if FLG = 0 or 1)
	ldi CNT_HI, high(DEL_MS);
	sei
	jmp loop_
