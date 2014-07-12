/*
 * This task is a test for the lcd44780, as mounted on the
 * LPC2104 chritmas tree.
 *
 * Build with "TASK-y=test-lcd44780.o"
 */

#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/string.h>
#include <bathos/lcd44780.h>
#include <arch/gpio.h>

static struct lcd44780 lcd_instance = {
	.gpio = {
		[LCD44780_RS] = 28,
		[LCD44780_E] = 29,
		[LCD44780_RW] = -1,
		[LCD44780_D4] = 24,
		[LCD44780_D5] = 25,
		[LCD44780_D6] = 26,
		[LCD44780_D7] = 27,
	},
};

static int lcd_test_init(void *arg)
{
	struct lcd44780 *lcd = arg;

	gpio_dir_af(30, 1, 1, 0); /* lcd power */
	gpio_dir_af(16, 1, 1, 0); /* lcd backlight */
	lcd44780_init(lcd);
	printf("...\n");
	return 0;
}

static void *lcd_test(void *arg)
{
	struct lcd44780 *lcd = arg;
	u8 s[8];
	static int t;

	sprintf((char *)s, "%05x", t++ & 0xfffff);
	if (t % 10 == 0)
		printf("%s: write %s\n", __func__, s);
	lcd44780_cmd(lcd, 0x2); /* home */
	lcd44780_data(lcd, s, 5);
	return arg;
}

static struct bathos_task __task t_lcd = {
	.name ="lcd-test",
	.period = HZ/2,
	.release = HZ/2,
	.init = lcd_test_init,
	.job = lcd_test,
	.arg = &lcd_instance,
};

