#include "pca9555.h"

void write_2_nibbles(uint8_t data);

void lcd_data (uint8_t data);

void lcd_command (uint8_t instr);

void lcd_clear_display();

void lcd_init ();

void lcd_string (const char* str);