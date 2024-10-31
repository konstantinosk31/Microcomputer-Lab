/*
 * main.c
 *
 * Created: 10/31/2024 1:20:10 AM
 *  Author: User
 */ 

#include <xc.h>

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define NOP() do { __asm__ __volatile__ ( "nop "); } while (0)
#define PD0 0
#define PD1 1
#define PD2 2
#define PD3 3
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7
#define V_REF 5

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
	lcd_data(0x30 + digit)
}

void lcd_number(uint8_t number){
	do{
		lcd_digit(number%10);
		number /= 10;
	} while(number >= 10);
}

int main(void)
{
	DDRD = 0xff; //set PORTD as output
	lcd_init();
	_delay_ms(100);
	
	//   Vref = 5V, ADC0, Left adjust
	ADMUX = (1 << REFS0) | (1 << ADLAR) | (0 << MUX0);
	//   Enable, no interrupt, no conversion, 125 kHz
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) | (7 << ADPS0);
	
	uint8_t val;
	
    while(1)
    {
        lcd_clear_display();
		_delay_ms(1000);
		ADCSRA |= 1 << ADSC;
		while (ADCSRA & (1 << ADSC));
		val = (ADC*V_REF)>>10;
		lcd_number(val);
		_delay_ms(1000);
    }
}