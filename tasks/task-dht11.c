/*
 * A timely task to print temperature and humidity
 *
 * Alessandro Rubini, 2013 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/dht11.h>
#include <arch/hw.h>

static struct dht11 d = {
	.gpio = 10,
	.flags = DHT11_FLAG_VERBOSE,
};

static struct bathos_task __task t_dht11 = {
	.name = "dht11",
	.period = 2 * HZ,
	.init = dht11_init,
	.job = dht11_job,
	.arg = &d,
	.release = HZ/2,
};
