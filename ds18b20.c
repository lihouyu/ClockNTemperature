/*
 * ds18b20.c
 *
 * Simple communication with DS18B20. No ROM search function.
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#include <msp430g2553.h>

#include "ds18b20.h"

#include "delay.h"

unsigned char DS18B20_init() {
	// Master reset
	DS18B20_DIR |= DS18B20_PIN;
	DS18B20_OUT &= ~DS18B20_PIN;
	// Wait at least 480us
	delay_us(480); // For 12MHz+, using this line
	// Master release & set the pin to input
	DS18B20_DIR &= ~DS18B20_PIN;
	// Wait pull-up by resistor for 60us
	delay_us(60);
	// If no slave presence reply we return 1
	//if (DS18B20_IN & DS18B20_PIN) return 1;
	while (DS18B20_IN & DS18B20_PIN);
	// Wait slave release for 420us
	delay_us(420); // For 12MHz+, using this line
	// If slave does not release the line, that it keeps low, we will return 2
	//if (!(DS18B20_IN & DS18B20_PIN)) return 2;

	// Everything works fine, we return 0
	return 0;
}

void DS18B20_write_bit(unsigned char bit) {
	// Set pin as output and pull low
	DS18B20_DIR |= DS18B20_PIN;
	DS18B20_OUT &= ~DS18B20_PIN;

	// Write 1 or 0
	if (bit) {
		delay_us(5); // Max can be 15us. // For 12MHz+, using this line
	} else {
		delay_us(60); // Min 60us; // For 12MHz+, using this line
	}

	// Release the pin, set to input
	DS18B20_DIR &= ~DS18B20_PIN;

	// Fill rest of the time
	if (bit) {
		delay_us(60); // For 12MHz+, using this line
	} else {
		delay_us(5); // For 12MHz+, using this line
	}

	// The recover time
	delay_us(1); // For 12MHz+, using this line
}

unsigned char DS18B20_read_bit() {
	unsigned char in_bit;

	// Set pin as output and pull low
	DS18B20_DIR |= DS18B20_PIN;
	DS18B20_OUT &= ~DS18B20_PIN;

	// Keep for at least 1us
	delay_us(1);

	// Release the pin, set to input
	DS18B20_DIR &= ~DS18B20_PIN;

	// Wait for the almost end of 15us
	delay_us(5); // For 12MHz+, using this line

	// Read input level
	in_bit = DS18B20_IN & DS18B20_PIN;

	// Fill the time slot
	delay_us(60); // For 12MHz+, using this line

	// The recover time
	delay_us(1); // For 12MHz+, using this line

	return in_bit;
}

void DS18B20_write_byte(unsigned char byte) {
	unsigned char i;
	for (i = 0; i < 8; i++) {
		DS18B20_write_bit(byte & 1);
		byte >>= 1;
	}
}

unsigned char DS18B20_read_byte() {
	unsigned char i;
	unsigned char in_byte = 0x00;

	for (i = 0; i < 8; i++) {
		in_byte >>= 1;
		if (DS18B20_read_bit()) in_byte |= 0x80;
	}

	return in_byte;
}
