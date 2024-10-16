.include "m328PBdef.inc"

.equ FOSC_MHZ=16
.equ FLASH_MS=500
.equ DEL_MS=5000 - FLASH_MS

.def CNT=r24 ; count number of delays that have happened
.def CNT_HI=r25
.def TMP=r26
.def FLG=r27 ; 0 --> Closed, 1 --> PB0, 2 --> All

; ---- INTERRUPTS ----
.org 0x0
	rjmp reset
.org 0x2
	rjmp ISR1

ISR1:
	ldi FLG, 2
	ldi CNT   , low (FLASH_MS)
	ldi CNT_HI, high(FLASH_MS)
	reti
	
; ---- MAIN CODE ----
delay_mS:	; Given procedure, altered so it runs for 1 ms (no args)
	ldi TMP, 249
loop_in:
	dec TMP
	nop
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
; set PORTD as INPUT
clr TMP
out DDRC, TMP

init:
	clr FLG			;
	clr CNT			; zero-out remaining open-time
	clr CNT_HI		;

loop_:
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
	call delay_mS		; run delay
	dec CNT			; decrease counter
	brne cont2		; if counter != 0, skip FLG updates
	dec FLG			; decrease flag
	sbrc 0			; if C == 1 (FLG < 0)
	ldi FLG, 0		;   then FLG = 0
	ldi CNT   , low(DEL_MS)	; load 4.5s (no matter if FLG = 0 or 1)
	ldi CNT_HI, high(DEL_MS);
cont2:	jmp loop_
