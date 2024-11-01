.include "m328PBdef.inc"

.org 0x0
	rjmp reset
.org 0x2A
	rjmp adcisr

.def tmp  = r16
.def lcd  = r17
.def val0 = r18
.def val1 = r19
.def tmp0 = r20
.def tmp1 = r21
.def res0 = r22
.def res1 = r23
.def argl = r24
.def argh = r25
.def res2 = r26
.def res3 = r27
.def i    = r28
.def k    = r29

adcisr:
	push tmp		;
	in tmp, SREG		; Push into stack
	push tmp		;

	lds val0, ADCL		;
	lds val1, ADCH		; Get analog input
	
	; Multiply x500{{{
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
	adc res3, r1; }}}

	; Divide by 1024
	ldi i, 10
_loop:	clc
	ror res2 ror res1 ror res0
	dec i
	brne _loop
	; HAVE THE ANSWER, 4 digits (1 + comma + 2 decimals)

	ldi i,3
	; Convert into decimals
convert:
	rcall divide_by10 ; Quotient in res0:1, remainder in r0:1
	adi r0, 0x30	; Convert into char
	push r0		; Push to stack
	dec i
	brne convert

	; Output
	rcall lcd_clear_display
	pop argl rcall lcd_data		; First digit
	mvi argl, 0x2e rcall lcd_data	; Period
	pop argl rcall lcd_data		; First decimal
	pop argl rcall lcd_data		; Second decimal

	pop tmp		;
	out tmp, SREG	; Pop from stack
	pop tmp		;
	ret
reset:
	ldi tmp, high(RAMEND)	;
	out SPH, tmp		; Initialize SP
	ldi tmp, low(RAMEND)	;
	out SPL, tmp		;

	set tmp			; Set PORTD (LSD) as output
	out DDRD, tmp		;

	rcall lcd_init		; Initialize LCD
	rcall delay_ms		; Hold time 100ms

	ldi tmp, 0b01000001	; Set ADC1: 5V, Right aligned, ADC1
	sts ADMUX, tmp		;
	ldi tmp, 0b10001111	; Enable, Interrupts, 125kHz
	sts ADCSRA, tmp

	sei			; Enable interrupts
	ldi k, 10

start:
	lds tmp, ADCSRA		; 
	ori tmp, (1 << ADSC)	; Begin conversion
st_loop:
	ldi r24, LOW(100*16)	; Set delay time to 100ms
	ldi r25, HIGH(100*16)	;
	rcall delay_ms		; Loop with delay
	dec k
	brne st_loop
	sts ADCSRA, tmp
	rjmp start

divide_by10:
	push tmp		;
	in tmp, SREG		; Push into stack
	push tmp		;

	clr r0
	clr r1
div_loop:
	tst res1
	brne div_cont
	cpi res0, 10
	brlt div_done
div_cont:
	sbiw res0, 10
	adiw r0, 1
	rjmp div_loop
div_done:
	mov tmp,r0 mov r0,res0 mov res0,tmp
	mov tmp,r1 mov r1,res1 mot res1,tmp
	pop tmp		;
	out tmp, SREG	; Pop from stack
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
	ret; }}}

delay_mS: ; delay of 1000*F1+6 cycles (almost equal to 1000*F1 cycles){{{
; total delay of next 4 instruction group = 1+(249*4-1) = 996 cycles
	ldi r23, 249			; (1 cycle)
loop_inn:
	dec r23					; 1 cycle
	nop						; 1 cycle
	brne loop_inn			; 1 or 2 cycles

	sbiw r24, 1				; 2 cycles
	brne delay_mS			; 1 or 2 cycles

	ret						; 4 cycles}}}
