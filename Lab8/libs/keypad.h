#ifndef __PCA_H__
#define __PCA_H__

#include "pca9555.h"

#endif

#ifndef __KEYPAD_H__
#define __KEYPAD_H__

uint8_t scan_row(uint8_t row){ //row = 0, 1, 2, 3
	uint8_t mask = 0x0f & ~(1<<row);
	PCA9555_0_write(REG_OUTPUT_1, mask); //enable row as input
	_delay_us(100);
	uint8_t in = ~PCA9555_0_read(REG_INPUT_1); //read columns of row pressed in positive logic
	in >>= 4; //remove IO1[0:3]
	return in; //4 bits
}

uint16_t scan_keypad(){
	uint16_t row0 = scan_row(0);
	uint16_t row1 = scan_row(1);
	uint16_t row2 = scan_row(2);
	uint16_t row3 = scan_row(3);
	return row0 | (row1<<4) | (row2<<8) | (row3<<12);
}

uint16_t scan_keypad_rising_edge(){
	//static uint16_t pressed_keys = 0;
	uint16_t pressed_keys_tempo = scan_keypad();
	_delay_ms(15); //wait to avoid triggering
	pressed_keys_tempo &= scan_keypad(); //only keep the actual buttons pressed
	return pressed_keys_tempo;
	//uint16_t keys_just_pressed = pressed_keys_tempo & (~pressed_keys);
	//pressed_keys = pressed_keys_tempo;
	//return keys_just_pressed;
}

char keypad_to_ascii(){
	uint16_t key = scan_keypad();
	if(key&(1<<0)) return '*';
	if(key&(1<<1)) return '0';
	if(key&(1<<2)) return '#';
	if(key&(1<<3)) return 'D';
	if(key&(1<<4)) return '7';
	if(key&(1<<5)) return '8';
	if(key&(1<<6)) return '9';
	if(key&(1<<7)) return 'C';
	if(key&(1<<8)) return '4';
	if(key&(1<<9)) return '5';
	if(key&(1<<10)) return '6';
	if(key&(1<<11)) return 'B';
	if(key&(1<<12)) return '1';
	if(key&(1<<13)) return '2';
	if(key&(1<<14)) return '3';
	if(key&(1<<15)) return 'A';
	return 0;
}

#endif //__KEYPAD_H__
