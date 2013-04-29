/*
 * tp_n_haclk.c
 *
 * External Hi-Accuracy Temperature and RTC Source Boost Pack Driver
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#include <msp430g2553.h>

#include "tp_n_haclk.h"

#include "4d7s_display.h"

#include "ds18b20.h"
#include "ds3231.h"

#include "delay.h"

void TnC_init() {
	// First initialize the 2 function button as input, enable interrupt and enable the pull up resistor
	// Set P2.6, P2.7 as I/O
	TnP_FUNC_BTN_SEL &= ~(TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2);
	TnP_FUNC_BTN_SEL2 &= ~(TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2);
	// Set P2.0, P2.6, P2.7 as input
	TnP_FUNC_BTN_DIR &= ~(TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2 + TnP_RTCINT_PIN);
	// Enable pull-up resistor
	TnP_FUNC_BTN_REN |= (TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2 + TnP_RTCINT_PIN);
	TnP_FUNC_BTN_OUT |= (TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2 + TnP_RTCINT_PIN);
	// Enable interrupt on these pins
	TnP_FUNC_BTN_IE |= (TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2 + TnP_RTCINT_PIN);
	// Select high-low transition interrupt
	TnP_FUNC_BTN_IES |= (TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2 + TnP_RTCINT_PIN);

	// Set Buzz pin as output
	TnP_FUNC_BTN_DIR |= TnP_BUZZ_PIN;
	// Set Buzz pin default to high
	TnP_FUNC_BTN_OUT |= TnP_BUZZ_PIN;

	// Clear interrupt before start
	TnC_clear_btn_int();
}

void TnC_clear_btn_int() {
	// Clear interrupt before start
	TnP_FUNC_BTN_IFG &= ~(TnP_FUNC_BTN_1 + TnP_FUNC_BTN_2 + TnP_RTCINT_PIN);
}

void TnC_read_temperature(unsigned char * tp_data) {
	DS18B20_init();
	DS18B20_write_byte(0xCC);
	DS18B20_write_byte(0x44);

	/** Looks like we don't need following wait code **/
	// This is the official way, but big delay
	while(!DS18B20_read_bit());
	// or ???
	//while(!DS18B20_IN & DS18B20_PIN);

	// This is the easy way, but only delay enough time for temperature convertion
	//delay_ms(750);
	// No delay here, we will read temperature every 1 second using Timer Interrupt.

	DS18B20_init();
	DS18B20_write_byte(0xCC);
	DS18B20_write_byte(0xBE);

	// Now the temperature
	*tp_data = DS18B20_read_byte(); // The low part of temperature register
	*(tp_data + 1) = DS18B20_read_byte(); // The high part of temperature register
}

void TnC_convert_temperature(unsigned char * tp_data, unsigned char * digits) {
	unsigned char T_H, T_L;
	unsigned int part_int = 0, part_float = 0;
	unsigned char i, digit;

	T_L = *tp_data;
	T_H = *(tp_data + 1);

	// Doing conversion now
    for (i = 4; i >= 1; i--) {
        if (T_L & 1)
        	part_float += 10000 / (0x02 << (i - 1));
        T_L >>= 1;
    }
    // The temprature above 0 - part L
    if (T_L & 1) part_int++; // 2^0
    T_L >>= 1;
    for (i = 1; i <= 3; i++) {
        if (T_L & 1)
        	part_int += (0x02 << (i - 1));
        T_L >>= 1;
    }
    // Convert the TPH
    // The temprature above 0 - part H
    for (i = 4; i <= 6; i++) {
        if (T_H & 1)
        	part_int += (0x02 << (i - 1));
        T_H >>= 1;
    }

    // Make a round up
    if (part_float % 1000 >= 500) part_float += 1000;
    if (part_float >= 10000) part_int++;
    part_float = (part_float / 1000) % 10;

    // If there is a sign, the 1st digit is minus
    // Otherwise just see whether there's any digit or just 0.
    if (T_H & 1) {
    	// Below zero, the 1st digit will be 0x80
    	*digits = 0x80;
    } else {
    	// Above zero
    	digit = part_int / 100;
    	if (digit > 0)
    		*digits = digit;
    	else
    		*digits = 0;
    }

    // Now the second digit
    part_int = part_int % 100;
    digit = part_int / 10;
    if (digit > 0)
    	*(digits + 1) = digit;
    else
    	*(digits + 1) = 0;

    // Now the 3rd digit
    *(digits + 2) = part_int % 10;

    // The last digit
    *(digits + 3) = part_float;
}

