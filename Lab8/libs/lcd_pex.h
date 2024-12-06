#include "pca9555.h"

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
	for (; *str; str++) {
		if (*str == '\n')
			lcd_command(0xc0);
		else
			lcd_data(*str);
	}
}