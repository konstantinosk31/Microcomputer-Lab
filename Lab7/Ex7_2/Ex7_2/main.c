/*
* main.c
*
* Created: 11/22/2024 11:05:01 AM
*  Author: User
*/ 

#include <xc.h>

#define F_CPU 16000000UL
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

typedef uint8_t bool;
#define true 1
#define false 0

/* LCD DISPLAY THROUGH PORT EXPANDER */
/* {{{ */
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
    
uint8_t LAST;

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
twi_write(reg);
twi_write(value);
twi_stop();
LAST = value;
//if (reg != REG_CONFIGURATION_0) exit(0);
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

void flash ()
{
_delay_us(50);
uint8_t tmp = PCA9555_0_read(REG_INPUT_0);
PCA9555_0_write(REG_OUTPUT_0, tmp | (1 << 3));
_delay_us(50);
PCA9555_0_write(REG_OUTPUT_0, tmp & ~(1 << 3));
}

void write_2_nibbles(uint8_t data){
uint8_t temp = LAST & 0x0f;
uint8_t out = data & 0xf0 | temp;
PCA9555_0_write(REG_OUTPUT_0, out);
flash();

out = (data << 4) & 0xf0 | temp;
PCA9555_0_write(REG_OUTPUT_0, out);
flash();
}

void lcd_data (uint8_t data)
{
uint8_t tmp = LAST;
PCA9555_0_write(REG_OUTPUT_0, tmp | (1 << 2));
write_2_nibbles(data);
_delay_us(500);
}

void lcd_command (uint8_t instr)
{
uint8_t tmp = LAST;
PCA9555_0_write(REG_OUTPUT_0, tmp & ~(1 << 2));
write_2_nibbles(instr);
_delay_us(500);
}

void lcd_clear_display(){
lcd_command(0x01);
_delay_ms(200);
}

void lcd_init ()
{
_delay_ms(200);

uint8_t out = 0x30;
for (int i=0; i<3; ++i) {
    PCA9555_0_write(REG_OUTPUT_0, out);
    flash();
    _delay_us(250);
}
PCA9555_0_write(REG_OUTPUT_0, 0x20);
flash();
_delay_us(250);

lcd_command(0x28);
lcd_command(0x0c);
lcd_clear_display();
lcd_command(0x06);
}

void lcd_number(uint16_t number){
	uint8_t digits[18];
	int i = 0;
	for (; number; number >>= 1)
	digits[i++] = number & 1;
	for (i-=1; i>=0; --i)
	lcd_data(digits[i] + '0');
}

void lcd_string (const char* str)
{
	lcd_command(0x02);
	for (; *str; str++)
		lcd_data(*str);
}

void lcd_temp (int16_t val, int precision)
{
	char tmp[17];
	int idx = 0;
	lcd_command(0x02);

	if (val < 0) {
		lcd_data('-');
		val = -val;
	}

	int   int_part  = val >> precision;
	float frac_part =  ((float)(val) / (1 << precision)) - int_part;

	if (int_part) {
		for (; int_part; int_part /= 10)
			tmp[idx++] = (int_part % 10) + '0';
		for (idx -= 1; idx >= 0; --idx)
			lcd_data(tmp[idx]);
	} else {
		lcd_data('0');
	}
	
	lcd_data('.');
	for (int i=0; i<precision; ++i) {
		frac_part *= 10;
		lcd_data((int)(frac_part) + '0');
		frac_part -= (int)(frac_part);
	}

	lcd_data(0b11011111); lcd_data('C');
}
/* }}} */

#define SET(x, b)   do { (x) |=   1 << (b);  } while (0)
#define CLEAR(x, b) do { (x) &= ~(1 << (b)); } while (0)

/* TEMPLATE CODE FROM GIVEN ASSEMBLY */
/* {{{ */
bool one_wire_reset (void)
{
    SET(DDRD, 4); // set output

    CLEAR(PORTD, 4); // sent 0
    _delay_us(480);

    CLEAR(DDRD, 4); // set input
    CLEAR(PORTD, 4); // disable pull-up
    _delay_us(100);

    int tmp = PIND; // read PORTD
    _delay_us(380);
    return (tmp & (1 << 4)) == 0;
}

uint8_t one_wire_receive_bit (void)
{
    SET(DDRD, 4); // Set output
    CLEAR(PORTD, 4);
    _delay_us(2);

    CLEAR(DDRD, 4); // set input
    CLEAR(PORTD, 4); // disable pull-up
    _delay_us(10);

    uint8_t ret = PIND & (1 << 4);
    _delay_us(49);
    return ret >> 4;
}

void one_wire_transmit_bit (uint8_t in)
{
    SET(DDRD, 4); // Set output
    CLEAR(PORTD, 4);
    _delay_us(2);

    in &= 1; // keep only LSB
    if (in) SET(PORTD, 4);
    else    CLEAR(PORTD, 4);

    _delay_us(58);
    CLEAR(DDRD, 4); // set input
    CLEAR(PORTD, 4); // disable pull-up
    _delay_us(1);
}

uint8_t one_wire_receive_byte (void)
{
    uint8_t ret = 0;
    for (int i=0; i<8; ++i)
    ret |= (one_wire_receive_bit() << i);
    return ret;
}

void one_wire_transmit_byte (uint8_t in)
{
    for (int i=0; i<8; ++i, in >>= 1)
    one_wire_transmit_bit(in);
}
/* }}} */

#define TEMP_ERR 0x8000

int16_t read_temp (void)
{
    int16_t ret = 0;
	
    if (one_wire_reset() != 1) return TEMP_ERR;
    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0x44);
	// lcd_string("Waiting...");
    while (one_wire_receive_bit() != 1);
	// lcd_string("Finished!");

    if (one_wire_reset() != 1) return TEMP_ERR;
    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0xBE);

    ret |= one_wire_receive_byte();
    ret |= one_wire_receive_byte() << 8;
    return ret;
}

const char err_msg[] = "NO Device";


int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
    lcd_init();
	lcd_clear_display();

	bool is_prev_err = false;
    while (1) {
        int16_t temp = read_temp();
		if (temp != TEMP_ERR) {
			is_prev_err = false;
			lcd_temp(temp, 4);
        } else if (!is_prev_err) {
			is_prev_err = true;
			lcd_string(err_msg);
		}
		_delay_ms(50);
    }
	
	return 0;
}