void TnC_read_clock(unsigned char * clock) {
	DS3231_read_time(clock);
}

void TnC_convert_clock(unsigned char * clock, unsigned char * digits) {
	unsigned char hour, minute;

	minute = BCD_to_DEC(*(clock + 1));
	hour = BCD_to_DEC(*(clock + 2));

	*digits = hour / 10;
	*(digits + 1) = hour % 10;
	*(digits + 2) = minute / 10;
	*(digits + 3) = minute % 10;
}

void TnC_beep(unsigned char beep) {
	if (beep)
		TnP_FUNC_BTN_OUT &= ~TnP_BUZZ_PIN;
	else
		TnP_FUNC_BTN_OUT |= TnP_BUZZ_PIN;
}

void TnC_stop_beep() {
	TnP_FUNC_BTN_OUT |= TnP_BUZZ_PIN;
}

void TnC_convert_year(unsigned char * clock, unsigned char * digits) {
	unsigned char year;

	year = BCD_to_DEC(*(clock + 6));

	*digits = 2;
	*(digits + 1) = 0;
	*(digits + 2) = year / 10;
	*(digits + 3) = year % 10;
}

void TnC_convert_mon_dat(unsigned char * clock, unsigned char * digits) {
	unsigned char month, date;

	date = BCD_to_DEC(*(clock + 4));
	month = BCD_to_DEC(*(clock + 5));

	*digits = month / 10;
	*(digits + 1) = month % 10;
	*(digits + 2) = date / 10;
	*(digits + 3) = date % 10;
}

void TnC_display(unsigned char * digits, unsigned char dot_pos, unsigned char blink_dot, unsigned char leading_zero) {
	if (*digits == 0x40) { // All display dash
		fD7S_show_minus(0);
		delay_us(10);
		fD7S_show_minus(1);
		delay_us(10);
		fD7S_show_minus(2);
		delay_us(10);
		fD7S_show_minus(3);
		delay_us(10);
	} else {
		// 1st digit
		if (*digits == 0x80)
			fD7S_show_minus(0);
		else {
			if (*digits != 0 || leading_zero) {
				if (dot_pos == 1)
					fD7S_show_digit(*digits, 0, blink_dot);
				else
					fD7S_show_digit(*digits, 0, 0);
			}
		}
		delay_us(10);
		// 2nd digit
		if (((*digits == 0 || *digits == 0x80) && *(digits+1) == 0) && !leading_zero) {
			// Do nothing
		} else {
			if (dot_pos == 2)
				fD7S_show_digit(*(digits + 1), 1, blink_dot);
			else
				fD7S_show_digit(*(digits + 1), 1, 0);
		}
		delay_us(10);
		// 3rd digit
		if (dot_pos == 3)
			fD7S_show_digit(*(digits + 2), 2, blink_dot);
		else
			fD7S_show_digit(*(digits + 2), 2, 0);
		delay_us(10);
		// 4th digit
		if (dot_pos == 4)
			fD7S_show_digit(*(digits + 3), 3, blink_dot);
		else
			fD7S_show_digit(*(digits + 3), 3, 0);
		delay_us(10);
	}
}

