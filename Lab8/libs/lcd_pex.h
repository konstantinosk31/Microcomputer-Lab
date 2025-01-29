#ifndef __LCD_H__
#define __LCD_H__

#include "pca9555.h"

void flash ()
{
	_delay_us(50);
	uint8_t tmp = PCA9555_0_read(REG_INPUT_0);
	PCA9555_0_write(REG_OUTPUT_0, tmp | (1 << 3));
	_delay_us(50);
	PCA9555_0_write(REG_OUTPUT_0, tmp & ~(1 << 3));
}

void write_2_nibbles(uint8_t data){
	uint8_t temp = LAST & 0x0f;
	uint8_t out = data & 0xf0 | temp;
	PCA9555_0_write(REG_OUTPUT_0, out);
	flash();

	out = (data << 4) & 0xf0 | temp;
	PCA9555_0_write(REG_OUTPUT_0, out);
	flash();
}

void lcd_data (uint8_t data)
{
	uint8_t tmp = LAST;
	PCA9555_0_write(REG_OUTPUT_0, tmp | (1 << 2));
	write_2_nibbles(data);
	_delay_us(500);
}

void lcd_command (uint8_t instr)
{
	uint8_t tmp = LAST;
	PCA9555_0_write(REG_OUTPUT_0, tmp & ~(1 << 2));
	write_2_nibbles(instr);
	_delay_us(500);
}

void lcd_clear_display(){
	lcd_command(0x01);
	_delay_ms(200);
}

void lcd_init ()
{
	_delay_ms(200);

	uint8_t out = 0x30;
	for (int i=0; i<3; ++i) {
		PCA9555_0_write(REG_OUTPUT_0, out);
		flash();
		_delay_us(250);
	}
	PCA9555_0_write(REG_OUTPUT_0, 0x20);
	flash();
	_delay_us(250);

	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_clear_display();
	lcd_command(0x06);
}

void lcd_string (const char* str)
{
	lcd_clear_display();
	for (; *str != '\0'; str++) {
		if (*str == '\n')
			lcd_command(0xc0);
		else
			lcd_data(*str);
		}
}

void lcd_temp (int16_t val, int decimals)
{
	char tmp[17];
	int idx = 0;
	lcd_command(0x02);

	if (val < 0) {
		lcd_data('-');
		val = -val;
	}

	int   int_part  = val >> PRECISION;
	float frac_part =  ((float)(val) / (1 << PRECISION)) - int_part;

	if (int_part) {
		for (; int_part; int_part /= 10)
			tmp[idx++] = (int_part % 10) + '0';
		for (idx -= 1; idx >= 0; --idx)
			lcd_data(tmp[idx]);
	} else {
		lcd_data('0');
	}

	lcd_data('.');
	for (int i=0; i<decimals; ++i) {
		frac_part *= 10;
		lcd_data((int)(frac_part) + '0');
		frac_part -= (int)(frac_part);
	}

	lcd_data(0b11011111); lcd_data('c');
}

#endif // _LCD_H__