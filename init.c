/* Alessandro Rubini 2009-2013, GNU GPL2 or later */
#include <bathos/bathos.h>
#include <bathos/init.h>

#define config_verbose_task_init 0

static int init_tasks(void)
{
	struct bathos_task *p;
	int ret;

	/* The list of tasks is static: no run-time creation is there, yet */
	for (p = __task_begin; p < __task_end; p++) {
		ret = 0;
		printf("Task: %s", p->name);
		if (p->init)
			ret = p->init(p->arg);
		if (ret)
			printf(": init error");
		if (config_verbose_task_init) {
			printf(" (%p, job %p, period %li, release %li)",
			       p, p->job, p->period, p->release);
		}
		printf("\n");
	}
	return 0;
}

late_initcall(init_tasks);
