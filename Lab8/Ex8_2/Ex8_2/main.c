/*
 * main.c
 *
 * Created: 12/6/2024 11:16:24 AM
 *  Author: User
 */ 

#include "../../libs/usart.h"
#include "../../libs/ds18b20.h"
#include "../../libs/lcd_pex.h"
#include "../../libs/keypad.h"
#include "../../libs/pot.h"

int main(void)
{
	int ret;
	char buf[30];

	usart_init(UBRR);
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output for lcd display
	lcd_init();
    one_wire_reset();
	pot_init();
	
	// usart_restart();
	ret = usart_connect();
	if (ret)
		snprintf(buf, sizeof(buf), "1.Fail (%d)", ret);
	else
		snprintf(buf, sizeof(buf), "1.Success");
	lcd_string(buf);
	
	_delay_ms(5000);

	if (usart_command("ESP:url:\"http://192.168.1.250:5000/data\""))
		snprintf(buf, sizeof(buf), "2.Fail (%d)", ret);
	else
		snprintf(buf, sizeof(buf), "2.Success");
	lcd_string(buf);

    _delay_ms(5000);

    int16_t raw_temp;
    int8_t pressure;
    char keypad;
    char status[16];
    char buf[60];

    while (1) {
        raw_temp  = read_temp();
        if (raw_temp == TEMP_ERR) raw_temp = 0;
        add_to_temp(&raw_temp, 10);
        pressure = read_pressure();
        keypad = keypad_to_ascii();

        if (keypad == '8')
            sprintf(status, "NURSE CALL");
        else if (keypad == '#' && !strcmp(status, "NURSE CALL"))
            sprintf(status, "OK");
        else if (pressure < 4 || 12 < pressure)
            sprintf(status, "CHECK PRESSURE");
        else if (TEMP(raw_temp) < 34 || 37 < TEMP(raw_temp))
            sprintf(status, "CHECK TEMP");
        else if (strcmp(status, "NURSE CALL"))
			sprintf(status, "OK");

        char* pressure_str = pressure_to_str(pressure);
        char* temp_str = temp_to_str(raw_temp, 1);
		
		strcpy(buf, "T: ");
		strcpy(buf + strlen(buf), temp_str);
		strcpy(buf + strlen(buf), " P: ");
		strcpy(buf + strlen(buf), pressure_str);
		strcpy(buf + strlen(buf), "\n");
		strcpy(buf + strlen(buf), status);
		
		lcd_string(buf);
		_delay_ms(1000);
    }
	
}