void TnC_show_time_conf(unsigned char * clock, unsigned char * alarm1, unsigned char * alarm2, unsigned char * rtc_conf, unsigned char conf_item_idx, unsigned char blink) {
	unsigned char hour, minute, A1_hour, A1_minute, A2_hour, A2_minute;
	unsigned char cen_01 = 2;
	unsigned char cen_02 = 0;
	unsigned char year;
	unsigned char month, date;
	unsigned char letter_A = 0xEE;
	unsigned char letter_n = 0x2A;
	unsigned char letter_o = 0x3A;
	unsigned char letter_F = 0x8E;

	minute = BCD_to_DEC(*(clock + 1));
	hour = BCD_to_DEC(*(clock + 2));
	year = BCD_to_DEC(*(clock + 6));
	date = BCD_to_DEC(*(clock + 4));
	month = BCD_to_DEC(*(clock + 5));

	A1_hour = BCD_to_DEC(*(alarm1 + 2));
	A1_minute = BCD_to_DEC(*(alarm1 + 1));
	A2_hour = BCD_to_DEC(*(alarm2 + 1));
	A2_minute = BCD_to_DEC(*alarm2);

	switch(conf_item_idx) {
	case 0: // Blink hour
		if (blink) {
			fD7S_show_digit(hour / 10, 0, 0);
			delay_us(10);
			fD7S_show_digit(hour % 10, 1, 1);
			delay_us(10);
		}
		fD7S_show_digit(minute / 10, 2, 0);
		delay_us(10);
		fD7S_show_digit(minute % 10, 3, 0);
		delay_us(10);
		break;
	case 1: // Blink minute
		fD7S_show_digit(hour / 10, 0, 0);
		delay_us(10);
		fD7S_show_digit(hour % 10, 1, 1);
		delay_us(10);
		if (blink) {
			fD7S_show_digit(minute / 10, 2, 0);
			delay_us(10);
			fD7S_show_digit(minute % 10, 3, 0);
			delay_us(10);
		}
		break;
	case 2: // Blink year
		fD7S_show_digit(cen_01, 0, 0);
		delay_us(10);
		fD7S_show_digit(cen_02, 1, 0);
		delay_us(10);
		if (blink) {
			fD7S_show_digit(year / 10, 2, 0);
			delay_us(10);
			fD7S_show_digit(year % 10, 3, 1);
			delay_us(10);
		}
		break;
	case 3: // Blink month
		if (blink) {
			fD7S_show_digit(month / 10, 0, 0);
			delay_us(10);
			fD7S_show_digit(month % 10, 1, 1);
			delay_us(10);
		}
		fD7S_show_digit(date / 10, 2, 0);
		delay_us(10);
		fD7S_show_digit(date % 10, 3, 0);
		delay_us(10);
		break;
	case 4: // Blink date
		fD7S_show_digit(month / 10, 0, 0);
		delay_us(10);
		fD7S_show_digit(month % 10, 1, 1);
		delay_us(10);
		if (blink) {
			fD7S_show_digit(date / 10, 2, 0);
			delay_us(10);
			fD7S_show_digit(date % 10, 3, 0);
			delay_us(10);
		}
		break;
	case 5: // Blink A1 on:of
		fD7S_show_char(letter_A, 0, 0);
		delay_us(10);
		fD7S_show_digit(1, 1, 1);
		delay_us(10);
		if (blink) {
			fD7S_show_char(letter_o, 2, 0);
			delay_us(10);
			if (*rtc_conf & 0x01)
				fD7S_show_char(letter_n, 3, 1);
			else
				fD7S_show_char(letter_F, 3, 1);
			delay_us(10);
		}
		break;
	case 6: // Blink A1 hour
		if (blink) {
			fD7S_show_digit(A1_hour / 10, 0, 0);
			delay_us(10);
			fD7S_show_digit(A1_hour % 10, 1, 1);
			delay_us(10);
		}
		fD7S_show_digit(A1_minute / 10, 2, 0);
		delay_us(10);
		fD7S_show_digit(A1_minute % 10, 3, 0);
		delay_us(10);
		break;
	case 7: // Blink A1 minute
		fD7S_show_digit(A1_hour / 10, 0, 0);
		delay_us(10);
		fD7S_show_digit(A1_hour % 10, 1, 1);
		delay_us(10);
		if (blink) {
			fD7S_show_digit(A1_minute / 10, 2, 0);
			delay_us(10);
			fD7S_show_digit(A1_minute % 10, 3, 0);
			delay_us(10);
		}
		break;
	case 8: // Blink A2 on:of
		fD7S_show_char(letter_A, 0, 0);
		delay_us(10);
		fD7S_show_digit(2, 1, 1);
		delay_us(10);
		if (blink) {
			fD7S_show_char(letter_o, 2, 0);
			delay_us(10);
			if (*rtc_conf & 0x02)
				fD7S_show_char(letter_n, 3, 1);
			else
				fD7S_show_char(letter_F, 3, 1);
			delay_us(10);
		}
		break;
	case 9: // Blink A2 hour
		if (blink) {
			fD7S_show_digit(A2_hour / 10, 0, 0);
			delay_us(10);
			fD7S_show_digit(A2_hour % 10, 1, 1);
			delay_us(10);
		}
		fD7S_show_digit(A2_minute / 10, 2, 0);
		delay_us(10);
		fD7S_show_digit(A2_minute % 10, 3, 0);
		delay_us(10);
		break;
	case 10: // Blink A2 minute
		fD7S_show_digit(A2_hour / 10, 0, 0);
		delay_us(10);
		fD7S_show_digit(A2_hour % 10, 1, 1);
		delay_us(10);
		if (blink) {
			fD7S_show_digit(A2_minute / 10, 2, 0);
			delay_us(10);
			fD7S_show_digit(A2_minute % 10, 3, 0);
			delay_us(10);
		}
		break;
	}
}

