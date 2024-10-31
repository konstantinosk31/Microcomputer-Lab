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

void lcd_number(uint8_t number){
	do{
		lcd_digit(number%10);
		number /= 10;
	} while(number >= 10);
}

uint8_t calc_ppm(uint8_t V_gas){
	uint8_t val = 10000.0/129.0*((double)V_gas - 0.1);
	if(val > 500) return 500;
	else return val;
}

bool interrupted = true;

ISR(TIMER1_COMPA_vect){
	interrupted = true;
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
	lcd_init();
	_delay_ms(100);
	
	//   Vref = 5V, ADC2, Left adjust
	ADMUX = (1 << REFS0) | (1 << ADLAR) | (1 << MUX1);
	//   Enable, no interrupt, no conversion, 125 kHz
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) | (7 << ADPS0);
	
	// Init Timer1
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, pre-scaler 1024
	OCR1A = 1562;                                         // Compare value for 100ms: 1562/16000000*1024 = 0.099968 = 100ms
	TIMSK1 |= (1 << OCIE1A);                              // Enable interrupt
	
	uint8_t val, prev_PORTB = 0;	
	sei();
	
	while(1)
	{
		if(!interrupted){
			_delay_ms(45);
			PORTB = 0;
			_delay_ms(45);
			PORTB = prev_PORTB;
			continue;
		}
		interrupted = false;
		lcd_clear_display();
		_delay_ms(1000);
		ADCSRA |= 1 << ADSC;
		while (ADCSRA & (1 << ADSC));
		uint8_t ppm = calc_ppm(ADC);
		lcd_number(ppm);
		if(ppm <= 70){
			PORTB = 0;
			lcd_string("CLEAR");
		}
		else{
			lcd_string("GAS DETECTED");
			if(ppm <= 140) PORTB = 1;
			else if(ppm <= 210) PORTB = 2;
			else if(ppm <= 280) PORTB = 4;
			else if(ppm <= 350) PORTB = 8;
			else if(ppm <= 420) PORTB = 16;
			else PORTB = 32;
			
		}
		prev_PORTB = PORTB;
	}
}