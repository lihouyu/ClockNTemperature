/**
 * The main program code
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#include <msp430g2553.h>

#include "tp_n_haclk.h"
#include "4d7s_display.h"

#include "delay.h"

unsigned char		op_mode = 1;					// Operation mode. Default 1: display info. 2: Set time
unsigned char		adj_item = 1;					// 1: minute, 2: hour, 4: date, 5: month, 6: year
unsigned char		show_mode = 1;

unsigned int		label_cycle = 2000;
unsigned int		disp_cycle = 500;

unsigned char		func = 0;
unsigned char		click = 0;

unsigned int		t_count = 0;
unsigned int		t_count1 = 0;
unsigned char		t_blink500ms = 0;
unsigned char		t_blink = 0;
unsigned char		t_blinkx5 = 0;
unsigned char		t_blink250ms = 0;
unsigned char		t_blink125ms = 0;
unsigned char		en_beep = 0;

unsigned char		tp_data[2] = {0, 0};
unsigned char		clock[7] = {0, 0, 0, 0, 0, 0, 0};
unsigned char		alarm1[4] = {0, 0, 0, 0x80};
unsigned char 		alarm2[3] = {0, 0, 0x80};
unsigned char		rtc_conf = 0;
unsigned char		digits[4] = {0, 0, 0, 0};

int					conf_item_idx = 0;	// Config items
										// 0: hour, 1: minute, 2: year, 3: month, 4: date
										// 5: Alarm 1 on:off, 6: Alarm 1 hour, 7: Alarm 1 minute
										// 8: Alarm 2 on:off, 9: Alarm 2 hour, 10: Alarm 2 minute

unsigned char		load_mask = 0x00;	// BIT1: Read RTC time registers
										// BIT2: Read temperature register data
										// BIT3: Read RTC time, alarm, configure registers
										// BIT4: Clear RTC alarm status flag
										// BIT5: Reset RTC time, alarm, configure registers
										// BIT6: Save RTC time registers or alarm & configure registers

/*
 * main.c
 */
void main(void) {
	unsigned int i;

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    BCSCTL1 = CALBC1_12MHZ;
    DCOCTL = CALDCO_12MHZ;

	__enable_interrupt();

	TnC_init();
	fD7S_init();

	// Timer0 for loading data bit repeatedly
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_2 + MC_1 + ID_2;
	TA0CCR0 = 15000;

	// Timer1 for set blink switches
	TA1CCTL0 = CCIE;
	TA1CTL = TASSEL_2 + MC_1 + ID_2;
	TA1CCR0 = 15000;

	// By default, we are showing clock
	for (i = 0; i < label_cycle; i++) {
			TnC_label_CLOC(); // The label
			fD7S_wipe();
	}

	while(1) {
		if (op_mode == 1) {
			if (en_beep)
				TnC_beep(t_blink250ms);
			if (load_mask & BIT1) { // Read time
				TnC_read_clock(clock);
				load_mask &= ~BIT1;
			}
			if (load_mask & BIT2) { // Read temperature
				TnC_read_temperature(tp_data);
				load_mask &= ~BIT2;
			}
			if (load_mask & BIT4) { // Clear alarm int flag
				TnC_stop_alarm();
				load_mask &= ~BIT4;
			}
			if (load_mask & BIT5) { // Reset time
				TnC_reset_RTC(); // The default is 2012-12-31 23:59:59
				TnC_read_clock(clock);
				show_mode = 1;
				t_count = 0;
				load_mask &= ~BIT5;
			}
			switch (show_mode) {
			case 1:
				// Show clock
				TnC_convert_clock(clock, digits);
				for (i = 0; i < disp_cycle; i++){
					TnC_display(digits, 2, t_blink500ms, 1);
					fD7S_wipe();
				}
				break;
			case 2:
				// Show date
				if (t_blinkx5)
					TnC_convert_year(clock, digits);
				else
					TnC_convert_mon_dat(clock, digits);

				for (i = 0; i < disp_cycle; i++){
					if (t_blinkx5)
						TnC_display(digits, 4, 1, 1);
					else
						TnC_display(digits, 2, 1, 1);
					fD7S_wipe();
				}
				break;
			case 3:
				// Show temperature
				TnC_convert_temperature(tp_data, digits);
				for (i = 0; i < disp_cycle; i++) {
					TnC_display(digits, 3, 1, 0);
					fD7S_wipe();
				}
				break;
			}
		}
		if (op_mode == 2) {
			if (load_mask & BIT3) { // Read all data
				TnC_bunch_read_RTC(clock, alarm1, alarm2, &rtc_conf);
				clock[0] = 0; // Reset second to 0 by default
				alarm1[0] = 0; // Alarm 1 second is default to 0
				load_mask &= ~BIT3;
			}
			if (load_mask & BIT6) { // Save changed data
				//TnC_clock_add_one(conf_item_idx, clock); // Temprary Fix data - 1 bug
				if (conf_item_idx >= 0 && conf_item_idx <= 4)
					TnC_save_time(clock); // Save time
				if (conf_item_idx >= 5 && conf_item_idx <= 10) {
					TnC_save_alarm(alarm1, alarm2); // Save alarm
					TnC_save_RTCconf(&rtc_conf); // Save configuration
				}
				op_mode = 1; // Swith to view mode (1)
				show_mode = 1; // Reset show mode as well
				TnC_read_clock(clock);
				t_count = 0;
				conf_item_idx = 0; // Reset configure item
				load_mask &= ~BIT6;
			}
			for (i = 0; i < disp_cycle; i++) {
				TnC_show_time_conf(clock, alarm1, alarm2, &rtc_conf, conf_item_idx, t_blink500ms);
				fD7S_wipe();
			}
		}
	}
}

