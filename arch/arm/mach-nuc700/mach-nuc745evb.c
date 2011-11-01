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
#include <linux/serial_8250.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <mach/map.h>
#include <mach/mfp-nuc745.h>
#include <mach/regs-serial.h>
#include <mach/regs-gcr.h>

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

};

/*NUC745 evb norflash driver data */

static struct mtd_partition nuc745_flash_partitions[] = {

	{
		.name	=	"BOOT_INFO",
		.size	=	0x10000,
		.offset	=	0x10000,
	},
	{
		.name	=	"linux",
		.size	=	0x130000,
		.offset	=	0x20000,
	},
	{
		.name	=	"romfs",
		.size	=	0x2b0000,
		.offset	=	0x150000,
	},
	{
		.name	=	"config",
		.size	=	0x10000,
		.offset	=	0x3F0000,
	},
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

/* Initial serial platform data */

struct plat_serial8250_port nuc745_uart_data[] = {
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

void __init nuc745_uart_clk_enable(void) {

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

static void __init nuc745evb_init_board(void)
{
	platform_device_register_resndata(NULL, "serial8250", PLAT8250_DEV_PLATFORM,
				NULL, 0, nuc745_uart_data, sizeof(nuc745_uart_data));

	platform_device_register_resndata(NULL, "physmap-flash",  -1,
				nuc745_flash_resources, ARRAY_SIZE(nuc745_flash_resources) , 
				&nuc745_flash_data, sizeof(nuc745_flash_data));
	nuc745_uart_clk_enable();
	nuc745_board_init();
}

static void __init nuc745evb_init(void)
{
	nuc745_read_id();
	nuc745_init_clocks();
	nuc745_gpio_init();
	nuc700_mfp_config(ARRAY_AND_SIZE(nuc745_multi_pin_config));

	// Enable USB host mode on the host/device port
	__raw_writel(0x01, REG_USBTXCON);
}

MACHINE_START(NUC745EVB, "NUC745EVB")
	/* Maintainer: Wan ZongShun */
	.boot_params	= 0,
	.init_irq	= nuc700_init_irq,
	.init_machine	= nuc745evb_init_board,
	.init_early	= nuc745evb_init,
	.timer		= &nuc700_timer,
MACHINE_END
