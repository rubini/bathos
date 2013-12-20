/*
 * This task is used to test reading jiffies: I want to know how much it
 * takes to read jiffies and wheter I get inconsistent results.
 * It uses 100% CPU: build with "TASK-y=test-jiffies.o"
 */

#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/delay.h>

#define NSEC 3

static void *test(void *unused)
{
	unsigned long timeout, j1, j2, i;
	int err = 0;

	/* We are scheduled at the beginning of a jiffie, read it */
	j1 = jiffies;
	timeout = j1 + NSEC * HZ;
	for (i = 0; ; i++) {
		j2 = jiffies;
		if (j2 == timeout)
			break;
		if (j2 - j1 > 1) {
			err++;
			printf("%li, %li\n", j1, j2);
			j2 = j1; /* good value */
		} else {
			j1 = j2;
		}
	}

	udelay(100); /* force linking the udelay library */

	printf("%i seconds: %li loops, %i errors\n", NSEC, i, err);
	return 0;
}

static struct bathos_task __task t_testjiffies = {
	.name = "test-jiffies",
	.job = test,
	.period = NSEC + 1,
};