void TnC_clock_add_one(unsigned char item_idx, unsigned char * clock) {
	unsigned char item_cache;

	switch (item_idx) { // Add time
	case 0:
		item_cache = BCD_to_DEC(*(clock + 2));
		item_cache++; // Add hour
		if (item_cache == 24) item_cache = 0;
		*(clock + 2) = DEC_to_BCD(item_cache);
		break;
	case 1:
		item_cache = BCD_to_DEC(*(clock + 1));
		item_cache++; // Add minute
		if (item_cache == 60) item_cache = 0;
		*(clock + 1) = DEC_to_BCD(item_cache);
		break;
	case 2:
		item_cache = BCD_to_DEC(*(clock + 6));
		item_cache++; // Add year
		if (item_cache == 100) item_cache = 0;
		*(clock + 6) = DEC_to_BCD(item_cache);
		break;
	case 3:
		item_cache = BCD_to_DEC(*(clock + 5));
		item_cache++; // Add month
		if (item_cache == 13) item_cache = 1;
		*(clock + 5) = DEC_to_BCD(item_cache);
		break;
	case 4:
		item_cache = BCD_to_DEC(*(clock + 4));
		item_cache++; // Add date
		if (item_cache == 32) item_cache = 1;
		*(clock + 4) = DEC_to_BCD(item_cache);
		break;
	}
}

void TnC_clock_minus_one(unsigned char item_idx, unsigned char * clock) {
	unsigned char item_cache;

	switch (item_idx) { // Minus time
	case 0:
		item_cache = BCD_to_DEC(*(clock + 2));
		if (item_cache == 0) item_cache = 24;
		item_cache--; // Minus hour
		*(clock + 2) = DEC_to_BCD(item_cache);
		break;
	case 1:
		item_cache = BCD_to_DEC(*(clock + 1));
		if (item_cache == 0) item_cache = 60;
		item_cache--; // Minus minute
		*(clock + 1) = DEC_to_BCD(item_cache);
		break;
	case 2:
		item_cache = BCD_to_DEC(*(clock + 6));
		if (item_cache == 0) item_cache = 100;
		item_cache--; // Minus year
		*(clock + 6) = DEC_to_BCD(item_cache);
		break;
	case 3:
		item_cache = BCD_to_DEC(*(clock + 5));
		if (item_cache == 1) item_cache = 13;
		item_cache--; // Minus month
		*(clock + 5) = DEC_to_BCD(item_cache);
		break;
	case 4:
		item_cache = BCD_to_DEC(*(clock + 4));
		if (item_cache == 1) item_cache = 31;
		item_cache--; // Minus date
		*(clock + 4) = DEC_to_BCD(item_cache);
		break;
	}
}

void TnC_alarm_add_one(unsigned char item_idx, unsigned char * alarm) {
	unsigned char item_cache;

	switch (item_idx) { // Add time
	case 6:
		item_cache = BCD_to_DEC(*(alarm + 2));
		item_cache++; // Add alarm 1 hour
		if (item_cache == 24) item_cache = 0;
		*(alarm + 2) = DEC_to_BCD(item_cache);
		break;
	case 7:
		item_cache = BCD_to_DEC(*(alarm + 1));
		item_cache++; // Add alarm 1 minute
		if (item_cache == 60) item_cache = 0;
		*(alarm + 1) = DEC_to_BCD(item_cache);
		break;
	case 9:
		item_cache = BCD_to_DEC(*(alarm + 1));
		item_cache++; // Add alarm 2 hour
		if (item_cache == 24) item_cache = 0;
		*(alarm + 1) = DEC_to_BCD(item_cache);
		break;
	case 10:
		item_cache = BCD_to_DEC(*alarm);
		item_cache++; // Add alarm 2 minute
		if (item_cache == 60) item_cache = 0;
		*alarm = DEC_to_BCD(item_cache);
		break;
	}
}

