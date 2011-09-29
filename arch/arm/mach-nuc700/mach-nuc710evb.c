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
#include <mach/nuc700_sd.h>
#include <mach/nuc700_fb.h>
#include <mach/nuc700_nand.h>

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
#if 1
	/* enable sd */
	GPIO22_SDCD,
	GPIO24_SDDAT3,
	GPIO25_SDDAT2,
	GPIO26_SDDAT1,
	GPIO27_SDDAT0,
	GPIO28_SDCLK,
	GPIO29_SDCMD,
#endif
	/* sd power pin */
	//GPIO23_GPIO23,
#ifdef CONFIG_SERIO_NUC700_PS2
	GPIO9_PS2CLK,
	GPIO10_PS2DATA,	
#endif
#ifdef CONFIG_FB_NUC700
	GPIO30_VCLK,
	GPIO31_VDEN,
	GPIO32_VSYNC,
	GPIO33_HSYNC,
	GPIO34_VD0,
	GPIO35_VD1,
	GPIO36_VD2,
	GPIO37_VD3,
	GPIO38_VD4,
	GPIO39_VD5,
	GPIO40_VD6,
	GPIO41_VD7,
#endif
#ifdef CONFIG_MTD_NAND_NUC700
	GPIO9_GPIO9,
	GPIO10_GPIO10,
#endif
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

/* sd device */
struct nuc700_sd_port nuc710_mmc_port_data = {
	.pwr_pin = 23,
};

static struct resource nuc710_sd_resources[] = {
	[0] = {
		.start = NUC700_PA_SDH,
		.end   = NUC700_PA_SDH + NUC700_SZ_SDH - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SD,
		.end   = IRQ_SD,
		.flags = IORESOURCE_IRQ,
	}
};
static u64 nuc700_device_sd_dmamask = 0xffffffffUL;

/* ps2 device */

static struct resource nuc710_ps2_resources[] = {
	[0] = {
		.start = NUC700_PA_PS2,
		.end   = NUC700_PA_PS2 + NUC700_SZ_PS2 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_PS2,
		.end   = IRQ_PS2,
		.flags = IORESOURCE_IRQ,
	}
};

/* nand device */
#define __GPIO10 10 /* FOR alarm gpio,9 10 have been used */
#define __GPIO9 9

struct nuc710_nand_port nuc710_nand_data = {
	.gpio_otheruse = __GPIO10,
	.gpio_checkrb = __GPIO9,
};

static struct resource nuc710_nand_resources[] = {
	[0] = {
		.start = NUC700_PA_EBI,
		.end   = NUC700_PA_EBI + NUC700_SZ_EBI - 1,
		.flags = IORESOURCE_MEM,
	},
};

/* LCD device */

static struct nuc700fb_display  nuc700_lcd_info[] = {
	/* Giantplus Technology GPM1040A0 320x240 Color TFT LCD */
	[0] = {/* AUO960240 */
		.lcdtype	= "tft",
		.width		= 320,
		.height		= 240,
		.xres		= 320,
		.yres		= 240,
		.bpp		= 16,
		.tfttype	= 1,
		.lcdbus		= 0,
		.rgbseq		= 3,
		.pixelseq	= 2,
	}, [1] = {/* CASIO */
		.lcdtype	= "tft",
		.width		= 480,
		.height		= 240,
		.xres		= 480,
		.yres		= 240,
		.bpp		= 16,
		.tfttype	= 1,
		.lcdbus		= 0,
		.rgbseq		= -1,
		.pixelseq	= 1,
	}, [2] = {/* MTV335 */
		.lcdtype	= "tft",
		.width		= 320,
		.height		= 240,
		.xres		= 320,
		.yres		= 240,
		.bpp		= 16,
		.tfttype	= 0,
		.lcdbus		= -1,
		.rgbseq		= -1,
		.pixelseq	= -1,
	}, [3] = {/* TOPPOLY240320 */
		.lcdtype	= "tft",
		.width		= 240,
		.height		= 320,
		.xres		= 240,
		.yres		= 320,
		.bpp		= 16,
		.tfttype	= 0,
		.lcdbus		= -1,
		.rgbseq		= -1,
		.pixelseq	= -1,
	},
};

