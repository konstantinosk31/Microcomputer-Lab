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

enum mode = {mode1, mode2};

#define DUTY_START 6
#define DUTY_LAST 12
#define DUTY_INC 0x30 // PD6
#define DUTY_DEC 0x10 // PD5

int8_t i;
uint8_t DC_VALUE;
uint8_t prev_PIND;
uint8_t DUTY[] = {5, 26, 46, 66, 87, 107, 128, 148, 168, 189, 209, 230, 250};
uint8_t out_led;
uint16_t measures[16], sum;
uint8_t measure_pos = 0;

ISR(PCINT2_vect){
	// Handle button presses
	uint8_t cur_PIND = PIND;
	uint8_t temp = (~prev_PIND) & cur_PIND; //get the pin that was just set
	if((temp & DUTY_INC) && i < DUTY_LAST)
		i++;
	else if((temp & DUTY_DEC) && i > 0)
		i--;
	// Handle ADC
	while (ADCSRA & (1 << ADSC));
	int16_t val = (ADCH << 7) | (ADCL);
	// Update sum;
	sum += val - measures[pos];
	measures[pos] = val;
	pos = (pos + 1) % 16;
	// Use average
	out_led = ((sum >> 4) - 1) / 200;
	if (out_led > 4) out_led = 4;
	PORTD = (1 << out_led);
}

int main(void)
{
	// Set PORTB as output
	DDRB = 0xff;
	
	// Set PORTC, PORTD as input
	DDRD = 0x00;
	DDRC = 0x00;
	
	// Fast PWM, 8 bit, non-inverting output, N = 256. BOTTOM = 0, TOP = 0x00ff = 255
	TCCR1A = (1<<WGM10) | (1<<COM1A1);
	TCCR1B = (1<<WGM12) | (1<<CS12);
	
	// Enable PCINT19 (PD3), PCINT20 (PD4) interrupts
	PCMSK2 = (1<<PCINT19) | (1<<PCINT20);
	
	// Enable Pin Change Interrupt 2: PCINT[23:16]
	PCICR = (1<<PCIE2);
	
	prev_PIND = 0;
	
	i = DUTY_START;
	
	// Init ADC:
	//   Vref = 5V, ADC0
	ADMUX = (1 << VREF0) | (0 << MUX0);
	//   Enable, no interrupt, no conversion, 125 kHz
	ADCSRA = (1 << ADEN) | (0 << ADCS) | (0 << ADIE) | (7 << ADPS0);
	
	sei();
	
	while(1)
	{
		OCR1AL = DC_VALUE;
		_delay_ms(10);
	}
}
