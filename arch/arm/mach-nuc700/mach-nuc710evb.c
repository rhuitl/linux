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
#include <linux/serial_8250.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <mach/map.h>
#include <mach/mfp-nuc710.h>
#include <mach/regs-serial.h>

#include <linux/mtd/physmap.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

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

#ifdef CONFIG_NUC700_UART1
	GPIO7_TXD1,
	GPIO8_RXD1,
#endif

#ifdef CONFIG_NUC700_UART2
	GPIO9_TXD2,
	GPIO10_RXD2,
#endif

#ifdef CONFIG_NUC700_UART1_CTSRTS
	GPIO9_CTS1,
	GPIO10_RTS1,
#endif

#ifdef CONFIG_NUC700_UART3
	GPIO1_UART_DTR3,
	GPIO2_UART_DSR3,
	GPIO3_UART_TXD3,
	GPIO4_UART_RXD3,
#endif
	/* enable sd */
	GPIO22_SDCD,
	GPIO24_SDDAT3,
	GPIO25_SDDAT2,
	GPIO26_SDDAT1,
	GPIO27_SDDAT0,
	GPIO28_SDCLK,
	GPIO29_SDCMD,

};

/*NUC710 evb norflash driver data */

static struct mtd_partition nuc710_flash_partitions[] = {

	{
		.name	=	"Rootfs Partition (2M)",
		.size	=	0x200000,
		.offset	=	0x200000,
	}
};

static struct physmap_flash_data nuc710_flash_data = {
	.width		=	2,
	.parts		=	nuc710_flash_partitions,
	.nr_parts	=	ARRAY_SIZE(nuc710_flash_partitions),
};

static struct resource nuc710_flash_resources[] = {
	{
		.start	=	CONFIG_FLASH_MEM_BASE,
		.end	=	CONFIG_FLASH_MEM_BASE + CONFIG_FLASH_SIZE - 1,
		.flags	=	IORESOURCE_MEM,
	}
};

/* Initial serial platform data */

struct plat_serial8250_port nuc710_uart_data[] = {
	NUC700_8250PORT(UART0),
#ifdef CONFIG_NUC700_UART1
	NUC700_8250PORT(UART1),
#endif
#ifdef CONFIG_NUC700_UART2
	NUC700_8250PORT(UART2),
#endif
#ifdef CONFIG_NUC700_UART3
	NUC700_8250PORT(UART3),
#endif
	{},
};

void __init nuc710_uart_clk_enable(void) {

#ifdef CONFIG_NUC700_UART1
	nuc700_uart_clk_enable(1);
#endif
#ifdef CONFIG_NUC700_UART2
	nuc700_uart_clk_enable(2);
#endif
#ifdef CONFIG_NUC700_UART3
	nuc700_uart_clk_enable(3);
#endif

}

static void __init nuc710evb_init_board(void)
{
	platform_device_register_resndata(NULL, "serial8250", PLAT8250_DEV_PLATFORM,
				NULL, 0, nuc710_uart_data, sizeof(nuc710_uart_data));

	platform_device_register_resndata(NULL, "physmap-flash",  -1,
				nuc710_flash_resources, ARRAY_SIZE(nuc710_flash_resources) , 
				&nuc710_flash_data, sizeof(nuc710_flash_data));
	nuc710_uart_clk_enable();
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
