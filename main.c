/*
 * Main function: a welcome message and a simple scheduler
 * Alessandro Rubini, 2009 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <arch/hw.h>

void __attribute__((noreturn)) bathos_main(void)
{
	struct bathos_task *p;
	unsigned long now;

	printf("Hello, Bathos is speaking (built on %s)\n", __DATE__);

	now = jiffies;
	for (p = __task_begin; p < __task_end; p++)
		p->release += now + 2;

	while (1) {
		struct bathos_task *t;

		for (t = p = __task_begin; p < __task_end; p++)
			if (p->release < t->release)
				t = p;
		while (time_before(jiffies, t->release))
			;
		t->arg = t->job(t->arg);
		t->release += t->period;
	}
}
