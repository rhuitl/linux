/*
 * linux/arch/arm/mach-w90x900/clock.c
 *
 * Copyright (c) 2008 Nuvoton technology corporation
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/regs-cache.h>

/* open i,d,w cache single */

void open_i_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | ICAEN), REG_CAHCNF);

}

void open_d_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | DCAEN), REG_CAHCNF);

}

void open_w_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | WRBEN), REG_CAHCNF);

}

/* close i,d,w cache single */

void close_i_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~ICAEN)), REG_CAHCNF);

}

void close_d_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~DCAEN)), REG_CAHCNF);

}

void close_w_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~WRBEN)), REG_CAHCNF);

}

/* open flush and close i,d,w cache whole */

void open_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | ICAEN |
						DCAEN | WRBEN), REG_CAHCNF);
}

void close_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~(ICAEN |
						DCAEN | WRBEN))), REG_CAHCNF);
}

void flush_cache(void){

	__raw_writel((__raw_readl(REG_CAHCON) |
				ICAH | DCAH | DRWB | FLHA), REG_CAHCON);
	while(__raw_readl(REG_CAHCON) != 0x0);
}

