//*****************************************************************************
//	USB LED Functions - AVR_DU Test
//
//	Author: Richard
//	Date:	2026-04-13
//
//*****************************************************************************
#ifndef LED_H
#define LED_H

#include "config.h"				// F_CPU
#include <avr/io.h>
#include <stdbool.h>

// Status LEDs
#define	LED_CONFIGURED			PIN7_bm
#define LED_SUSPENDED			PIN6_bm

typedef enum {
	OFF = false,
	ON = true
}led_t;

void inline ledInit(void)
{
	PORTD.DIRSET = LED_CONFIGURED;
	PORTD.DIRSET = LED_SUSPENDED;
}

void inline ledConfigured(led_t state)
{
	if (state == true)
		PORTD.OUTSET = LED_CONFIGURED;
	else
		PORTD.OUTCLR = LED_CONFIGURED;
}

void inline ledSuspended(led_t state)
{
	if (state == true) 
		PORTD.OUTSET = LED_SUSPENDED;
	else
		PORTD.OUTCLR = LED_SUSPENDED;
}

#endif