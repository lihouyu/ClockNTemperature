/*
 * tp_n_haclk.h
 *
 * External Hi-Accuracy Temperature and RTC Source Boost Pack Driver
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#ifndef TP_N_HACLK_H_
#define TP_N_HACLK_H_

#define TnP_FUNC_BTN_DIR			P2DIR
#define TnP_FUNC_BTN_IE				P2IE
#define TnP_FUNC_BTN_REN			P2REN
#define TnP_FUNC_BTN_IN				P2IN
#define TnP_FUNC_BTN_OUT			P2OUT
#define TnP_FUNC_BTN_IES			P2IES
#define TnP_FUNC_BTN_IFG			P2IFG
#define TnP_FUNC_BTN_SEL			P2SEL
#define TnP_FUNC_BTN_SEL2			P2SEL2

#define TnP_FUNC_BTN_1				BIT6
#define TnP_FUNC_BTN_2				BIT7
#define TnP_BUZZ_PIN				BIT5
#define TnP_RTCINT_PIN				BIT0

void TnC_init();
void TnC_clear_btn_int();
void TnC_read_temperature(unsigned char * tp_data);
void TnC_convert_temperature(unsigned char * tp_data, unsigned char * digits);
void TnC_read_clock(unsigned char * clock);
void TnC_convert_clock(unsigned char * clock, unsigned char * digits);
void TnC_beep(unsigned char beep);
void TnC_stop_beep();
void TnC_convert_year(unsigned char * clock, unsigned char * digits);
void TnC_convert_mon_dat(unsigned char * clock, unsigned char * digits);
void TnC_display(unsigned char * digits, unsigned char dot_pos, unsigned char blink_dot, unsigned char leading_zero);
void TnC_show_time_conf(unsigned char * clock, unsigned char * alarm1, unsigned char * alarm2, unsigned char * rtc_conf, unsigned char conf_item_idx, unsigned char blink);
void TnC_clock_add_one(unsigned char item_idx, unsigned char * clock);
void TnC_clock_minus_one(unsigned char item_idx, unsigned char * clock);
void TnC_alarm_add_one(unsigned char item_idx, unsigned char * alarm);
void TnC_alarm_minus_one(unsigned char item_idx, unsigned char * alarm);
void TnC_stop_alarm();
void TnC_reset_RTC();
void TnC_bunch_read_RTC(unsigned char * clock, unsigned char * alarm1, unsigned char * alarm2, unsigned char * rtc_conf);
void TnC_save_time(unsigned char * clock);
void TnC_save_alarm(unsigned char * alarm1, unsigned char * alarm2);
void TnC_save_RTCconf(unsigned char * rtc_conf);

void TnC_label_CLOC();
void TnC_label_dAtE();
void TnC_label_tEnP();
void Tnc_flash_minus();

#endif /* TP_N_HACLK_H_ */
