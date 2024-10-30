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

#define DUTY_START 6
#define DUTY_LAST 12
#define DUTY_INC (1 << 6) // PD6
#define DUTY_DEC (1 << 5) // PD5

int8_t i;
uint8_t DC_VALUE;
uint8_t prev_PIND;
uint8_t DUTY[] = {5, 26, 46, 66, 87, 107, 128, 148, 168, 189, 209, 230, 250};
uint8_t out_led;
int16_t measures[16], sum;
uint8_t pos = 0;

ISR(PCINT2_vect){
	// Handle button presses
	uint8_t cur_PIND = ~PIND;
	uint8_t temp = (~prev_PIND) & cur_PIND; //get the pin that was just set
	prev_PIND = cur_PIND;
	
	// Handle PD5, PD6
	if((temp & DUTY_INC) && i < DUTY_LAST)
		i++;
	else if((temp & DUTY_DEC) && i > 0)
		i--;
	DC_VALUE = DUTY[i];
}

int main(void)
{
	// Set PORTB as output
	DDRB = 0xff;
	
	// Set PORTC as input
	DDRC = 0x00;
	// Set PORTD[0-4] as output, PORTD[5-6] as input
	DDRD = 0x1f;
	
	// Fast PWM, 8 bit, non-inverting output, N = 256. BOTTOM = 0, TOP = 0x00ff = 255
	TCCR1A = (1<<WGM10) | (1<<COM1A1);
	TCCR1B = (1<<WGM12) | (1<<CS12);
	
	// Enable PCINT21 (PD5), PCINT23 (PD6) interrupts
	PCMSK2 = (1<<PCINT21) | (1<<PCINT22);
	
	// Enable Pin Change Interrupt 2: PCINT[23:16]
	PCICR = (1<<PCIE2);
	
	prev_PIND = 0;
	
	i = DUTY_START;
	DC_VALUE = DUTY[i];
	
	// Init ADC:
	//   Vref = 5V, ADC1
	ADMUX = (1 << REFS0) | (1 << MUX0);
	//   Enable, no interrupt, no conversion, 125 kHz
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) | (7 << ADPS0);
	
	sei();
	
	while(1)
	{
		OCR1AL = DC_VALUE;
		
		// Handle ADC
		ADCSRA |= (1 << ADSC);
		while (ADCSRA & (1 << ADSC));
		int16_t val = ADC & ((1 << 10) - 1);
		// Update sum;
		sum += val - measures[pos];
		measures[pos] = val;
		pos = (pos + 1) & ((1 << 4) - 1);
		uint16_t tmp = sum >> 4;
		// uint16_t tmp = val;
		// Use average
		if (tmp <= 200)      out_led = 0;
		else if (tmp <= 400) out_led = 1;
		else if (tmp <= 600) out_led = 2;
		else if (tmp <= 800) out_led = 3;
		else                 out_led = 4;
		PORTD = (1 << out_led);
		
		_delay_ms(100);
	}
}