struct nuc700fb_platformdata nuc710_lcd_data = {
	.displays		= nuc700_lcd_info,
	.num_displays		= ARRAY_SIZE(nuc700_lcd_info),
#ifdef CONFIG_NUC700_LCD_TFT_AUO960240
	.default_display	= 0,
#endif
#ifdef CONFIG_NUC700_LCD_TFT_CASIO
	.default_display	= 1,
#endif
#ifdef NUC700_LCD_TFT_MTV335
	.default_display	= 2,
#endif
#ifdef NUC700_LCD_TFT_TOPPOLY240320
	.default_display	= 3,
#endif
};

static struct resource nuc710_lcd_resources[] = {
	[0] = {
		.start = NUC700_PA_LCD,
		.end   = NUC700_PA_LCD + NUC700_SZ_LCD - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD,
		.end   = IRQ_LCD,
		.flags = IORESOURCE_IRQ,
	}
};
static u64 nuc700_device_lcd_dmamask = 0xffffffffUL;

/* Initial serial platform data */

struct plat_serial8250_port nuc710_uart0_data[] = {
	NUC700_8250PORT(UART0),
	{},
};
#ifdef CONFIG_NUC700_UART1
struct plat_serial8250_port nuc710_uart1_data[] = {
	NUC700_8250PORT(UART1),
	{},
};
#endif
#ifdef CONFIG_NUC700_UART2
struct plat_serial8250_port nuc710_uart2_data[] = {
	NUC700_8250PORT(UART2),
	{},
};
#endif
#ifdef CONFIG_NUC700_UART3
struct plat_serial8250_port nuc710_uart3_data[] = {
	NUC700_8250PORT(UART3),
	{},
};
#endif
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
	struct platform_device * pdev;

	/* uart 0 1 2 3 register device */
	platform_device_register_resndata(NULL, "serial8250", PLAT8250_DEV_PLATFORM,
				NULL, 0, nuc710_uart0_data, sizeof(nuc710_uart0_data));
	#ifdef CONFIG_NUC700_UART1
		platform_device_register_resndata(NULL, "serial8250", PLAT8250_DEV_PLATFORM1,
				NULL, 0, nuc710_uart1_data, sizeof(nuc710_uart1_data));
	#endif
		#ifdef CONFIG_NUC700_UART2
			platform_device_register_resndata(NULL, "serial8250", PLAT8250_DEV_PLATFORM2,
				NULL, 0, nuc710_uart2_data, sizeof(nuc710_uart2_data));
		#endif
			#ifdef CONFIG_NUC700_UART3
				platform_device_register_resndata(NULL, "serial8250", PLAT8250_DEV_FOURPORT,
				NULL, 0, nuc710_uart3_data, sizeof(nuc710_uart3_data));
			#endif

	/* Nor flash register device */
	platform_device_register_resndata(NULL, "physmap-flash",  -1,
				nuc710_flash_resources, ARRAY_SIZE(nuc710_flash_resources) , 
				&nuc710_flash_data, sizeof(nuc710_flash_data));

	/* sd register device */
	pdev = platform_device_register_resndata(NULL, "nuc700-sd",  -1,
				nuc710_sd_resources, ARRAY_SIZE(nuc710_sd_resources) , 
				&nuc710_mmc_port_data, sizeof(nuc710_mmc_port_data));
	pdev->dev.dma_mask = &nuc700_device_sd_dmamask;
	pdev->dev.coherent_dma_mask = 0xffffffffUL;

	/* ps2 register device */
	platform_device_register_resndata(NULL, "nuc700-ps2",  -1,
				nuc710_ps2_resources, ARRAY_SIZE(nuc710_ps2_resources), NULL, 0);

	/* nand register device */
	platform_device_register_resndata(NULL, "nuc700-nand",  -1,
				nuc710_nand_resources, ARRAY_SIZE(nuc710_nand_resources), 
				&nuc710_nand_data, sizeof(nuc710_nand_data));
	 
	/* lcd register device */
	pdev = platform_device_register_resndata(NULL, "nuc700-lcd",  -1,
				nuc710_lcd_resources, ARRAY_SIZE(nuc710_lcd_resources) , 
				&nuc710_lcd_data, sizeof(nuc710_lcd_data));
	pdev->dev.dma_mask = &nuc700_device_lcd_dmamask;
	pdev->dev.coherent_dma_mask = 0xffffffffUL;

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
