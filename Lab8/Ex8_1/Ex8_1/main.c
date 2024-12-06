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
	int ret;
	char buf[30];

	usart_init(UBRR);
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output for lcd display
	lcd_init();
	
	// usart_restart();
	ret = usart_connect();
	if (ret)
		snprintf(buf, sizeof(buf), "1.Fail (%d)", ret);
	else
		snprintf(buf, sizeof(buf), "1.Success");
	lcd_string(buf);
	
	_delay_s(5);

	if (usart_command("ESP:url:\"http://192.168.1.250:5000/data\""))
		snprintf(buf, sizeof(buf), "2.Fail (%d)", ret);
	else
		snprintf(buf, sizeof(buf), "2.Success");
	lcd_string(buf);

	while (1);
	
}