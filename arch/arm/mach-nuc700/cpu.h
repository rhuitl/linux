/*
 * arch/arm/mach-w90x900/cpu.h
 *
 * Based on linux/include/asm-arm/plat-s3c24xx/cpu.h by Ben Dooks
 *
 * Copyright (c) 2008 Nuvoton technology corporation
 * All rights reserved.
 *
 * Header file for NUC900 CPU support
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#define NUC700_8250PORT(name)					\
{								\
	.membase	= name##_PA,				\
	.mapbase	= name##_PA,				\
	.irq		= IRQ_##name,				\
	.uartclk	= 11313600,				\
	.regshift	= 2,					\
	.iotype		= UPIO_MEM,				\
	.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,	\
}

/*Cpu identifier register*/

#define NUC700PDID	NUC700_PA_GCR
#define NUC710_CPUID	0x0710
#define NUC740_CPUID	0x0740
#define NUC745_CPUID	0x0745

/* extern file from cpu.c */

extern void nuc700_clock_source(struct device *dev, unsigned char *src);
extern void nuc700_init_clocks(void);
extern void nuc700_read_id(void);
extern void nuc700_board_init(void);
extern void nuc700_gpio_init(void);

/* for either public between 710 and 745, or between 745 and 740 */

extern struct platform_device nuc700_serial_device;
extern struct platform_device nuc700_device_fmi;
extern struct platform_device nuc700_device_rtc;
