#ifndef _DS18B20_H_
#define _DS18B20_H_

#include "utils.h"

#define SET(x, b)   do { (x) |=   1 << (b);  } while (0)
#define CLEAR(x, b) do { (x) &= ~(1 << (b)); } while (0)

/* TEMPLATE CODE FROM GIVEN ASSEMBLY */
/* {{{ */
static bool one_wire_reset (void)
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

static uint8_t one_wire_receive_bit (void)
{
	SET(DDRD, 4); // Set output
	CLEAR(PORTD, 4);
	_delay_us(2);

	CLEAR(DDRD, 4); // set input
	CLEAR(PORTD, 4); // disable pull-up
	_delay_us(10);

	uint8_t ret = PIND & (1 << 4);
	_delay_us(49);
	return ret >> 4;
}

static void one_wire_transmit_bit (uint8_t in)
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

static uint8_t one_wire_receive_byte (void)
{
	uint8_t ret = 0;
	for (int i=0; i<8; ++i)
		ret |= (one_wire_receive_bit() << i);
	return ret;
}

static void one_wire_transmit_byte (uint8_t in)
{
	for (int i=0; i<8; ++i, in >>= 1)
		one_wire_transmit_bit(in);
}
/* }}} */

#define TEMP_ERR 0x8000

int16_t read_temp (void)
{
	int16_t ret = 0;

	if (one_wire_reset() != 1) return TEMP_ERR;
	one_wire_transmit_byte(0xCC);
	one_wire_transmit_byte(0x44);
	while (one_wire_receive_bit() != 1);

	if (one_wire_reset() != 1) return TEMP_ERR;
	one_wire_transmit_byte(0xCC);
	one_wire_transmit_byte(0xBE);

	ret |= one_wire_receive_byte();
	ret |= one_wire_receive_byte() << 8;
	return ret;
}

#define TEMP(raw) ((raw) >> PRECISION)

void add_to_temp(int16_t *temp, int8_t add)
{
    *temp += (add << PRECISION);
}

char *temp_to_str(int16_t val, int decimals)
{
	char *temp_buf = malloc(64);
	char tmp[17];
	int idx = 0, pos = 0;

	if (val < 0) {
		temp_buf[pos++] = '-';
		val = -val;
	}

	int   int_part  = val >> PRECISION;
	float frac_part =  ((float)(val) / (1 << PRECISION)) - int_part;

	if (int_part) {
		for (; int_part; int_part /= 10)
			tmp[idx++] = (int_part % 10) + '0';
		for (idx -= 1; idx >= 0; --idx)
			temp_buf[pos++] = tmp[idx];
	} else {
		temp_buf[pos++] = '0';
	}

	temp_buf[pos++] = '.';
	for (int i=0; i<decimals; ++i) {
		frac_part *= 10;
		temp_buf[pos++] = (int)(frac_part) + '0';
		frac_part -= (int)(frac_part);
	}

	temp_buf[pos++] = 'C';
	temp_buf[pos] = '\0';
	return temp_buf;
}

#endif // _DS18B20_H_
