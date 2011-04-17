/*
 * linux/arch/arm/mach-w90x900/mach-nuc910evb.c
 *
 * Based on mach-s3c2410/mach-smdk2410.c by Jonas Dietsche
 *
 * Copyright (C) 2008 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/platform_device.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <mach/map.h>

#include "nuc740.h"

static void __init nuc740evb_init_first(void)
{
	nuc740_read_id();
	nuc740_init_clocks();
	nuc740_gpio_init();
}

static void __init nuc740evb_init(void)
{
	nuc740evb_init_first();
	nuc740_board_init();
}

MACHINE_START(NUC740EVB, "NUC740EVB")
	/* Maintainer: Wan ZongShun */
	.boot_params	= 0,
	.init_irq	= nuc700_init_irq,
	.init_machine	= nuc740evb_init,
	.timer		= &nuc700_timer,
MACHINE_END
