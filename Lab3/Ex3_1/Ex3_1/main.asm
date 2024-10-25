;
; Ex3_1.asm
;
; Created: 10/25/2024 5:06:29 PM
; Author : User
;

.def temp = r16
.def DC_VALUE = r17
.def DC_INC = r18 ; DC_INC = 1 -> DC_VALUE increasing, DC_INC = 0 -> DC_VALUE decreasing
.def i = r19

DUTY: .DB 5, 26, 46, 66, 87, 107, 128, 148, 168, 189, 209, 230, 250 ; 255*(2+8k)/100, k = 0 ... 12. Initial k = 6.
.equ DUTY_LEN = 13
.equ DUTY_START = 6

init:
	; Init stack pointer
	ldi temp, high(RAMEND)
	out SPH, temp
	ldi temp, low(RAMEND)
	out SPL, temp

	; Set PORTB as output
	ser temp
	out DDRD, temp

	; Fast PWM, 8 bit, non-inverting output, N = 256. BOTTOM = 0, TOP = 0x00ff = 255
	ldi temp, (1<<WGM10) | (1<<COM1A1)
	sts TCCR1A, temp
	ldi temp, (1<<WGM12) | (1<<CS12)
	sts TCCR1B, temp

	ldi DC_VALUE, 128
	sts OCR1AL, DC_VALUE

	; Initialize i to duty cycle starting position: 6 (50%)
	ldi i, DUTY_START

	; Load the starting address of the duty cycle value into Z
	LDI ZH, HIGH(DUTY+DUTY_START)
	LDI ZL, LOW(DUTY+DUTY_START)

	; Init DC_INC to 1 (increasing DC_VALUE)
	ldi DC_INC, 1

; Replace with your application code
start:
	lpm DC_VALUE, Z
	sts OCR1AL, DC_VALUE
	ldi r24, LOW(10*16)	;
	ldi r25, HIGH(10*16)	; Set delay (msec * 16)
	rcall delay_mS

    cpi DC_INC, 1
	brne decreasing
increasing:
	inc i
	adiw Z, 1
	cpi i, DUTY_LEN
	breq decreasing

	rjmp start
decreasing:
	dec i
	sbiw Z, 1
	brmi increasing

    rjmp start
	
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