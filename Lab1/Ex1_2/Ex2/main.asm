;
; Ex2.asm
;
; Created: 10/13/2024 2:48:57 PM
; Author : User
;

.include "m328PBdef.inc"

.def a=r16
.def b=r17
.def c=r18
.def d=r19
.def nota=r20
.def notb=r21
.def notc=r22
.def notd=r23
.def f0=r24
.def f1=r25
.def i=r26
.def temp=r27

; Replace with your application code
start:
    ldi a, 0x51
	ldi b, 0x41
	ldi c, 0x21
	ldi d, 0x01
	ldi i, 6
loop:
	mov nota, a
	com nota
	mov notb, b
	com notb
	mov notc, c
	com notc
	mov notd, d
	com notd
	
	mov f0, a
	and f0, notb
	mov temp, notb
	and temp, d
	or f0, temp
	com f0

	mov f1, a
	or f1, notc
	mov temp, b
	or temp, notd
	and f1, temp

	inc a
	subi b, -2 ; since there is no addi, we subtract its opposite
	subi c, -3 ; similarly
	subi d, -4 ; similarly

	dec i
	brne loop
