/*
 * linux/arch/arm/mach-nuc700/mach-nuc710evb.c
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
#include <mach/mfp-nuc710.h>

#include "nuc710.h"

static unsigned long nuc710_multi_pin_config[] __initdata = {

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

static void __init nuc710evb_init_board(void)
{
	nuc710_board_init();
}

static void __init nuc710evb_init(void)
{
	nuc710_read_id();
	nuc710_init_clocks();
	nuc710_gpio_init();
	nuc700_mfp_config(ARRAY_AND_SIZE(nuc710_multi_pin_config));
}

MACHINE_START(NUC710EVB, "NUC710EVB")
	/* Maintainer: Wan ZongShun */
	.boot_params	= 0,
	.init_irq	= nuc700_init_irq,
	.init_machine	= nuc710evb_init_board,
	.init_early	= nuc710evb_init,
	.timer		= &nuc700_timer,
MACHINE_END
