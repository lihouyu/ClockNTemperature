/*
 * 4d7s_display.c
 *
 * 4 Digit 7 Segment Display Boost Pack Driver
 *
 * The circuit is based on MSP430 Launchpad Clock by Luc Small
 * Details here http://lucsmall.com/2011/05/29/msp430-launchpad-clock/
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#include <msp430g2553.h>

#include "4d7s_display.h"

#include "74hc595_conf.h"
#include "74hc595.h"

const unsigned char fD7S_digit_map[10] = {
		0xFC, 0x60, 0xDA, 0xF2, 0x66,
		0xB6, 0xBE, 0xE0, 0xFE, 0xF6
};

const unsigned char fD7S_minus_map = 0x02;

const unsigned char fD7S_com_addr[4] = {
		0x7F, 0xBF, 0xDF, 0xEF
};

void fD7S_init() {
	HC595_PDIR |= (HC595_DATA + HC595_CLOCK + HC595_LATCH + HC595_ENABLE); // Set HC595 related ping as output
	HC595_enable(); // Enable HC595
}

void fD7S_show_digit(unsigned char digit, unsigned char pos, unsigned char show_dec_point) { // pos: start from 0, from left to right
	fD7S_show_char(fD7S_digit_map[digit], pos, show_dec_point);
}

void fD7S_show_char(unsigned char c, unsigned char pos, unsigned char show_dec_point) {
	HC595_latch_off();
	HC595_shiftout(fD7S_com_addr[pos]);
	if (show_dec_point)
		HC595_shiftout(c + 1);
	else
		HC595_shiftout(c);
	HC595_latch_on();
}

void fD7S_show_minus(unsigned char pos) {
	HC595_latch_off();
	HC595_shiftout(fD7S_com_addr[pos]);
	HC595_shiftout(fD7S_minus_map);
	HC595_latch_on();
}

void fD7S_wipe() {
	HC595_latch_off();
	HC595_shiftout(0xFF);
	HC595_shiftout(0x00);
	HC595_latch_on();
}
