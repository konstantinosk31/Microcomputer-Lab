/*
 * main.c
 *
 * Created: 11/8/2024 10:58:48 AM
 *  Author: User
 */ 

#include <xc.h>

#define F_CPU 16000000UL
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#define PCA9555_0_ADDRESS 0x40 //A0=A1=A2=0 by hardware
#define TWI_READ 1 // reading from twi device
#define TWI_WRITE 0 // writing to twi device
#define SCL_CLOCK 100000L // twi clock in Hz

//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2

#define NOP() do { __asm__ __volatile__ ( "nop "); } while (0)

// PCA9555 REGISTERS
typedef enum {
	REG_INPUT_0 = 0,
	REG_INPUT_1 = 1,
	REG_OUTPUT_0 = 2,
	REG_OUTPUT_1 = 3,
	REG_POLARITY_INV_0 = 4,
	REG_POLARITY_INV_1 = 5,
	REG_CONFIGURATION_0 = 6,
	REG_CONFIGURATION_1 = 7
} PCA9555_REGISTERS;

//----------- Master Transmitter/Receiver -------------------
#define TW_START 0x08
#define TW_REP_START 0x10

//---------------- Master Transmitter ----------------------
#define TW_MT_SLA_ACK 0x18
#define TW_MT_SLA_NACK 0x20
#define TW_MT_DATA_ACK 0x28

//---------------- Master Receiver ----------------
#define TW_MR_SLA_ACK 0x40
#define TW_MR_SLA_NACK 0x48
#define TW_MR_DATA_NACK 0x58

#define TW_STATUS_MASK 0b11111000
#define TW_STATUS (TWSR0 & TW_STATUS_MASK)

//initialize TWI clock
void twi_init(void)
{
	TWSR0 = 0; // PRESCALER_VALUE=1
	TWBR0 = TWBR0_VALUE; // SCL_CLOCK 100KHz
}

// Read one byte from the twi device (request more data from device)
unsigned char twi_readAck(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR0 & (1<<TWINT)));
	return TWDR0;
}

//Read one byte from the twi device, read is followed by a stop condition
unsigned char twi_readNak(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR0 & (1<<TWINT)));
	return TWDR0;
}

// Issues a start condition and sends address and transfer direction.
// return 0 = device accessible, 1= failed to access device
unsigned char twi_start(unsigned char address)
{
	uint8_t twi_status;
	
	// send START condition
	TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	
	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT)));
	
	// check value of TWI Status Register.
	twi_status = TW_STATUS & 0xF8;
	if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) return 1;
	
	// send device address
	TWDR0 = address;
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	
	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR0 & (1<<TWINT)));
	// check value of TWI Status Register.
	twi_status = TW_STATUS & 0xF8;
	if ( (twi_status != TW_MT_SLA_ACK) && (twi_status != TW_MR_SLA_ACK) )
	{
		return 1;
	}
	return 0;
}

// Send start condition, address, transfer direction.
// Use ack polling to wait until device is ready
void twi_start_wait(unsigned char address)
{
	uint8_t twi_status;
	while ( 1 )
	{
		// send START condition
		TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
		
		// wait until transmission completed
		while(!(TWCR0 & (1<<TWINT)));
		
		// check value of TWI Status Register.
		twi_status = TW_STATUS & 0xF8;
		if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) continue;
		
		// send device address
		TWDR0 = address;
		TWCR0 = (1<<TWINT) | (1<<TWEN);
		
		// wail until transmission completed
		while(!(TWCR0 & (1<<TWINT)));
		
		// check value of TWI Status Register.
		twi_status = TW_STATUS & 0xF8;
		if ( (twi_status == TW_MT_SLA_NACK )||(twi_status ==TW_MR_DATA_NACK) )
		{
			/* device busy, send stop condition to terminate write operation */
			TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
			
			// wait until stop condition is executed and bus released
			while(TWCR0 & (1<<TWSTO));
			continue;
		}
		break;
	}
}

// Send one byte to twi device, Return 0 if write successful or 1 if write failed
unsigned char twi_write( unsigned char data )
{
	// send data to the previously addressed device
	TWDR0 = data;
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	
	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT)));
	if( (TW_STATUS & 0xF8) != TW_MT_DATA_ACK) return 1;
	return 0;
}

// Send repeated start condition, address, transfer direction
//Return: 0 device accessible
// 1 failed to access device
unsigned char twi_rep_start(unsigned char address)
{
	return twi_start( address );
}

// Terminates the data transfer and releases the twi bus
void twi_stop(void)
{
	// send stop condition
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR0 & (1<<TWSTO));
}

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
	twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
	twi_write(reg);
	twi_write(value);
	twi_stop();
}

uint8_t PCA9555_0_read(PCA9555_REGISTERS reg)
{
	uint8_t ret_val;
	twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
	twi_write(reg);
	twi_rep_start(PCA9555_0_ADDRESS + TWI_READ);
	ret_val = twi_readNak();
	twi_stop();
	return ret_val;
}

uint8_t LCD = 0;

void write_2_nibbles(uint8_t data){
	uint8_t temp = PCA9555_0_read(REG_INPUT_1);
	uint8_t out = data & 0xf0 | temp;
	PCA9555_0_write(REG_OUTPUT_1, out);

	PCA9555_0_write(REG_OUTPUT_1, out | (1 << 3));
	NOP();
	NOP();
	PCA9555_0_write(REG_OUTPUT_1, out & ~(1 << 3));

	out = (data << 4) & 0xf0 | temp;
	PCA9555_0_write(REG_OUTPUT_1, out);

	PCA9555_0_write(REG_OUTPUT_1, out | (1 << 3));
	NOP();
	NOP();
	PCA9555_0_write(REG_OUTPUT_1, out & ~(1 << 3));
}

void lcd_data (uint8_t data)
{
	uint8_t tmp = PCA9555_0_read(REG_INPUT_1);
	PCA9555_0_write(REG_OUTPUT_1, tmp | (1 << 2));
	write_2_nibbles(data);
	_delay_us(250);
}

void lcd_command (uint8_t instr)
{
	uint8_t tmp = PCA9555_0_read(REG_INPUT_1);
	PCA9555_0_write(REG_OUTPUT_1, tmp & ~(1 << 2));
	write_2_nibbles(instr);
	_delay_us(250);
}

void lcd_clear_display(){
	lcd_command(0x01);
	_delay_ms(5);
}

void lcd_init ()
{
	_delay_ms(200);

	for (int i=0; i<3; ++i) {
		PCA9555_0_write(REG_OUTPUT_1, 0x30);
		PCA9555_0_write(REG_OUTPUT_1, out | (1 << 3));
		NOP();
		NOP();
		PCA9555_0_write(REG_OUTPUT_1, out & ~(1 << 3));
		_delay_us(250);
	}

	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_clear_display();
	lcd_command(0x06);
}

void lcd_string (const char* str)
{
	for (; *str; str++) lcd_data(*str);
}

const char name[] = "Jim Balatos";

int main(void) {
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_1, 0x00); //Set EXT_PORT1 as output

	lcd_init();
	lcd_clear_display();
	lcd_string(name);
}
