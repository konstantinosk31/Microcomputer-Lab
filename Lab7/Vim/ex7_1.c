#include <xc.h>

#define F_CPU 16000000UL
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

typedef uint8_t bool;
#define true 1
#define false 0

#define SET(x, b)   do { (x) |=   1 << (b);  } while (0)
#define RESET(x, b) do { (x) &= ~(1 << (b)); } while (0)

/* TEMPLATE CODE FROM GIVEN ASSEMBLY */
/* {{{ */
bool one_wire_reset (void)
{
	SET(DDRD, 4); // set output

	CLEAR(PORTD, 4); // sent 0
	_delay_us(480);

	CLEAR(DDRD, 4); // set input
	CLEAR(PORTD, 4); // disable pull-up
	_delay_us(100);

	int tmp = PIND; // read PORTD
	_delay_us(380);
	return (tmp & (1 << 4)) == 0;
}

uint8_t one_wire_receive_bit (void)
{
	SET(DDRD, 4); // Set output
	CLEAR(PORTD, 4);
	_delay_us(2);

	CLEAR(DDRD, 4); // set input
	CLEAR(PORTD, 4); // disable pull-up
	_delay_us(10);

	uint8_t ret = PIND & 1;
	_delay_us(49);
	return ret;
}

void one_wire_transmit_bit (uint8_t in)
{
	SET(DDRD, 4); // Set output
	CLEAR(PORTD, 4);
	_delay_us(2);

	in &= 1; // keep only LSB
	if (in) SET(PORTD, 4);
	else    CLEAR(PORTD, 4);

	_delay_us(58);
	CLEAR(DDRD, 4); // set input
	CLEAR(PORTD, 4); // disable pull-up
	_delay_us(1);
}

uint8_t one_wire_receive_byte (void)
{
	uint8_t ret = 0;
	for (int i=0; i<8; ++i)
		ret |= (one_wire_receive_bit() << i);
	return ret;
}

void one_wire_transmit_byte (uint8_t in)
{
	for (int i=0; i<8; ++i, in >>= 1)
		one_wire_transmit_bit(in);
}
/* }}} */

float read_temp (void)
{
	int16_t ret = 0;

	if (one_wire_reset() != 0) return 0x8000;
	one_wire_transmit_byte(0xCC);
	one_wire_transmit_byte(0x44);
	while (one_wire_receive_bit() != 1);

	if (one_wire_reset() != 0) return 0x8000;
	one_wire_transmit_byte(0xCC);
	one_wire_transmit_byte(0xBE);

	ret |= one_wire_receive_byte() << 8;
	ret |= one_wire_receive_byte();
	return (float)(ret) / 2;
}

int main(void)
{
	return 0;
}

