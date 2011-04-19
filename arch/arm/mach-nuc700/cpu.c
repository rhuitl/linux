/*
 * linux/arch/arm/mach-w90x900/cpu.c
 *
 * Copyright (c) 2009 Nuvoton corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * NUC900 series cpu common support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/irq.h>

#include <mach/hardware.h>
#include <mach/regs-serial.h>
#include <mach/regs-ebi.h>
#include <mach/regs-gpio.h>

#include "cpu.h"
#include "clock.h"

/* Initial clock declarations. */

static DEFINE_CLK(uart0, 5);
static DEFINE_CLK(timer, 6);
static DEFINE_CLK(ohci, 7);
static DEFINE_CLK(wdt, 8);
static DEFINE_CLK(emc, 10);
static DEFINE_CLK(lcd, 11);
static DEFINE_CLK(sd, 12);
static DEFINE_CLK(gdma, 13);
static DEFINE_CLK(usbd, 14);
static DEFINE_CLK(usbcks, 15);
static DEFINE_CLK(ac97, 16);
static DEFINE_CLK(pwm, 17);
static DEFINE_CLK(rtc, 18);
static DEFINE_CLK(i2c0, 19);
static DEFINE_CLK(i2c1, 20);
static DEFINE_CLK(uart1, 21);
static DEFINE_CLK(uart2, 22);
static DEFINE_CLK(uart3, 23);
static DEFINE_CLK(usi, 24);
static DEFINE_CLK(sch0, 25);
static DEFINE_CLK(sch1, 26);
static DEFINE_CLK(kpi, 27);
static DEFINE_CLK(ps2, 28);
static DEFINE_CLK(ext, 0);

static struct clk_lookup nuc700_clkregs[] = {
	DEF_CLKLOOK(&clk_uart0, "nuc700-uart0", NULL),
	DEF_CLKLOOK(&clk_timer, NULL, "timer"),
	DEF_CLKLOOK(&clk_ohci, "nuc700-ohci", NULL),
	DEF_CLKLOOK(&clk_wdt, "nuc700-wdt", NULL),
	DEF_CLKLOOK(&clk_emc, "nuc700-emc", NULL),
	DEF_CLKLOOK(&clk_lcd, "nuc700-lcd", NULL),
	DEF_CLKLOOK(&clk_sd, "nuc700-sd", NULL),
	DEF_CLKLOOK(&clk_gdma, "nuc700-gdma", NULL),
	DEF_CLKLOOK(&clk_usbd, "nuc700-usbd", NULL),
	DEF_CLKLOOK(&clk_usbcks, "nuc700-usbcks", NULL),
	DEF_CLKLOOK(&clk_rtc, "nuc700-rtc", NULL),
	DEF_CLKLOOK(&clk_i2c0, "nuc700-i2c0", NULL),
	DEF_CLKLOOK(&clk_i2c1, "nuc700-i2c1", NULL),
	DEF_CLKLOOK(&clk_uart1, "nuc700-uart1", NULL),
	DEF_CLKLOOK(&clk_uart2, "nuc700-uart2", NULL),
	DEF_CLKLOOK(&clk_uart3, "nuc700-uart3", NULL),
	DEF_CLKLOOK(&clk_usi, "nuc700-spi", NULL),
	DEF_CLKLOOK(&clk_sch0, "nuc700-sch0", NULL),
	DEF_CLKLOOK(&clk_sch1, "nuc700-sch1", NULL),
	DEF_CLKLOOK(&clk_kpi, "nuc700-kpi", NULL),
	DEF_CLKLOOK(&clk_ac97, "nuc700-ac97", NULL),
	DEF_CLKLOOK(&clk_pwm, "nuc700-pwm", NULL),
	DEF_CLKLOOK(&clk_ps2, "nuc700-ps2", NULL),
	DEF_CLKLOOK(&clk_ext, NULL, "ext"),
};

/*Init NUC700 evb read id*/

void __init nuc700_read_id(void)
{
	unsigned long idcode = 0x0;

	idcode = __raw_readl(NUC700PDID);
	idcode &= 0xfff;

	if (idcode == NUC710_CPUID)
		printk(KERN_INFO "CPU type 0x%08lx is NUC710\n", idcode);
	else if (idcode == NUC740_CPUID)
		printk(KERN_INFO "CPU type 0x%08lx is NUC740\n", idcode);
	else if (idcode == NUC745_CPUID)
		printk(KERN_INFO "CPU type 0x%08lx is NUC745\n", idcode);
}

/*Init NUC700 clock*/

void __init nuc700_init_clocks(void)
{
	clkdev_add_table(nuc700_clkregs, ARRAY_SIZE(nuc700_clkregs));
}

void __init nuc700_gpio_init(void)
{
	nuc700_init_gpio_port();
}

