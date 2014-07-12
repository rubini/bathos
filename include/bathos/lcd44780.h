/*
 * Alessandro Rubini, 2014 GNU GPL2 or later
 */
#ifndef __LCD44780_H__
#define __LCD44780_H__
#include <bathos/types.h>

#ifdef CONFIG_VERBOSE_LCD44780
#  define VERBOSE_LCD44780 1
#else
#  define VERBOSE_LCD44780 0
#endif

/* indexes in the array below */
#define LCD44780_RS	 0
#define LCD44780_E	 1
#define LCD44780_RW	 2
/* 4-bit only is supported by now: D0..3 is missing but who knows later... */
#define LCD44780_D0	 3
#define LCD44780_D1	 4
#define LCD44780_D2	 5
#define LCD44780_D3	 6
#define LCD44780_D4	 7
#define LCD44780_D5	 8
#define LCD44780_D6	 9
#define LCD44780_D7	10
#define LCD44780_GPIOS	11

#define LCD44780_FLAG_8BITS	0x0001 /* default is 4 bits */
#define LCD44780_FLAG_1LINE	0x0002 /* default is 2 lines */

struct lcd44780 {
	int gpio[LCD44780_GPIOS];
	int flags;
};

extern int lcd44780_init(struct lcd44780 *lcd);
extern int lcd44780_cmd(struct lcd44780 *lcd, int cmd);
extern int lcd44780_data(struct lcd44780 *lcd, u8 *buf, int len);

#endif /* __LCD44780_H__ */
