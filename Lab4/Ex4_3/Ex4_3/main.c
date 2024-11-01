/*
 * main.c
 *
 * Created: 10/31/2024 11:29:07 PM
 *  Author: User
 */ 

#include <xc.h>

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#define NOP() do { __asm__ __volatile__ ( "nop "); } while (0)

void write_2_nibbles(uint8_t data){
	uint8_t temp = PIND & 0x0f;
	PORTD = data&(0xf0) | temp;
	
	PORTD |= (1<<PD3);
	NOP();
	NOP();
	PORTD &= ~(1<<PD3);
	
	PORTD = (data<<4)&(0xf0) | temp;
	
	PORTD |= (1<<PD3);
	NOP();
	NOP();
	PORTD &= ~(1<<PD3);
}

void lcd_data(uint8_t data){
	PORTD |= (1<<PD2);
	write_2_nibbles(data);
	_delay_us(250);
}

void lcd_command(uint8_t instruction){
	PORTD &= ~(1<<PD2);
	write_2_nibbles(instruction);
	_delay_us(250);
}

void lcd_clear_display(){
	lcd_command(0x01);
	_delay_ms(5);
}

void lcd_init(){
	_delay_ms(200);
	
	PORTD = 0x30;
	PORTD |= (1<<PD3);
	NOP();
	NOP();
	PORTD &= ~(1<<PD3);
	_delay_us(250);
	
	PORTD = 0x30;
	PORTD |= (1<<PD3);
	NOP();
	NOP();
	PORTD &= ~(1<<PD3);
	_delay_us(250);
	
	PORTD = 0x30;
	PORTD |= (1<<PD3);
	NOP();
	NOP();
	PORTD &= ~(1<<PD3);
	_delay_us(250);
	
	lcd_command(0x28);
	
	lcd_command(0x0c);
	
	lcd_clear_display();
	
	lcd_command(0x06);
}

void lcd_digit(uint8_t digit){
	lcd_data(0x30 + digit);
}

void lcd_number(uint32_t number){
	uint8_t digits[10];
	int i = 0;
	if(number == 0){
		lcd_digit(0);
		return;
	}
	do{
		digits[i++] = number%10;
		number /= 10;
	} while(number > 0);
	for(; i > 0; ) lcd_digit(digits[--i]);
}

uint16_t calc_ppm(float V_gas){
	if(V_gas <= 0.1) return 0;
	uint16_t val = 10000.0/129.0*(V_gas - 0.1);
	if(val > 500) return 500;
	else return val;
}

bool interrupted = true;

ISR(TIMER1_COMPA_vect){
	interrupted = true;
	return;
}

void lcd_char(char c){
	lcd_data(c);
}

void lcd_string(char *s){
	while(*s != '\0'){
		lcd_char(*s);
		s++;
	} 
}

int main(void)
{
	DDRD = 0xff; //set PORTD as output
	DDRB = 0xff; //set PORTB as output
	lcd_init();
	_delay_ms(100);
	
	//   Vref = 5V, ADC2, Right adjust
	ADMUX = (1 << REFS0) | (0 << ADLAR) | (1 << MUX1);
	//   Enable, no interrupt, no conversion, 125 kHz
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) | (7 << ADPS0);
	
	// Init Timer1
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);		// CTC mode, pre-scaler 1024
	OCR1A = 1562;											// Compare value for 100ms: 1562/16000000*1024 = 0.099968 = 100ms
	TIMSK1 |= (1 << OCIE1A);								// Enable interrupt
	
	uint8_t prev_PORTB = 0;	
	sei();
	
	uint16_t ppm = 0;
	
	while(1)
	{
		// flicker
		_delay_ms(50);
		if (ppm > 70) PORTB = 0;
		_delay_ms(50);
		PORTB = prev_PORTB;
		
		if(!interrupted) continue;
		interrupted = false;
		
		// _delay_ms(1000);
		ADCSRA |= 1 << ADSC;
		while (ADCSRA & (1 << ADSC));
		float V_gas = ADC*5.0/(1<<10);
		ppm = calc_ppm(V_gas);
		lcd_clear_display();
		if(ppm <= 70){
			PORTB = 1;
			char str[6] = "CLEAR\0";
			lcd_string(str);
		} else {
			char str[13] = "GAS DETECTED\0";
			lcd_string(str);
			if(ppm <= 100) PORTB = 1;
			else if(ppm <= 200) PORTB = 2;
			else if(ppm <= 300) PORTB = 4;
			else if(ppm <= 400) PORTB = 8;
			else if(ppm <= 500) PORTB = 16;
			else PORTB = 32;
			
		}
		prev_PORTB = PORTB;
		// _delay_ms(1000);
	}
}