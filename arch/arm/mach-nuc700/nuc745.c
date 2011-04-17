/*
 * linux/arch/arm/mach-nuc700/nuc745.c
 *
 * Copyright (c) 2011 Nuvoton corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * NUC745 cpu support
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

/* define specific CPU platform device */

static struct platform_device *nuc745_dev[] __initdata = {
	&nuc700_device_rtc,
};

/*Init NUC710 evb read id*/

void __init nuc745_read_id(void)
{
	nuc700_read_id();
}

void __init nuc745_gpio_init(void)
{
	nuc700_gpio_init();
}

/*Init NUC745 clock*/

void __init nuc745_init_clocks(void)
{
	nuc700_init_clocks();
}

/*Init NUC745 board info*/

void __init nuc745_board_init(void)
{
	nuc700_board_init(nuc745_dev, ARRAY_SIZE(nuc745_dev));
}
