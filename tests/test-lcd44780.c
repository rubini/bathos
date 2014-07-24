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
#include <bathos/gpio.h>

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

	gpio_dir_af(30, GPIO_DIR_OUT, 1, GPIO_AF_GPIO); /* lcd power */
	gpio_dir_af(16,  GPIO_DIR_OUT, 1, GPIO_AF_GPIO); /* lcd backlight */
	lcd44780_init(lcd);
	return 0;
}

static void *lcd_test(void *arg)
{
	struct lcd44780 *lcd = arg;
	char str[16];
	static int s, m, h;

	sprintf(str, "%02i:%02i:%02i", h, m, s);
	lcd44780_cmd(lcd, 0x2); /* home */
	lcd44780_data(lcd, (void *)str, strlen(str));
	s = (s + 1) % 60;
	if (!s) {
		m = (m + 1) % 60;
		if (!m)
			h = (h + 1) % 100;
	}
	return arg;
}

static struct bathos_task __task t_lcd = {
	.name ="lcd-test",
	.period = HZ,
	.release = 0,
	.init = lcd_test_init,
	.job = lcd_test,
	.arg = &lcd_instance,
};

