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
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <mach/hardware.h>
#include <mach/regs-gpio.h>
#include "cpu.h"
#include "clock.h"

/* nuc745 gpio group */
static struct nuc700_gpio_chip nuc745_gpio[] = {
	NUC700_GPIO_CHIP("PORT0", 0, 5, 0),
	NUC700_GPIO_CHIP("PORT1", 18, 2, 0),
	NUC700_GPIO_CHIP("PORT2", 20, 10, 0),
	NUC700_GPIO_CHIP("PORT3", 0, 0, 1),
	NUC700_GPIO_CHIP("PORT4", 30, 1, 0),
	NUC700_GPIO_CHIP("PORT5", 5, 13, 0),
	NUC700_GPIO_CHIP("PORT6", 0, 0, 1),
};

/*Init NUC710 evb read id*/

void __init nuc745_read_id(void)
{
	nuc700_read_id();
}

void __init nuc745_gpio_init(void)
{
	nuc700_gpio_init(nuc745_gpio, ARRAY_SIZE(nuc745_gpio));
}

/*Init NUC745 clock*/

void __init nuc745_init_clocks(void)
{
	nuc700_init_clocks();
}

/*Init NUC745 board info*/

void __init nuc745_board_init(void)
{
	nuc700_board_init();
}
