/*
 * main.c
 *
 * Created: 10/25/2024 11:40:30 PM
 *  Author: User
 */ 

#include <xc.h>

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

enum {MODE1, MODE2} mode;

#define DUTY_START 6
#define DUTY_LAST 12

int8_t i;
uint8_t *DC_VALUE;
uint8_t prev_PIND;
uint8_t DUTY[] = {5, 26, 46, 66, 87, 107, 128, 148, 168, 189, 209, 230, 250};
uint8_t POT_VALUE;

ISR(PCINT2_vect){
	uint8_t cur_PIND = ~PIND;
	uint8_t temp = (~prev_PIND) & cur_PIND; //get the pin that was just set
	prev_PIND = cur_PIND;
	
	if(temp & (1<<6)){ //PD6 pressed -> select mode1
		mode = MODE1;
	} else if(temp & (1<<7)){ //PD7 pressed -> select mode2
		mode = MODE2;
	}
	
	if (mode == MODE1) {
		if((temp & (1<<1)) && i < DUTY_LAST) //PD1 pressed
			i++;
		else if((temp & (1<<2)) && i > 0) //PD2 pressed
			i--;
		DC_VALUE = DUTY + i;
	} else if (mode == MODE2) {
		DC_VALUE = &POT_VALUE;
	}
}

int main(void)
{
	// Set PORTB as output
	DDRB = 0xff;
	
	// Set PORTD as input
	DDRD = 0x00;
	
	// Fast PWM, 8 bit, non-inverting output, N = 256. BOTTOM = 0, TOP = 0x00ff = 255
	TCCR1A = (1<<WGM10) | (1<<COM1A1);
	TCCR1B = (1<<WGM12) | (1<<CS12);
	
	// Enable PCINT17 (PD1), PCINT18 (PD2) interrupts
	PCMSK2 = (1<<PCINT17) | (1<<PCINT18) | (1 << PCINT22) | (1 << PCINT23);
	
	// Enable Pin Change Interrupt 2: PCINT[23:16]
	PCICR = (1<<PCIE2);
	
	prev_PIND = 0;
	
	i = DUTY_START;
	DC_VALUE = DUTY + i;
	mode = MODE1;
	
	//   Vref = 5V, ADC0, Left adjust
	ADMUX = (1 << REFS0) | (1 << ADLAR) | (0 << MUX0);
	//   Enable, no interrupt, no conversion, 125 kHz
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) | (7 << ADPS0);
	
	sei();
	while(1)
	{
		ADCSRA |= 1 << ADSC;
		while (ADCSRA & (1 << ADSC));
		POT_VALUE = 255 - ADCH;
		
		OCR1AL = *DC_VALUE;
		_delay_ms(10);
	}
}
