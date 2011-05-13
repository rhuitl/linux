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

#include <mach/map.h>
#include <mach/nuc700_keypad.h>
#include <mach/nuc700_spi.h>
#include <mach/i2c.h>

#include "cpu.h"

/*
 * public device definition between 910 and 920, or 910
 * and 950 or 950 and 960...,their dev platform register
 * should be in specific file such as nuc950, nuc960 c
 * files rather than the public dev.c file here. so the
 * corresponding platform_device definition should not be
 * static.
*/

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
static u64 nuc700_device_ohci_dmamask = 0xffffffffUL;

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

/* SPI device */

static struct nuc700_spi_info nuc700_spiflash_data = {
        .num_cs		= 1,
        .lsb		= 0,
        .txneg		= 1,
        .rxneg		= 0,
        .divider	= 24,
        .sleep		= 0,
        .txnum		= 0,
        .txbitlen	= 8,
        .bus_num	= 0,
};

#define SPIOFFSET		0x200
#define SPIOREG_SIZE		0x100
static struct resource nuc700_spi_resource[] = {
        [0] = {
                .start = NUC700_PA_I2C + SPIOFFSET,
                .end   = NUC700_PA_I2C + SPIOFFSET + SPIOREG_SIZE  - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_SPI,
                .end   = IRQ_SPI,
                .flags = IORESOURCE_IRQ,
        }
};

/* spi device, spi flash info */

static struct mtd_partition nuc700_spi_flash_partitions[] = {
        {
                .name = "SPI flash",
                .size = 0x0400000,
                .offset = 0,
        },
};

static struct flash_platform_data nuc700_spi_flash_data = {
        .name = "m25p80",
        .parts =  nuc700_spi_flash_partitions,
        .nr_parts = ARRAY_SIZE(nuc700_spi_flash_partitions),
        .type = "mx25l3205d",
};

static struct spi_board_info nuc700_spi_board_info[] __initdata = {
        {
                .modalias = "m25p80",
                .max_speed_hz = 20000000,
                .bus_num = 0,
                .chip_select = 0,
                .platform_data = &nuc700_spi_flash_data,
                .mode = SPI_MODE_0,
        },
};

/* I2C */

#define I2C0FFSET	0x100

/* for port 0 */
static struct resource nuc700_i2c_p0_resource[] = {
        [0] = {
                .start = NUC700_PA_I2C,
                .end   = NUC700_PA_I2C + I2C0FFSET - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_I2C0,
                .end   = IRQ_I2C0,
                .flags = IORESOURCE_IRQ,
        }

};

static struct nuc700_platform_i2c nuc700_i2c_p0_platform = {
        .flags		= 0,
        .slave_addr	= 0x10,
        .bus_freq	= 100,
        .max_freq	= 400,
        .channel	= 0,
        .bus_num	=0,
};


/* for port 1 */
static struct resource nuc700_i2c_p1_resource[] = {
        [0] = {
                .start = NUC700_PA_I2C + I2C0FFSET,
                .end   = NUC700_PA_I2C + SPIOFFSET - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_I2C1,
                .end   = IRQ_I2C1,
                .flags = IORESOURCE_IRQ,
        }

};

static struct nuc700_platform_i2c nuc700_i2c_p1_platform = {
        .flags		= 0,
        .slave_addr	= 0x10,
        .bus_freq	= 100,
        .max_freq	= 400,
        .channel	= 1,
        .bus_num	= 1,
};


/* WDT Device */

static struct resource nuc700_wdt_resource[] = {
	[0] = {
		.start = NUC700_PA_TIMER,
		.end   = NUC700_PA_TIMER + NUC700_SZ_TIMER - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_WDT,
		.end   = IRQ_WDT,
		.flags = IORESOURCE_IRQ,
	}
};

/* Audio Device */

static struct resource nuc700_audio_resource[] = {
	[0] = {
		.start = NUC700_PA_AC97I2S,
		.end   = NUC700_PA_AC97I2S + NUC700_SZ_AC97I2S - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_ACTL,
		.end   = IRQ_ACTL,
		.flags = IORESOURCE_IRQ,
	}

};

/* Provide adding specific CPU platform devices API */

void __init nuc700_board_init(void)
{
	struct platform_device * pdev;
	
	pdev = platform_device_register_resndata(NULL, "nuc700-ohci", -1,
				nuc700_ohci_resource, ARRAY_SIZE(nuc700_ohci_resource) , NULL, 0);
	pdev->dev.dma_mask = &nuc700_device_ohci_dmamask;
	pdev->dev.coherent_dma_mask = 0xffffffffUL;
	
	pdev = platform_device_register_resndata(NULL, "nuc700-emc",  -1,
				nuc700_emc_resource, ARRAY_SIZE(nuc700_emc_resource) , NULL, 0);
	pdev->dev.dma_mask = &nuc700_device_emc_dmamask;
	pdev->dev.coherent_dma_mask = 0xffffffffUL;
		
	platform_device_register_resndata(NULL, "nuc700-kpi",  -1,
				nuc700_kpi_resource, ARRAY_SIZE(nuc700_kpi_resource) , 
				&nuc700_keypad_info, sizeof(nuc700_keypad_info));

	platform_device_register_resndata(NULL, "nuc700-spi",  -1,
				nuc700_spi_resource, ARRAY_SIZE(nuc700_spi_resource) , 
				&nuc700_spiflash_data, sizeof(nuc700_spiflash_data));

	platform_device_register_resndata(NULL, "nuc700-wdt",  -1,
				nuc700_wdt_resource, ARRAY_SIZE(nuc700_wdt_resource) , NULL, 0);

	platform_device_register_resndata(NULL, "nuc700-i2c-p0",  -1,
				nuc700_i2c_p0_resource, ARRAY_SIZE(nuc700_i2c_p0_resource), 
				&nuc700_i2c_p0_platform, sizeof(nuc700_i2c_p0_platform));

	platform_device_register_resndata(NULL, "nuc700-i2c-p1",  -1,
				nuc700_i2c_p1_resource, ARRAY_SIZE(nuc700_i2c_p1_resource), 
				&nuc700_i2c_p1_platform, sizeof(nuc700_i2c_p1_platform));

	platform_device_register_resndata(NULL, "nuc700-ac97",  -1,
				nuc700_audio_resource, ARRAY_SIZE(nuc700_audio_resource) , NULL, 0);

	spi_register_board_info(nuc700_spi_board_info,
                                ARRAY_SIZE(nuc700_spi_board_info));
}

