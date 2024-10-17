/*
 * main.c
 *
 * Created: 10/16/2024 4:46:07 PM
 *  Author: User
 */ 

#include <xc.h>

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define FLG_OFF 0
#define FLG_ONE 1
#define FLG_ALL 2

#define CNT_ALL 500
#define CNT_ONE 4500

int flg = FLG_OFF, count = 0;

ISR (INT1_vect)
{
	flg = FLG_ALL, count = CNT_ALL;
}

#define max(a, b) ((a) > (b) ? (a) : (b))

int main ()
{
	EICRA = (1 << ISC11) | (1 << ISC10);
	EIMSK = 1 << INT1;
	sei();

	DDRB = 0xff; // input
	DDRD = 0x00; // output

	while (1) {
		while (count--) {
			PORTB = (flg == FLG_OFF) ? 0x00 :
			(flg == FLG_ONE) ? 0x01 : 0x0ff;
			_delay_ms(1);
		}
		flg = max(flg - 1, 0);
		count = CNT_ONE;
	}
}
