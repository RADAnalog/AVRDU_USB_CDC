
#include "config.h"		// F_CPU
#include <avr/io.h>
#include <stdbool.h>
#include "led.h"

// Status LEDs
#define	LED_CONFIGURED			PIN7_bm
#define LED_SUSPENDED			PIN6_bm

void inline ledInit(void)
{
	PORTD.DIRSET = LED_CONFIGURED;
	PORTD.DIRSET = LED_SUSPENDED;
}

void inline ledConfigured(uint8_t state)
{
	(state == false) ?	(PORTD.OUTCLR = LED_CONFIGURED) : (PORTD.OUTSET = LED_CONFIGURED);
}

void inline ledSuspended(uint8_t state)
{
	(state == false) ? (PORTD.OUTCLR = LED_SUSPENDED) : (PORTD.OUTSET = LED_SUSPENDED);
}