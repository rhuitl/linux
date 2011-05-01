/*
 * linux/arch/arm/mach-w90x900/nuc910.c
 *
 * Based on linux/arch/arm/plat-s3c24xx/s3c244x.c by Ben Dooks
 *
 * Copyright (c) 2009 Nuvoton corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * NUC910 cpu support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <mach/hardware.h>
#include "cpu.h"
#include "clock.h"

/*Init NUC710 evb read id*/

void __init nuc710_read_id(void)
{
	nuc700_read_id();
}

void __init nuc710_gpio_init(void)
{
	nuc700_gpio_init();
}

/*Init NUC710 clock*/

void __init nuc710_init_clocks(void)
{
	nuc700_init_clocks();
}

/*Init NUC710 board info*/

void __init nuc710_board_init(void)
{
	nuc700_board_init();
}
