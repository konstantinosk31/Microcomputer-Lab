/*
 * main.c
 *
 * Created: 12/6/2024 11:16:24 AM
 *  Author: User
 */ 

#include "../../libs/usart.h"
#include "../../libs/lcd_pex.h"

int main(void)
{
	usart_init(UBRR);
	uint8_t num;
    while(1)
    {
        _delay_ms(1000);
		num = usart_receive();
		_delay_ms(1000);
		usart_transmit(num);
    }
}