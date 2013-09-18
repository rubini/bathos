#ifndef __DHT11_H__
#define __DHT11_H__
#include <stdint.h>

struct dht11 {
	int gpio;	/* which pin is used */
	int flags;	/* is the following data valid? Are we verbose? */
	uint16_t t;	/* result: 8 bit integer, 8 decimal */
	uint16_t h;	/* result: 8 bit integer, 8 decimal */
};
#define DHT11_FLAG_VALID	0x0001
#define DHT11_FLAG_VERBOSE	0x0002

/*
 * The dht11 init and job use the above structure in the void pointer
 *
 * The job takes almost 25ms without interruptions. Actually, it may
 * be split in two, because we don't need a shart 18ms for the first
 * pulse, but there is currently no support for splitting the
 * calls. The data transfer is 5ms busy, although it 
 */
extern int dht11_init(void *);
extern void *dht11_job(void *);

#endif /* __DHT11_H__ */
