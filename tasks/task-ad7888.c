#include <bathos/bathos.h>
#include <bathos/types.h>
#include <bathos/spi.h>
#include <bathos/gpio.h>
#include <arch/hw.h>

/* AD7888 */

/* We always do 8 conversions, so 9 transfers of 2 bytes */
static const u8 odata[] = {
	/*
	 * 6,5,4 = CHANNEL
	 * 2 = REF => 0 = on-chip
	 * 1,0 = PM => 00 = normal operation
	 */
	0 << 3, 0x00,
	1 << 3, 0x00,
	2 << 3, 0x00,
	3 << 3, 0x00,
	4 << 3, 0x00,
	5 << 3, 0x00,
	6 << 3, 0x00,
	7 << 3, 0x00,
	0 << 3, 0x00, /* used to retrieva data */
};

static u8 idata[sizeof(odata)];

static struct spi_dev *dev;
static const struct spi_ibuf ad7888_ibuf = {
	.len = sizeof(idata),
	.buf = idata,
};
static const struct spi_obuf ad7888_obuf = {
	.len = sizeof(idata),
	.buf = odata,
};
/* Test config: SPI0 with configured CS */
const struct spi_cfg ad7888_config = {
	.gpio_cs = CONFIG_AD7888_CS_PIN,
	.freq= HZ / 100,
	.pol = 0,
	.phase = 0,
	.devn = 0
};

static struct spi_dev ad7888_device = {
	.cfg = &ad7888_config,
};

static struct spi_dev *dev;

static int ad7888_init(void *unused)
{
	dev = spi_create(&ad7888_device);
	if (!dev)
		return -1;

	return 0;
}

static void *ad7888_acq(void *arg)
{
	int i;

	/* At each loop, get the data and printk it */
	spi_xfer(dev, SPI_F_DEFAULT, &ad7888_ibuf, &ad7888_obuf);

	for (i = 0; i < 8; i++) {
		printf("%03x%c",
		       idata[2+(2*i)] << 8 | idata[3+(2*i)],
		       i == 7 ? '\n' : ' ');
	}
	return 0;
}

static struct bathos_task __task t_ad7888 = {
	.name = "ad7888", .period = HZ,
	.init = ad7888_init, .job = ad7888_acq,
	.release = 15
};
