/*
 * ds18b20.h
 *
 * Simple communication with DS18B20. No ROM search function.
 *
 * HouYu Li <karajan_ii@hotmail.com>
 *
 * Does not apply any license. Use as you wish!
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#define DS18B20_DIR		P1DIR
#define DS18B20_IN		P1IN
#define DS18B20_OUT		P1OUT
#define DS18B20_PIN		BIT5

unsigned char DS18B20_init();
void DS18B20_write_bit(unsigned char bit);
unsigned char DS18B20_read_bit();
void DS18B20_write_byte(unsigned char byte);
unsigned char DS18B20_read_byte();

#endif /* DS18B20_H_ */
