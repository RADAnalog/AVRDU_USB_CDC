#ifndef USART_H
#define USART_H

#include "config.h"
#include <avr/io.h>

//#define USART_BAUD_RATE(BAUD_RATE) ((64ul * F_CPU) / (16ul * (uint32_t)BAUD_RATE))
#define USART_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

// Valid Baud rates
typedef enum BaudRate {
	BAUD_9600   = 9600UL,
	BAUD_115200 = 115200UL,
	BAUD_921600 = 921600UL
} BaudRate_t;

uint16_t usartInit(BaudRate_t baud);
int USART_printChar(char data, FILE *stream);
int USART_inputChar(FILE *stream);
int USART_isCharAvailable(void);
void USART_putChar(char data);

#endif
