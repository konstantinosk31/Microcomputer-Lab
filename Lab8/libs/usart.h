#include "utils.h"

/* Routine: usart_init
Description: This routine initializes the usart as shown below.
------- INITIALIZATIONS -------
Baud rate: 9600 (Fck= 8MH)
Asynchronous mode
Transmitter on
Receiver on
Communication parameters: 8 Data ,1 Stop, no Parity
--------------------------------
parameters: ubrr to control the BAUD.
return value: None.*/
void usart_init(unsigned int ubrr);

/* Routine: usart_transmit
Description: This routine sends a byte of data using usart.
parameters: data: the byte to be transmitted
return value: None. */
void usart_transmit(uint8_t data);

/* Routine: usart_receive
Description: This routine receives a byte of data from usart.
parameters: None.
return value: the received byte */
uint8_t usart_receive();

const int BAUD = 9600;
const int UBRR = 103;