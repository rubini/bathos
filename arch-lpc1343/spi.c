/*
 * SPI interface for LPC-1343
 * Alessandro Rubini, 2013 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/types.h>
#include <bathos/delay.h>
#include <bathos/spi.h>
#include <bathos/gpio.h>
#include <bathos/jiffies.h>
#include <bathos/io.h>
#include <arch/hw.h>

#ifndef DEBUG_SPI
#define DEBUG_SPI 0
#endif

#define SPI_CS_DELAY 10

struct spi_dev *spi_create(struct spi_dev *dev)
{
	const struct spi_cfg *cfg = dev->cfg;
	u32 cr;

	if (DEBUG_SPI)
		printf("%s: gpio is %i\n", __func__, cfg->gpio_cs);

	dev->current_freq = 0;

	if (cfg->devn > 0) /* We only have SSP0 */
		return NULL;

	/* Power on the device */
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_SSP;
	regs[REG_SSPCLKDIV] = 1; /* enable, divide by 1 */

	/* Reset the device */
	regs[0x40048004 / 4] = 0; udelay(50); regs[0x40048004 / 4] = 1;

	/* Select pio2_11 for sck */
	regs[REG_SCKLOC] = 0x01; /* 0 = PIO0_10; 2 = PIO0_6 */

	/* Configure the GPIO pins (PIO0 8 and 9, PIO2 11) */
	gpio_dir_af(GPIO_NR(0,  8), GPIO_DIR_IN, 0, GPIO_AF(1));
	gpio_dir_af(GPIO_NR(0,  9), GPIO_DIR_OUT, 0, GPIO_AF(1));
	gpio_dir_af(GPIO_NR(2, 11), GPIO_DIR_OUT, 0, GPIO_AF(1));

	/* Configure the CS GPIO */
	gpio_dir_af(cfg->gpio_cs, GPIO_DIR_OUT, 1, GPIO_AF_GPIO);

	cr = 7 /* spi, 8 bits */ | (cfg->phase << 7) | (cfg->pol << 6) |
		(64 << 8 /* FIXME: serial clock rate. Slow... */);
	regs[REG_SSP0CR0] = cr;

	/* FIXME: frequency support is missing */
	regs[REG_SSP0CPSR] = 128; /* start slow (FIXME) */

	regs[REG_SSP0CR1] = 2 /* SSE - enable */;

	return dev;
}

void spi_destroy(struct spi_dev *dev)
{
	/* De-configure the GPIO pins (back to gpio mode) */
	gpio_dir_af(GPIO_NR(0,  8), GPIO_DIR_IN, 0, GPIO_AF_GPIO);
	gpio_dir_af(GPIO_NR(0,  9), GPIO_DIR_IN, 0, GPIO_AF_GPIO);
	gpio_dir_af(GPIO_NR(2, 11), GPIO_DIR_IN, 0, GPIO_AF_GPIO);

	/* Power off the device */
	regs[REG_AHBCLKCTRL] &= ~REG_AHBCLKCTRL_SSP;

	dev->cfg = NULL;
	dev->current_freq = 0;
}

/* Local functions to simplify xfer code */
static void __spi_wait_busy(void)
{
	unsigned long j = jiffies + HZ/10;

	while (1) {
		if (regs[REG_SSP0SR] & 0x04 /* RNE */)
			return;
		if (jiffies >= j)
			break;
	}
	printf("%s: timeout (SSP0SR = 0x%02x)\n", __func__,
	       (int)regs[REG_SSP0SR]);
}

static void __spi_cs(struct spi_dev *dev, int value)
{
	gpio_set(dev->cfg->gpio_cs, value);
}

/* And the core of it */
int spi_xfer(struct spi_dev *dev,
		     enum spi_flags flags,
		     const struct spi_ibuf *ibuf,
		     const struct spi_obuf *obuf)
{
	int i, len;
	u8 val;

	/* if it's both input and output, lenght must be the same */
	if (ibuf && obuf) {
		if (ibuf->len != ibuf->len)
			return -1; /* EINVAL ... */
	}
	if (!ibuf && !obuf)
		return 0; /* nothing to do */

	len = ibuf ? ibuf->len : obuf->len;


	if (DEBUG_SPI)
		printf("%s: %s", __func__, flags & SPI_F_NOINIT ? "--" : "cs");

	if ( !(flags & SPI_F_NOINIT) ) {
		udelay(SPI_CS_DELAY);
		__spi_cs(dev, 0);
		udelay(SPI_CS_DELAY);
	}
	for (i = 0; i < len; i++) {
		regs[REG_SSP0DR] = obuf ? obuf->buf[i] : 0xff;
		__spi_wait_busy();
		val = regs[REG_SSP0DR];
		if (DEBUG_SPI)
			printf(" %02x(%02x)", obuf ? obuf->buf[i] : 0xff, val);
		if (ibuf)
			ibuf->buf[i] = val;
	}

	if (DEBUG_SPI)
		printf(" %s\n", flags & SPI_F_NOFINI ? "--" : "cs");

	if ( !(flags & SPI_F_NOFINI) ) {
		udelay(SPI_CS_DELAY);
		__spi_cs(dev, 1);
		udelay(SPI_CS_DELAY);
	}
	return 0;
}
