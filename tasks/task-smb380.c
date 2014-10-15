#include <bathos/bathos.h>
#include <bathos/types.h>
#include <bathos/spi.h>
#include <bathos/gpio.h>
#include <arch/hw.h>

/* I can read one register a time, and no more. Bah... */
static  u8 odata[] = {
	0x80, 0,
};


static u8 idata[sizeof(odata)];

static struct spi_dev *dev;
static const struct spi_ibuf smb380_ibuf = {
	.len = sizeof(idata),
	.buf = idata,
};
static const struct spi_obuf smb380_obuf = {
	.len = sizeof(odata),
	.buf = odata,
};
/* SPI0 on uext (if olimex lpc-p1343) */
const struct spi_cfg smb380_config = {
	.gpio_cs = GPIO_NR(2,10),
	.freq = 1000 * 1000,
	.pol = 1,
	.phase = 1,
	.devn = 0
};

static struct spi_dev smb380_device = {
	.cfg = &smb380_config,
};

static struct spi_dev *dev;

static int smb380_init(void *unused)
{
	dev = spi_create(&smb380_device);
	if (!dev)
		return -1;

	/* Ok, now try to configure. This is what I have as default
	   00: 02 11 41 1e
	   04: 81 18 41 fc
	   08: 5d 00 00 03
	   0c: 14 96 a0 96
	   10: 00 00 a2 0d
	   14: 2e 80 00 00 (range +/-4G, 1500hz)
	*/
	odata[0] = 0x94;
	spi_xfer(dev, SPI_F_DEFAULT, &smb380_ibuf, &smb380_obuf);
	odata[0] = 0x14;
	odata[1] = (idata[1] & 0xe0) | 0x09; /* 50Hz, +/-4G */
	spi_xfer(dev, SPI_F_DEFAULT, &smb380_ibuf, &smb380_obuf);

	return 0;
}

static void *smb380_job(void *arg)
{
	int i, x, y, z;
	unsigned char v[0x1d];

	/* At each loop, get the data and printk it */
	do {
		for (i = 0x80; i <= 0x88; i++) { /* was: up to 0x9d to check */
			odata[0] = i;
			spi_xfer(dev, SPI_F_DEFAULT, &smb380_ibuf,
				 &smb380_obuf);
			v[i & 0x7f] = idata[1];
			/* printf("%02x%c", idata[1],
			   i == 0x88 ? '\n' : ' '); */
		}
		if (0) {
			for (i = 0; i <= 8; i++)
				printf(" %02x", v[i]);
			printf("\n");
		}
	}
	while (((v[2] & 1) == 0)
	       || ((v[4] & 1) == 0)
	       || ((v[6] & 1) == 0));

	x = (v[2] >> 6) | (v[3] << 2); if (x > 511) x -= 1024;
	y = (v[4] >> 6) | (v[5] << 2); if (y > 511) y -= 1024;
	z = (v[6] >> 6) | (v[7] << 2); if (z > 511) z -= 1024;
	printf("%4d  %4d %4d  %2d.%d\n", x, y, z,
	       (v[8] - 60) / 2, v[8] & 1 ? 5 : 0);
	if (0) {
		for (i = 0; i <= 0x1d; i++)
			printf(" %02x", v[i]);
		printf("\n");
	}

	return 0;
}

static struct bathos_task __task t_smb380 = {
	.name = "smb380", .period = HZ/100,
	.init = smb380_init, .job = smb380_job,
	.release = 15
};
