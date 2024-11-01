;
; Ex4_1_new.asm
;
; Created: 11/1/2024 12:40:09 PM
; Author : User
;

.include "m328PBdef.inc"

.org 0x0
	rjmp reset
.org 0x2a
	rjmp ADC_INT

.def tmp0 = r16
.def tmp1 = r17
.def val0 = r18
.def val1 = r19
.def i    = r20
.def tmp  = r21
.def argl = r24
.def argh = r25
.def res0 = r26
.def res1 = r27
.def res2 = r28
.def res3 = r29

.equ PD2 = 2
.equ PD3 = 3

ADC_INT:
	push tmp		;
	in tmp, SREG		; Push into stack
	push tmp		;

	lds val0, ADCL		;
	lds val1, ADCH		; Get analog input
	
	; ---Multiply x500---
	ldi tmp0, LOW(500)
	ldi tmp1, HIGH(500)
	clr tmp
	mul val0, tmp0
	movw res0, r0
	clr res2
	clr res3
	mul val0, tmp1
	add res1, r0
	adc res2, r1
	adc res3, tmp
	mul val1, tmp0
	add res1, r0
	adc res2, r1
	adc res3, tmp
	mul val1, tmp1
	add res2, r0
	adc res3, r1

	; Divide by 1024
	ldi i, 10
_loop:
	clc
	ror res3 ror res2 ror res1 ror res0
	dec i
	brne _loop

	out PORTB, res1

	ldi i,3
	; Convert into decimals
convert:
	rcall divide_by10 ; Quotient in res0:1, remainder in r0
	mov tmp, r0
	subi tmp, -0x30	; Convert into char
	push tmp		; Push to stack
	dec i
	brne convert

	; Output
	rcall lcd_clear_display

	pop argl rcall lcd_data		; First digit
	ldi argl, 0x2e rcall lcd_data	; Period
	pop argl rcall lcd_data		; First decimal
	pop argl rcall lcd_data		; Second decimal
	
	ldi argl, LOW(100)
	ldi argh, HIGH(100)
	rcall wait_msec

	lds tmp, ADCSRA		;
	ori tmp, (1 << ADSC); Start new conversion
	sts ADCSRA, tmp		;

	pop tmp		;
	out SREG, tmp	; Pop from stack
	pop tmp		;
	reti
reset:
	ldi tmp, HIGH(RAMEND)
	out SPH, tmp
	ldi tmp, LOW(RAMEND)
	out SPL, tmp

	ser tmp
	out DDRB, tmp
	out DDRD, tmp

	rcall lcd_init
	rcall lcd_clear_display

	sei
	ldi tmp, 0

	ldi tmp, 0b01000001
	sts ADMUX, tmp
	ldi tmp, 0b11001111
	sts ADCSRA, tmp
start:
	inc tmp
	; out PORTB, tmp

	ldi argl, LOW(100)
	ldi argh, HIGH(100)
	rcall wait_msec
	
	rjmp start

divide_by10:
	push tmp		;
	in tmp, SREG	; Push into stack
	push tmp		;
	push argl push argh

	clr argl clr argh ; Store quotient
	; res stores remainder
div_loop:
	tst res1
	brne div_cont
	sbrc res0, 7
		rjmp div_cont
	cpi res0, 10
	brlt div_done
div_cont:
	sbiw res0, 10
	adiw argl, 1
	rjmp div_loop
div_done:
	mov r0, res0
	mov res0, argl mov res1, argh

	pop argh pop argl
	pop tmp		;
	out SREG, tmp	; Pop from stack
	pop tmp		;
	ret

write_2_nibbles:; {{{
	push r24
	; save r24(LCD_Data)
	in r25 ,PIND; read PIND
	andi r25 ,0x0f
	andi r24 ,0xf0
	add r24 ,r25
	out PORTD ,r24;
	; r24[3:0] Holds previus PORTD[3:0]
	; r24[7:4] <-- LCD_Data_High_Byte
	;
	sbi PORTD ,PD3
	nop
	nop
	cbi PORTD ,PD3; Enable Pulse
	pop r24
	swap r24
	andi r24 ,0xf0
	add r24 ,r25
	out PORTD ,r24; Recover r24(LCD_Data)
	;
	; r24[3:0] Holds previus PORTD[3:0]
	; r24[7:4] <-- LCD_Data_Low_Byte
	sbi PORTD ,PD3
	nop
	nop
	cbi PORTD ,PD3; Enable Pulse
	ret; }}}

lcd_data:; {{{
	sbi PORTD ,PD2
	rcall write_2_nibbles
	ldi r24 ,250
	ldi r25 ,0
	rcall wait_usec
	ret; }}}

lcd_command:; {{{
	cbi PORTD ,PD2
	rcall write_2_nibbles
	ldi r24 ,250
	ldi r25 ,0
	rcall wait_usec
	ret; }}}

lcd_clear_display:; {{{
	ldi r24 ,0x01
	rcall lcd_command	; clear display command
	ldi r24 ,low(5)
	ldi r25 ,high(5)
	rcall wait_msec		; Wait 5 mSec
	ret; }}}

lcd_init:; {{{
	ldi r24 ,low(200)
	ldi r25 ,high(200)
	rcall wait_msec;
	; Wait 200 mSec
	;
	ldi r24 ,0x30
	out PORTD ,r24
	sbi PORTD ,PD3
	nop
	nop
	cbi PORTD ,PD3
	ldi r24 ,250
	ldi r25 ,0
	rcall wait_usec; command to switch to 8 bit mode
	;
	; Enable Pulse
	ldi r24 ,0x30
	out PORTD ,r24
	sbi PORTD ,PD3
	nop
	nop
	cbi PORTD ,PD3
	ldi r24 ,250
	ldi r25 ,0
	rcall wait_usec; command to switch to 8 bit mode
	;
	; Enable Pulse
	ldi r24 ,0x30
	out PORTD ,r24
	sbi PORTD ,PD3
	nop
	nop
	cbi PORTD ,PD3
	ldi r24 ,250
	ldi r25 ,0
	rcall wait_usec

	ldi r24 ,0x20
	out PORTD ,r24
	sbi PORTD ,PD3
	nop
	nop
	cbi PORTD ,PD3
	ldi r24 ,250
	ldi r25 ,0
	rcall wait_usec; command to switch to 4 bit mode
	ldi r24 ,0x28
	rcall lcd_command; 5x8 dots, 2 lines
	ldi r24 ,0x0c
	rcall lcd_command
	rcall lcd_clear_display
	ldi r24 ,0x06
	rcall lcd_command
	ret ;}}

wait_msec:
	push r24 ; 2 cycles
	push r25 ; 2 cycles
	ldi r24 , low(999) ; 1 cycle
	ldi r25 , high(999) ; 1 cycle
	rcall wait_usec ; 998.375 usec
	pop r25 ; 2 cycles
	pop r24 ; 2 cycles
	nop ; 1 cycle
	nop ; 1 cycle
	sbiw r24 , 1 ; 2 cycles
	brne wait_msec ; 1 or 2 cycles
	ret ; 4 cycles
wait_usec:
	sbiw r24 ,1 ; 2 cycles (2/16 usec)
	call delay_8cycles ; 4+8=12 cycles
	brne wait_usec ; 1 or 2 cycles
	ret
delay_8cycles:
	nop
	nop
	nop
	ret