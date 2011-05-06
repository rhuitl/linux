/*
 * linux/arch/arm/mach-nuc700/mach-nuc740evb.c
 *
 * Copyright (C) 2011 Nuvoton technology corporation.
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
#include <mach/mfp.h>

#include "nuc740.h"

static unsigned long nuc740_multi_pin_config[] __initdata = {

	/* enable ethernet */
	GPIO42_PHYRXERR,
	GPIO43_PHYCRSDV,
	GPIO44_PHYRXD0,
	GPIO45_PHYRXD1,
	GPIO46_PHYREFCLK,
	GPIO47_PHYTXEN,
	GPIO48_PHYTXD0,
	GPIO49_PHYTXD1,
	GPIO50_PHYMDIO,
	GPIO51_PHYMDC,
	
};

static void __init nuc740evb_init_board(void)
{
	nuc740_board_init();
}

static void __init nuc740evb_init(void)
{
	nuc740_read_id();
	nuc740_init_clocks();
	nuc740_gpio_init();
	nuc700_mfp_config(ARRAY_AND_SIZE(nuc740_multi_pin_config));
}

MACHINE_START(NUC740EVB, "NUC740EVB")
	/* Maintainer: Wan ZongShun */
	.boot_params	= 0,
	.init_irq	= nuc700_init_irq,
	.init_machine	= nuc740evb_init_board,
	.init_early	= nuc740evb_init,
	.timer		= &nuc700_timer,
MACHINE_END
