#ifndef __USART_H__
#define __USART_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int BAUD = 9600;
const int UBRR = 103;

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
void usart_init(unsigned int ubrr){
  UCSR0A=0;
  UCSR0B=(1<<RXEN0)|(1<<TXEN0);
  UBRR0H=(unsigned char)(ubrr>>8);
  UBRR0L=(unsigned char)ubrr;
  UCSR0C=(3 << UCSZ00);
  return;
}

/* Routine: usart_transmit
Description: This routine sends a byte of data using usart.
parameters: data: the byte to be transmitted
return value: None. */
void usart_transmit(uint8_t data){
  while(!(UCSR0A&(1<<UDRE0)));
  UDR0=data;
}

/* Routine: usart_receive
Description: This routine receives a byte of data from usart.
parameters: None.
return value: the received byte */
uint8_t usart_receive(){
  while(!(UCSR0A&(1<<RXC0)));
  return UDR0;
}

void usart_transmit_string(const char *str){
  while(*str != '\0'){
    usart_transmit(*str++);
  }
}

char* usart_receive_string(){
  char *str = (char*) malloc(1024*sizeof(char));
  int i = 0;
  while((str[i++] = usart_receive()) != '\n');
  str[i] = '\0';
  return str;
}

char* create_command(const char *name, const char* value){
  char *buff = (char*) malloc(64*sizeof(char));
  strcpy(buff, "{\"name\":\"");
  strcpy(buff + strlen(buff), name);
  strcpy(buff + strlen(buff), "\",\"value\":\"");
  strcpy(buff + strlen(buff), value);
  strcpy(buff + strlen(buff), "\"}");
  return buff;
}

char* create_payload(int argc, char **argv){
  char *buff = (char*) malloc(312*sizeof(char));
  strcpy(buff, "ESP:payload:[");
  for(int i = 0; i < argc; i++){
	  strcpy(buff + strlen(buff), argv[i]);
	  strcpy(buff + strlen(buff), (i == argc-1) ? "]\n" : ",");
  }
  return buff;
}

static enum { NO_RESTART, RESTART } usart_state = NO_RESTART;

static int are_same (const char *a, const char *b)
{
	while (*a != '\0' && *b != '\0')
		if (*a != *b) return 1;
		else a++, b++;
	return (*a != '\0') || (*a != '\0');
}

int usart_command(const char *cmd)
{
	char *buf = NULL;
	int ret;


	_delay_ms(500);
	usart_transmit_string(cmd);
	buf = usart_receive_string();
	ret = are_same(buf, "\"Success\"\n");
	
	free(buf);
	return ret;
}

void usart_restart()
{
	char *buf;
	usart_transmit_string("ESP:restart\n");
	buf = usart_receive_string(); // restart response
	free(buf);
	
	buf = usart_receive_string(); // restart response
	free(buf);
	
	usart_state = RESTART;
}

int usart_connect()
{
	//int tmp = -1;
		//goto connect;
//
	//tmp = 1;
	//if (usart_command("ESP:ssid:\"Micro_IoT\"\n"))
		//return 1;
	//if (usart_command("ESP:password:\"Microlab_IoT\"\n"))
		//return 2;
	//if (usart_command("ESP:debug:\"false\"\n"))
		//return 3;
	//if (usart_command("ESP:baudrate:\"9600\"\n"))
		//return 4;

	if (usart_command("ESP:connect\n"))
		return 5;
	return 0;
}

#endif // __USART_H__
