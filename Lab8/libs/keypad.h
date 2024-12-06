#include "pca9555.h"

uint8_t scan_row(uint8_t row);

uint16_t scan_keypad();

uint16_t scan_keypad_rising_edge();

char keypad_to_ascii();