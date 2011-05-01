/*
 * linux/arch/arm/mach-nuc700/nuc740.c
 *
 * Copyright (c) 2011 Nuvoton corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * NUC740 cpu support
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

/*Init NUC740 evb read id*/

void __init nuc740_read_id(void)
{
	nuc700_read_id();
}

void __init nuc740_gpio_init(void)
{
	nuc700_gpio_init();
}

/*Init NUC740 clock*/

void __init nuc740_init_clocks(void)
{
	nuc700_init_clocks();
}

/*Init NUC740 board info*/

void __init nuc740_board_init(void)
{
	nuc700_board_init();
}
