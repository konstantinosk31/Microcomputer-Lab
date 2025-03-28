#ifndef __POT_H__
#define __POT_H__

void pot_init(){
	// Fast PWM, 8 bit, non-inverting output, N = 256. BOTTOM = 0, TOP = 0x00ff = 255
	TCCR1A = (1<<WGM10) | (1<<COM1A1);
	TCCR1B = (1<<WGM12) | (1<<CS12);

	// Init ADC:
	//   Vref = 5V, ADC0
	ADMUX = (1 << REFS0) | (1 << ADLAR) | (0 << MUX0);
	//   Enable, no interrupt, no conversion, 125 kHz
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) | (7 << ADPS0);
}

int16_t read_pot(){
	// Handle ADC
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return 255 - ADCH;
}

int8_t read_pressure(){
	int16_t pressure = read_pot();
	return (pressure * 20) >> 8;
}

char* pressure_to_str(int8_t pressure){
	char *buff = (char*) malloc(64*sizeof(char));
	char num[5];
	int pos = 0, idx = 0;
	
	if (pressure == 0) {
		strcpy(buff, "0");
	} else {
		for (; pressure; pressure /= 10, pos++)
			num[pos] = (pressure % 10) + '0';
		for (pos -= 1; pos >= 0; pos--)
			buff[idx++] = num[pos];
		buff[idx] = '\0';
	}
	return buff;
}

#endif //__POT_H__
