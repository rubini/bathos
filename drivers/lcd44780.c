/*
 * Alessandro Rubini, 2014 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/lcd44780.h>
#include <bathos/delay.h>
#include <bathos/gpio.h>

#define DELAY_CLK 2

/* Local helper: write 4 bits */
static void lcd44780_nibble(struct lcd44780 *lcd, int nibble)
{
	int i;
	for (i = 0; i < 4; i++)
		gpio_set(lcd->gpio[LCD44780_D4 + i], nibble & (1 << i));
	udelay(DELAY_CLK);
	gpio_set(lcd->gpio[LCD44780_E], 1);
	udelay(DELAY_CLK);
	gpio_set(lcd->gpio[LCD44780_E], 0);
	udelay(DELAY_CLK);
}

/* RS =  0: instruction, 1: data */
static void lcd44780_w_byte(struct lcd44780 *lcd, int byte, int rs)
{
	gpio_set(lcd->gpio[LCD44780_RW], 0); /* write */
	gpio_set(lcd->gpio[LCD44780_RS], rs);
	lcd44780_nibble(lcd, byte >> 4);
	lcd44780_nibble(lcd, byte >> 0);
}

static int lcd44780_is_busy(struct lcd44780 *lcd, int cmd)
{
	int ret;

	/* We can check busy only if R/W is connected */
	if (lcd->gpio[LCD44780_RW] < 0) {
		/* go-home take 1.52ms under some clock: use 2ms */
		if (cmd == 0x02)
			udelay(2000);
		else
			udelay(50);
		return 0;
	}

	gpio_set(lcd->gpio[LCD44780_RW], 1); /* read */
	gpio_set(lcd->gpio[LCD44780_RS], 0);
	/* mv D7 temporarily to input mode */
	gpio_dir(lcd->gpio[LCD44780_D7], GPIO_DIR_IN, 0);
	gpio_set(lcd->gpio[LCD44780_E], 1);
	udelay(DELAY_CLK);
	gpio_set(lcd->gpio[LCD44780_E], 0);
	ret = gpio_get(lcd->gpio[LCD44780_D7]);
	udelay(DELAY_CLK);
	/* 4-bit mode: one more shot */
	gpio_set(lcd->gpio[LCD44780_E], 1);
	udelay(DELAY_CLK);
	gpio_set(lcd->gpio[LCD44780_E], 0);
	udelay(DELAY_CLK);
	/* back to output */
	gpio_dir(lcd->gpio[LCD44780_D7], GPIO_DIR_OUT, 0);
	return ret;
}

int lcd44780_cmd(struct lcd44780 *lcd, int cmd)
{
	if (VERBOSE_LCD44780)
		printf("%s: %02x\n", __func__,  cmd);
	lcd44780_w_byte(lcd, cmd, 0);
	while (lcd44780_is_busy(lcd, cmd))
		;
	return 0;
}

 int lcd44780_data(struct lcd44780 *lcd, u8 *buf, int len)
{
	int i;

	if (VERBOSE_LCD44780)
		printf("%s: %i bytes\n", __func__, len);
	for (i = 0; i < len; i++)
		lcd44780_w_byte(lcd, buf[i], 1);
	return i;
}

int lcd44780_init(struct lcd44780 *lcd)
{
	int i;
	int bits4 = (lcd->flags & LCD44780_FLAG_8BITS) == 0;
	if (!bits4) {
		printf("%s: only %s mode supported by now\n", __func__,
		       "4-bits");
		return -1;
	}
	if (lcd->flags & LCD44780_FLAG_1LINE) {
		printf("%s: only %s mode supported by now\n", __func__,
		       "2-lines");
		return -1;
	}

	/*
	 * Turn all gpios to output mode and zero by default.
	 * Any at -1 is not existing. If 4 bits ignore D3..D0 (can be 0)
	 */
	for (i = 0; i < LCD44780_GPIOS; i++) {
		if (bits4 && i >= LCD44780_D0 && i <= LCD44780_D3)
			continue;
		if (lcd->gpio[i] < 0)
			continue;
		if (VERBOSE_LCD44780)
			printf("%s: use gpio %2i for index %i\n", __func__,
			       lcd->gpio[i], i);
		gpio_dir_af(lcd->gpio[i], GPIO_DIR_OUT, 0, GPIO_AF_GPIO);
	}

	/*
	 * Trick: force to 8-bit mode, in a way that works in both modes.
	 * This allows the same code to run after hw reset and after sw reset.
	 * However, we had an enable pulse, so we are 4-bit bout out of phase.
	 * Thus, do it twice: if the second is lost, the second is caught.
	 */
	lcd44780_cmd(lcd, 0x33);
	lcd44780_cmd(lcd, 0x33);

	/* Now, we are 8-bit mode for sure, go on, set 4-bit mode */
	if (VERBOSE_LCD44780)
		printf("setting to 4 bit mode (assume it is reset)\n");
	lcd44780_nibble(lcd, 0x2);

	/* We are now 4-bits, and synced. Continue the setup */
	if (VERBOSE_LCD44780)
		printf("go on setting stuff...");
	lcd44780_cmd(lcd, 0x20); /* again 4 bits, 2 lines, etc */
	lcd44780_cmd(lcd, 0x0c); /* turn on, etc */
	lcd44780_cmd(lcd, 0x06); /* entry mode */
	lcd44780_cmd(lcd, 0x01); /* clear the screen */

	if (VERBOSE_LCD44780)
		printf("done\n");
	return 0;
}

