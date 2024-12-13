#ifndef __USART_H__
#define __USART_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

static const int BAUD = 9600;
static const int UBRR = 103;
static const int MAX_STR_SIZE = 256;

/*** PRIVATE FUNCTIONS ***/

static uint8_t
usart_receive_byte (void)
{
	while(!(UCSR0A&(1<<RXC0)));
	return UDR0;
}

static char*
usart_receive_string (void)
{
	char *str = malloc(MAX_STR_SIZE * sizeof(char));
	int i = 0;
	while((str[i++] = usart_receive_byte()) != '\n');
	str[i] = '\0';
	return str;
}

static void
usart_transmit_byte (uint8_t data)
{
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0=data;
}

static void
usart_transmit_string (const char *str)
{
	while(*str != '\0')
		usart_transmit_byte(*str++);
}

static int
usart_command (const char *cmd, const char *expected)
{
	char *buff = NULL;
	int ret;

	if (cmd != NULL) usart_transmit_string(cmd);
	buff = usart_receive_string();
	ret = str_eq(buff, expected);

	free(buff);
	return ret;
}

#define usart_command_1(cmd)      usart_command(cmd, "\"Success\"\n")
#define usart_command_2(cmd, ans) usart_command(cmd, ans)
#define usart_command(...) VARG_SET(usart_command_, __VA_ARGS__)

#define usart_create_payload(...) __usart_create_payload(0, __VA_ARGS__)

static char*
__usart_create_payload (int unused, ...)
{
	char *buff = stralloc(MAX_STR_SIZE);
	va_list args; va_start(args, unused);
	const char *json;

	APPEND(buff, "ESP:payload:[");
	while ((json = va_arg(args, const char *)) != NULL) {
		APPEND(buff, json);
		APPEND(buff, ",");
	}

	strcpy(buff + strlen(buff) - 1, "]\n");
	return buff;
}


/*** PUBLIC FUNCTIONS ***/

void
usart_init (void)
{
	UCSR0A=0;
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);
	UBRR0H=(unsigned char)(UBRR>>8);
	UBRR0L=(unsigned char)UBRR;
	UCSR0C=(3 << UCSZ00);
}

char*
usart_create_object (const char *name, const char *value)
{
	char *buff = stralloc(MAX_STR_SIZE);

	APPEND(buff,   "{\"name\":\"");  APPEND(buff, name);
	APPEND(buff, "\",\"value\":\""); APPEND(buff, value);
	APPEND(buff, "\"}");

	return buff;
}

#define usart_payload(...) ({ \
	char *buf = usart_create_payload(__VA_ARGS__); \
	usart_transmit(buf); })

int
usart_restart (void)
{
	usart_command("ESP:restart\n");
	usart_command(NULL);
	return 0;
}

int
usart_connect (void)
{ return usart_command("ESP:connect\n"); }

int
usart_transmit (void)
{ return usart_command("ESP:transmit\n", "200 OK\n"); }

#endif //__USART_H__