// Timer0 A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void) {
	t_count++;
	// Read time every 1 second
	if (t_count % 200 == 0 && op_mode == 1 && (show_mode == 1 || show_mode == 2)) {
		load_mask |= BIT1;
	}
	// Read temperature every 1.5 second
	if (t_count % 300 == 0 && op_mode == 1 && show_mode == 3) {
		load_mask |= BIT2;
	}
}

// Timer1 A0 for blink variables
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
	t_count1++;
	// Blink for every 125ms
	if (t_count1 % 25 == 0)
		t_blink125ms ^= 1;
	// Blink for every 250ms
	if (t_count1 % 50 == 0)
		t_blink250ms ^= 1;
	// Blink for every 500ms
	if (t_count1 % 100 == 0)
		t_blink500ms ^= 1;
	// Blink for every second
	if (t_count1 % 200 == 0)
		t_blink ^= 1;
	// Blink for every 5 second
	if (t_count1 % 1000 == 0)
		t_blinkx5 ^= 1;
}

/**
 * Function button interrupts
 */
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {
	unsigned int i;

	fD7S_wipe();

	if (!(TnP_FUNC_BTN_IN & TnP_RTCINT_PIN)) { // Alarm interrup
		en_beep = 1;
	}
	if (!(TnP_FUNC_BTN_IN & TnP_FUNC_BTN_1)) { // Function 1, short click.
		func = 1;
		delay_ms(500);
		if (!(TnP_FUNC_BTN_IN & TnP_FUNC_BTN_1))
			click = 2; // Function 1, long click;
		else
			click = 1; // Function 1, short click;
	}
	if (!(TnP_FUNC_BTN_IN & TnP_FUNC_BTN_2)) { // Function 2, short click.
		func = 2;
		delay_ms(500);
		if (!(TnP_FUNC_BTN_IN & TnP_FUNC_BTN_2))
			click = 2; // Function 2, long click;
		else
			click = 1; // Function 2, short click;
	}

	if (op_mode == 1) { // In view mode (1)
		if (func == 1 && click == 1) {	// Function 1, short click loops the content being shown on display
										// 1. Clock
										// 2. Year. + Month.Date
										// 3. Temperature from DS18B20
			if (show_mode == 3)
				show_mode = 1;
			else
				show_mode++;

			for (i = 0; i < label_cycle; i++) { // Display the content type label according to the show mode.
				switch (show_mode) {
				case 1:
					TnC_label_CLOC(); // The label
					break;
				case 2:
					TnC_label_dAtE(); // The label
					// Reset t_count1 and t_blinkx5 so that we are showing year at first
					t_count1 = 0;
					t_blinkx5 = 1;
					break;
				case 3:
					TnC_label_tEnP(); // The label
					break;
				}
				fD7S_wipe();
			}
			digits[0] = 0x40; // Force to show all minus
			t_count = 0;
		}
		if (func == 1 && click == 2) { // Function 1, long click
			op_mode = 2; // Switch to config mode (2)
			load_mask |= BIT3;
		}
		if (func == 2 && click == 1) { // Function 2, short click
			load_mask |= BIT4;
			en_beep = 0;
		}
		if (func == 2 && click == 2) { // Function 2, long click
			load_mask |= BIT5;
		}
	} else if (op_mode == 2) { // In config mode (2)
		if (func == 1 && click == 2) { // Function 1, long click
			load_mask |= BIT6;
		}
		if (func == 2 && click == 2) { // Function 2, long click
			if (conf_item_idx == 10) // Loop the config item
				conf_item_idx = 0;
			else
				conf_item_idx++;
			t_blink500ms = 1; // Reset blink counter so that we show numbers from the beginning of the count
			t_count1 = 0;
		}
		if (func == 1 && click == 1) { // Function 1, short click
			if (conf_item_idx >= 0 && conf_item_idx <= 4)
				TnC_clock_minus_one(conf_item_idx, clock); // Operate time value
			if (conf_item_idx == 5)
				rtc_conf ^= 0x01; // Toggle alarm 1 switch
			if (conf_item_idx >= 6 && conf_item_idx <= 7)
				TnC_alarm_minus_one(conf_item_idx, alarm1); // Operate alarm1 value
			if (conf_item_idx == 8)
				rtc_conf ^= 0x02; // Toggle alarm 2 switch
			if (conf_item_idx >= 9 && conf_item_idx <= 10)
				TnC_alarm_minus_one(conf_item_idx, alarm2); // Operate alarm2 value
			t_blink500ms = 1; // Reset blink counter so that we show numbers from the beginning of the count
			t_count1 = 0;
		}
		if (func == 2 && click == 1) { // Function 2, short click
			if (conf_item_idx >= 0 && conf_item_idx <= 4)
				TnC_clock_add_one(conf_item_idx, clock); // Operate time value
			if (conf_item_idx == 5)
				rtc_conf ^= 0x01; // Toggle alarm 1 switch
			if (conf_item_idx >= 6 && conf_item_idx <= 7)
				TnC_alarm_add_one(conf_item_idx, alarm1); // Operate alarm1 value
			if (conf_item_idx == 8)
				rtc_conf ^= 0x02; // Toggle alarm 2 switch
			if (conf_item_idx >= 9 && conf_item_idx <= 10)
				TnC_alarm_add_one(conf_item_idx, alarm2); // Operate alarm2 value
			t_blink500ms = 1; // Reset blink counter so that we show numbers from the beginning of the count
			t_count1 = 0;
		}
	}

	//delay_us(500);
	if (click == 2) {
		Tnc_flash_minus();
		fD7S_wipe();
	}

	// Reset bit
	func = 0;
	click = 0;

	// Reset button interrupt
	TnC_clear_btn_int();
}
