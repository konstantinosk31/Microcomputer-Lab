.include "m328PBdef.inc"

.equ FOSC_MHZ=16
.equ DEL_mS=2000
.equ DEL_NU=FOSC_MHZ * DEL_mS
.equ INT_mS=500
.equ INT_NU=FOSC_MHZ * INT_mS

.def DELL=r24
.def DELH=r25
.def TMP=r26
.def COUNTER=r27

; ---- INTERRUPTS ----
.org 0x0
	rjmp reset
.org 0x2
	rjmp ISR0

ISR0:
	push TMP		;
	push DELL		; (this will be used for input)
	push DELH		; Save registers to stack
	in TMP, SREG		;
	push TMP		;
	push COUNTER		;

	; TODO: output
	int DELL, PINB		; Read input
	andi DELL, 15		; Keep 4 LSBs
	clr TMP			; Initialize result
	clr COUNTER		; Initialize counter
check_bit:
	lsr DELL
	brcc skip_inc		; If popped a 0, skip increasing TMP
	inc TMP
skip_inc:
	inv COUNTER
	cpi COUNTER, 4		; If haven't checked 4 bits, repeat
	brne check_bit

	out PORTC, TMP		; Output

	ldi DELL, low(INT_NU)	;
	ldi DELH, high(INT_NU)	; Call delay
	call delay_mS		;

	pop COUNTER		;
	pop TMP			;
	out SREG, TMP		;
	pop DELH		; restore registers from stack
	pop DELL		;
	pop TMP			;

reset: ; configure eternal Interrupts
	ldi TMP, (1 << ISC01) | (1 << ISC00)	; set rising edge
	sts EICRA, TMP				;
	ldi TMP, (1 << INT0)			; enable INT0
	out EIMSK, TMP				;
	sei					; Enable Interrupts

; ---- MAIN CODE ----

; init SP
ldi TMP, LOW(RAMEND)
out SPL, TMP
ldi TMP, HIGH(RAMEND)
out SPH, TMP

; set PORTB as INPUT
clr TMP
out DDRB, TMP
; set PORTC as OUTPUT
ser TMP
out DDRC, TMP

init:
	clr COUNTER		; reset to 0
loop_:
	out COUNTER		; output
	load DELL, low(DEL_NU)	;
	load DELH, high(DEL_NU)	; call delay
	rcall delay_mS		;

	inc COUNTER		; increase

	cpi COUNTER, 32		; If COUNTER exceedes range
	breq init		; then go to beginning
	rjmp loop_		; else loop

delay_mS:	; Given procedure
	ldi TMP, 249
loop_in:
	dec TMP
	nop
	brne loop_inn
	sbiw DELL, 1
	brne delay_mS
	ret
