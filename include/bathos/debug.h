/*
 * This is a boring list of things turning config symbols into integers
 */
#ifdef CONFIG_VERBOSE_TASK_INIT
#  define VERBOSE_TASK_INIT 1
#else
#  define VERBOSE_TASK_INIT 0
#endif

