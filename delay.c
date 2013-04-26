/*
 * delay.c
 *
 * Simple delay function. Not so accurate!!!
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#include <msp430g2553.h>

#include "delay.h"

void delay_us(unsigned int us) {
	while (us--) {
		__delay_cycles(6); // Set for 12MHz
	}
}

void delay_ms(unsigned int ms) {
	while (ms--) {
		__delay_cycles(12000); // Set for 12MHz
	}
}
