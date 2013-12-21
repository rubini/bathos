/*
 * Arch-dependent initialization and I/O functions
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#include <stdint.h>
#include <bathos/bathos.h>
#include <bathos/init.h>
#include <arch/hw.h>

/* We need a main function, called by libc initialization */
int main(int argc, char **argv)
{
	bathos_setup();
	bathos_main();
	return 0;
}

/*
 * There is no romcall for arch-unix and we have no lds to provide symbols
 * We strongly hope the linker will continue keeping these at the same address
 */
initcall_t  romcall_begin[0];
initcall_t  romcall_end[0];


/* And this trivially uses stdout */
void putc(int c)
{
	uint8_t ch = c;

	write(1, &ch, 1);
}
