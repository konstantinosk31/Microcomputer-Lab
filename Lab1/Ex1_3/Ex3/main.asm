;
; Ex3.asm
;
; Created: 10/13/2024 2:49:41 PM
; Author : User
;

.include "m328PBdef.inc"

.def msl=r24
.def msh=r25
.def il=r26
.def ih=r27

.def TRAIN=r19

.equ Delay_ms=1000

; Replace with your application code
init:
	ldi r26, LOW(RAMEND)
	out SPL, r26
	ldi r26, HIGH(RAMEND)
	out SPH, r26

	;Init PORTD as output
	ser r16
	out DDRD, r16

	set ; LEFT (shifting right)
	ldi TRAIN, 0x80

Output:
	out PORTD, TRAIN
start:
	brtc RIGHT
LEFT: ; T = 1
	rcall wait_x_msec ; wait 1 sec
	clc
	ror TRAIN
	brcc OUTPUT
	rol TRAIN ; reset train
	clt
	rjmp OUTPUT
RIGHT: ; T = 0
	rcall wait_x_msec ; wait 1 sec
	clc
	rol TRAIN
	brcc OUTPUT
	ror TRAIN ; reset train
	set
	rjmp OUTPUT

wait_x_msec: ; Delay_ms*16000 +2-9+4 + 3 overhead = Delay_ms*16000
	ldi msl, LOW(Delay_ms) ; 1 cycle
	ldi msh, HIGH(Delay_ms) ; 1 cycle
delay1: ; Delay_ms*16000 - 9
	rcall wait_approx_1_msec ; 15980 cycles = 15977 cycles + 3 cycles overhead
	sbiw msl, 1 ; 2 cycles
	brne label1 ; 2 or 1 cycles
label1:
	brne label2 ; 2 or 1 cycles
label2:
	brne label3 ; 2 or 1 cycles
label3:
	brne label4 ; 2 or 1 cycles
label4:
	brne label5 ; 2 or 1 cycles
label5:
	brne label6 ; 2 or 1 cycles
label6:
	brne label7 ; 2 or 1 cycles
label7:
	brne label8 ; 2 or 1 cycles
label8:
	brne delay1 ; 2 or 1 cycles
delay1_end:
	ret ; 4 cycles

wait_approx_1_msec: ; 3991*4 - 1 + 14 cycles = 15977 cycles
	push il ; 2 cycles
	push ih ; 2 cycles
	ldi il, LOW(3991) ; 1 cycle
	ldi ih, HIGH(3991) ; 1 cycle
loop: ; x*4 - 1 cycles
	sbiw il, 1 ; 2 cycles
	brne loop ; 2 or 1 cycle
	pop ih ; 2 cycles
	pop il ; 2 cycles
	ret ; 4 cycles