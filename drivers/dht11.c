/*
 * Trivial dht11 procedures for bathos, Alessandro Rubini 2013, GNU GPL V2
 */
#include <bathos/bathos.h>
#include <bathos/delay.h>
#include <bathos/jiffies.h>
#include <bathos/dht11.h>
#include <arch/gpio.h>

int dht11_init(void *arg)
{
	struct dht11 *d = arg;

	gpio_dir_af(d->gpio, 0, 0, 0); /* input, AF 0 */
	d->flags &= ~DHT11_FLAG_VALID;
	if (d->flags & DHT11_FLAG_VERBOSE)
		printf("%s: initialized GPIO %i (0x%x)\n", __func__,
		       d->gpio, d->gpio);
	return 0;
}

static inline int waitbit(int gpio, int value)
{
	unsigned long j = jiffies + 20;

	while (gpio_get(gpio) != value)
		if (time_after(jiffies, j)) {
			printf("%i\n", gpio_get(gpio));
			printf("late waiting for %i: %li > %li\n", value,
			       jiffies, j);
			return -1;
		}
	return 0;
}

static int getbit(int gpio)
{
	if (waitbit(gpio, 0))
		return -1;
	/* we have a 50usec low pulse */
	if (waitbit(gpio, 1))
		return -1;
	/*
	 * High is 26-28 or 50-70 usec; so we wait, and see where we are.
	 * If we are already low, it's 0, and we have 40usec to reenter here.
	 */
	udelay(30);
	return gpio_get(gpio);
}

static int getbyte(int gpio, int *ck)
{
	int i, bit, res = 0;

	for (i = 0; i < 8; i++) {
		bit = getbit(gpio);
		if (bit < 0) {
			printf("error bit %i\n", i);
			return bit;
		}
		res = (res << 1) + bit;
	}
	if (ck)
		*ck += res;
	return res;
}

void *dht11_job(void *arg)
{
	struct dht11 *d = arg;
	int gpio = d->gpio;
	int i, ck = 0;

	/* Start pulse: 18ms minimum */
	gpio_dir(gpio, 1, 0);
	for (i = 0; i < 18; i++)
		udelay(1000);
	gpio_dir(gpio, 0, 0); /* input */

	/* Wait for the bit to become low, and high; this is the ack pulse */
	if (waitbit(gpio, 0) || waitbit(gpio, 1)) {
		printf("%s: no device\n", __func__);
		return arg;
	}

	/* Now we start reading, so data is not valid. */
	d->t = d->h = 0;
	d->flags &= ~DHT11_FLAG_VALID;

	/* boring read of 5 bytes, storing them */
	if ( (i = getbyte(gpio, &ck)) < 0)
		goto error;
	d->h = i << 8;

	if ( (i = getbyte(gpio, &ck)) < 0)
		goto error;
	d->h |= i;

	if ( (i = getbyte(gpio, &ck)) < 0)
		goto error;
	d->t = i << 8;

	if ( (i = getbyte(gpio, &ck)) < 0)
		goto error;
	d->t |= i;

	if ( (i = getbyte(gpio, NULL)) < 0)
		goto error;

	if (i != (ck & 0xff)) {
		printf("%s: cksum error\n", __func__);
		return arg;
	}

	d->flags |= DHT11_FLAG_VALID;

	if (d->flags & DHT11_FLAG_VERBOSE)
		printf("%s: T = %i, H = %i\n", __func__, d->t >> 8, d->h >> 8);

	return arg;

error:
	printf("%s: error (now %04x.%04x)\n", __func__, d->t, d->h);
	return arg;

}
