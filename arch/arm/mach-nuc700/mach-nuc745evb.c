/*
 * linux/arch/arm/mach-nuc700/mach-nuc745evb.c
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
#include <mach/mfp-nuc745.h>

#include <linux/mtd/physmap.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include "nuc745.h"

static unsigned long nuc745_multi_pin_config[] __initdata = {

	/* enable ethernet */
	GPIO20_PHYRXERR,
	GPIO21_PHYCRSDV,
	GPIO22_PHYRXD0,
	GPIO23_PHYRXD1,
	GPIO24_PHYREFCLK,
	GPIO25_PHYTXEN,
	GPIO26_PHYTXD0,
	GPIO27_PHYTXD1,
	GPIO28_PHYMDIO,
	GPIO29_PHYMDC,
};

/*NUC745 evb norflash driver data */

static struct mtd_partition nuc745_flash_partitions[] = {

	{
		.name	=	"Rootfs Partition (2M)",
		.size	=	0x200000,
		.offset	=	0x200000,
	}
};

static struct physmap_flash_data nuc745_flash_data = {
	.width		=	2,
	.parts		=	nuc745_flash_partitions,
	.nr_parts	=	ARRAY_SIZE(nuc745_flash_partitions),
};

static struct resource nuc745_flash_resources[] = {
	{
		.start	=	CONFIG_FLASH_MEM_BASE,
		.end	=	CONFIG_FLASH_MEM_BASE + CONFIG_FLASH_SIZE - 1,
		.flags	=	IORESOURCE_MEM,
	}
};

static void __init nuc745evb_init_board(void)
{
	platform_device_register_resndata(NULL, "physmap-flash",  -1,
				nuc745_flash_resources, ARRAY_SIZE(nuc745_flash_resources) , 
				&nuc745_flash_data, sizeof(nuc745_flash_data));
	nuc745_board_init();
}

static void __init nuc745evb_init(void)
{
	nuc745_read_id();
	nuc745_init_clocks();
	nuc745_gpio_init();
	nuc700_mfp_config(ARRAY_AND_SIZE(nuc745_multi_pin_config));
}

MACHINE_START(NUC745EVB, "NUC745EVB")
	/* Maintainer: Wan ZongShun */
	.boot_params	= 0,
	.init_irq	= nuc700_init_irq,
	.init_machine	= nuc745evb_init_board,
	.init_early	= nuc745evb_init,
	.timer		= &nuc700_timer,
MACHINE_END
