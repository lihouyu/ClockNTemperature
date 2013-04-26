/*
 * 4d7s_display.h
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

#ifndef fD7S_DISPLAY_H_
#define fD7S_DISPLAY_H_

extern const unsigned char fD7S_digit_map[10];
extern const unsigned char fD7S_minus_map;
extern const unsigned char fD7S_com_addr[4];

void fD7S_init();
void fD7S_show_digit(unsigned char digit, unsigned char pos, unsigned char show_dec_point);
void fD7S_show_char(unsigned char c, unsigned char pos, unsigned char show_dec_point);
void fD7S_show_minus(unsigned char pos);
void fD7S_wipe();

#endif /* 4D7S_DISPLAY_H_ */
