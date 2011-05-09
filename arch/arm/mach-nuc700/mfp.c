/*
 * linux/arch/arm/mach-nuc700/mfp.c
 *
 * Copyright (c) 2011 Nuvoton corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * NUC700 series cpu common support
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

#include <mach/hardware.h>
#include <mach/regs-gpio.h>
#include <mach/mfp.h>

static DEFINE_SPINLOCK(mfp_spin_lock);

static int default_val[7] = {
	GPIO_CFG0_DEFAULT,
	GPIO_CFG1_DEFAULT,
	GPIO_CFG2_DEFAULT,
	GPIO_CFG3_DEFAULT,
	GPIO_CFG4_DEFAULT,
	GPIO_CFG5_DEFAULT,
	GPIO_CFG6_DEFAULT,
};

static int using_by_boot[7] = {-1,-1,-1,-1,-1,-1,-1};

static int __init check_gpio_default_from_boot(void) {

	int i, j = 0, val;

	for (i = 0; i< 7; i++) {

		val = __raw_readl(GPIO_BASE + i*GROUPINERV);

		if (val != default_val[i]) {
		printk(KERN_WARNING"The GPIO_CFG%d is 0x%x,  not equal to 0x%x, maybe using by boot!\n",i ,val, default_val[i]);
			using_by_boot[j++] = i;
		}

	}
	return j;
}

void __init nuc700_mfp_config(unsigned long *mfp_cfgs, int num)
{
	unsigned long flags;
	int i;

	check_gpio_default_from_boot();

	spin_lock_irqsave(&mfp_spin_lock, flags);

	for (i = 0; i < num; i++, mfp_cfgs++) {
		unsigned long val, c = *mfp_cfgs;

		val = ((GET_GPIO_VAL(c) << GET_GPIO_CFG(c)) |
			( __raw_readl(GPIO_BASE + GET_GPIO_PT(c)) &
					(~(0x03 <<  GET_GPIO_CFG(c)))));

		__raw_writel(val , (GPIO_BASE + GET_GPIO_PT(c)));
	}

	spin_unlock_irqrestore(&mfp_spin_lock, flags);
}
EXPORT_SYMBOL(nuc700_mfp_config);
