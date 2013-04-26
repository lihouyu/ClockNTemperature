/*
 * 74hc595.c
 *
 * 74HC595 Driver Supporting Multiple Cascade
 *
 * Based on sample code from TI MSP430 Launchpad Wiki
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#include <msp430g2553.h>

#include "74hc595_conf.h"
#include "74hc595.h"

void HC595_tick_clock() {
	HC595_POUT |= HC595_CLOCK;
	HC595_POUT ^= HC595_CLOCK;
	//HC595_POUT &= ~HC595_CLOCK;
}

void HC595_latch_off() {
	HC595_POUT &= ~HC595_LATCH;
}

void HC595_latch_on() {
	HC595_POUT |= HC595_LATCH;
	HC595_POUT &= ~HC595_LATCH;
}

void HC595_write(unsigned int bit, unsigned char val) {
	if (val) {
		HC595_POUT |= bit;
	} else {
		HC595_POUT &= ~bit;
	}
}

void HC595_shiftout (unsigned char val) {
	unsigned char i;
	for (i = 0; i < 8; i++) {
		HC595_write(HC595_DATA, (val & (1 << i)));
		HC595_tick_clock();
	}
}

void HC595_enable() {
	HC595_POUT &= ~HC595_ENABLE;
}

void HC595_disable() {
	HC595_POUT |= HC595_ENABLE;
}
