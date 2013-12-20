#ifndef __ATMEGA_ARCH_H__
#define __ATMEGA_ARCH_H__
/* Our counter is not incremented atomically: disable interrupts */
extern volatile unsigned long _jiffies;

static inline unsigned long get_jiffies(void)
{
	unsigned long j;

	asm("cli");
	j = _jiffies;
	asm("sei");
	return j;
}

#define __get_jiffies  get_jiffies

#endif /* __ATMEGA_ARCH_H__ */
