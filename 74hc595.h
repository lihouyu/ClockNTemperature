/*
 * 74hc595.h
 *
 * 74HC595 Driver Supporting Multiple Cascade
 *
 * Based on sample code from TI MSP430 Launchpad Wiki
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#ifndef HC595_H_
#define HC595_H_

void HC595_tick_clock();
void HC595_latch_off();
void HC595_latch_on();
void HC595_write(unsigned int bit, unsigned char val);
void HC595_shiftout (unsigned char val);
void HC595_enable();
void HC595_disable();

#endif /* 74HC595_H_ */
