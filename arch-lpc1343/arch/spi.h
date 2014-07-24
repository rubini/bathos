#ifndef __LPC13_SPI_H__
#define __LPC13_SPI_H__

/* SSP (Synchronous Serial Port) */

#define REG_SSP0CR0	(0x40040000 / 4)
#define REG_SSP0CR1	(0x40040004 / 4)
#define REG_SSP0DR	(0x40040008 / 4)
#define REG_SSP0SR	(0x4004000c / 4)
#define REG_SSP0CPSR	(0x40040010 / 4)
#define REG_SSP0IMSC	(0x40040014 / 4)
#define REG_SSP0RIS	(0x40040018 / 4)
#define REG_SSP0MIS	(0x4004001c / 4)
#define REG_SSP0ICR	(0x40040020 / 4)

#define REG_SCKLOC	(0x400440b0 / 4)
#endif /* __LPC13_SPI_H__ */
