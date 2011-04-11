/*
 * linux/arch/arm/mach-w90x900/dev.c
 *
 * Copyright (C) 2009 Nuvoton corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/mtd/physmap.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/mach-types.h>

#include <mach/regs-serial.h>
#include <mach/map.h>
#include <mach/nuc700_keypad.h>

#include "cpu.h"

/*
 * public device definition between 910 and 920, or 910
 * and 950 or 950 and 960...,their dev platform register
 * should be in specific file such as nuc950, nuc960 c
 * files rather than the public dev.c file here. so the
 * corresponding platform_device definition should not be
 * static.
*/

/* RTC controller*/

static struct resource nuc700_rtc_resource[] = {
	[0] = {
		.start = NUC700_PA_RTC,
		.end   = NUC700_PA_RTC + NUC700_SZ_RTC -1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_RTC,
		.end   = IRQ_RTC,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device nuc700_device_rtc = {
	.name		= "nuc700-rtc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(nuc700_rtc_resource),
	.resource	= nuc700_rtc_resource,
};

/* OHCI controller*/

static struct resource nuc700_ohci_resource[] = {
	[0] = {
		.start = NUC700_PA_USBH,
		.end   = NUC700_PA_USBH + NUC700_SZ_USBH -1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_USBH0,
		.end   = IRQ_USBH0,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device nuc700_device_ohci = {
	.name		= "nuc700-ohci",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(nuc700_ohci_resource),
	.resource	= nuc700_ohci_resource,
};

/* MAC device */

static struct resource nuc700_emc_resource[] = {
	[0] = {
		.start = NUC700_PA_EMAC,
		.end   = NUC700_PA_EMAC + NUC700_SZ_EMAC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_EMCTX,
		.end   = IRQ_EMCTX,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_EMCRX,
		.end   = IRQ_EMCRX,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 nuc700_device_emc_dmamask = 0xffffffffUL;
static struct platform_device nuc700_device_emc = {
	.name		= "nuc700-emc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(nuc700_emc_resource),
	.resource	= nuc700_emc_resource,
	.dev              = {
		.dma_mask = &nuc700_device_emc_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

/* KPI controller*/

static int nuc700_keymap[] = {
	KEY(0, 0, KEY_A),
	KEY(0, 1, KEY_B),
	KEY(0, 2, KEY_C),
	KEY(0, 3, KEY_D),

	KEY(1, 0, KEY_E),
	KEY(1, 1, KEY_F),
	KEY(1, 2, KEY_G),
	KEY(1, 3, KEY_H),

	KEY(2, 0, KEY_I),
	KEY(2, 1, KEY_J),
	KEY(2, 2, KEY_K),
	KEY(2, 3, KEY_L),

	KEY(3, 0, KEY_M),
	KEY(3, 1, KEY_N),
	KEY(3, 2, KEY_O),
	KEY(3, 3, KEY_P),
};

static struct matrix_keymap_data nuc700_map_data = {
	.keymap			= nuc700_keymap,
	.keymap_size		= ARRAY_SIZE(nuc700_keymap),
};

struct nuc700_keypad_platform_data nuc700_keypad_info = {
	.keymap_data	= &nuc700_map_data,
	.prescale	= 0xfa,
	.debounce	= 0x50,
};

static struct resource nuc700_kpi_resource[] = {
	[0] = {
		.start = NUC700_PA_KPI,
		.end   = NUC700_PA_KPI + NUC700_SZ_KPI - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_KPI,
		.end   = IRQ_KPI,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device nuc700_device_kpi = {
	.name		= "nuc700-kpi",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(nuc700_kpi_resource),
	.resource	= nuc700_kpi_resource,
	.dev		= {
				.platform_data = &nuc700_keypad_info,
			}
};

/*Here should be your evb resourse,such as LCD*/

static struct platform_device *nuc700_public_dev[] __initdata = {
	&nuc700_serial_device,
	&nuc700_device_ohci,
	&nuc700_device_emc,
	&nuc700_device_kpi,
};

/* Provide adding specific CPU platform devices API */

void __init nuc700_board_init(struct platform_device **device, int size)
{
	platform_add_devices(device, size);
	platform_add_devices(nuc700_public_dev, ARRAY_SIZE(nuc700_public_dev));
}