void TnC_alarm_minus_one(unsigned char item_idx, unsigned char * alarm) {
	unsigned char item_cache;

	switch (item_idx) { // Minus time
	case 6:
		item_cache = BCD_to_DEC(*(alarm + 2));
		if (item_cache == 0) item_cache = 24;
		item_cache--; // Minus alarm 1 hour
		*(alarm + 2) = DEC_to_BCD(item_cache);
		break;
	case 7:
		item_cache = BCD_to_DEC(*(alarm + 1));
		if (item_cache == 0) item_cache = 60;
		item_cache--; // Minus alarm 1 minute
		*(alarm + 1) = DEC_to_BCD(item_cache);
		break;
	case 9:
		item_cache = BCD_to_DEC(*(alarm + 1));
		if (item_cache == 0) item_cache = 24;
		item_cache--; // Minus alarm 2 hour
		*(alarm + 1) = DEC_to_BCD(item_cache);
		break;
	case 10:
		item_cache = BCD_to_DEC(*alarm);
		if (item_cache == 0) item_cache = 60;
		item_cache--; // Minus alarm 2 minute
		*alarm = DEC_to_BCD(item_cache);
		break;
	}
}

void TnC_stop_alarm() {
	DS3231_clear_alarm_FG();
}

void TnC_reset_RTC() {
	DS3231_set_init();
}

void TnC_bunch_read_RTC(unsigned char * clock, unsigned char * alarm1, unsigned char * alarm2, unsigned char * rtc_conf) {
	DS3231_read_time(clock); // Read current time
	DS3231_read_alarms(alarm1, alarm2); // Read alarms
	DS3231_read_RTCconf(rtc_conf); // Read configuration
}

void TnC_save_time(unsigned char * clock) {
	DS3231_set_time(clock);
}

void TnC_save_alarm(unsigned char * alarm1, unsigned char * alarm2) {
	DS3231_set_alarms(alarm1, alarm2);
}

void TnC_save_RTCconf(unsigned char * rtc_conf) {
	DS3231_set_RTCconf(rtc_conf);
}

void TnC_label_CLOC() {
	unsigned char letter_C = 0x9C;
	unsigned char letter_L = 0x1C;
	unsigned char letter_o = 0x3A;

	fD7S_show_char(letter_C, 0, 0);
	delay_us(10);
	fD7S_show_char(letter_L, 1, 0);
	delay_us(10);
	fD7S_show_char(letter_o, 2, 0);
	delay_us(10);
	fD7S_show_char(letter_C, 3, 0);
	delay_us(10);
}

void TnC_label_dAtE() {
	unsigned char letter_d = 0x7A;
	unsigned char letter_A = 0xEE;
	unsigned char letter_t = 0x1E;
	unsigned char letter_E = 0x9E;

	fD7S_show_char(letter_d, 0, 0);
	delay_us(10);
	fD7S_show_char(letter_A, 1, 0);
	delay_us(10);
	fD7S_show_char(letter_t, 2, 0);
	delay_us(10);
	fD7S_show_char(letter_E, 3, 0);
	delay_us(10);
}

void TnC_label_tEnP() {
	unsigned char letter_t = 0x1E;
	unsigned char letter_E = 0x9E;
	unsigned char letter_n = 0x2A;
	unsigned char letter_P = 0xCE;

	fD7S_show_char(letter_t, 0, 0);
	delay_us(10);
	fD7S_show_char(letter_E, 1, 0);
	delay_us(10);
	fD7S_show_char(letter_n, 2, 0);
	delay_us(10);
	fD7S_show_char(letter_P, 3, 0);
	delay_us(10);
}

void Tnc_flash_minus() {
	fD7S_show_minus(3);
	delay_ms(100);
}
