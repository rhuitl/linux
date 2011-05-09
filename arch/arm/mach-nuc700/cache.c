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

#ifdef CONFIG_CACHE_NUC700
/* open i,d,w cache single */

asmlinkage void  open_i_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | ICAEN), REG_CAHCNF);

}

asmlinkage void  open_d_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | DCAEN), REG_CAHCNF);

}

asmlinkage void  open_w_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | WRBEN), REG_CAHCNF);

}

/* close i,d,w cache single */

asmlinkage void  close_i_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~ICAEN)), REG_CAHCNF);

}

asmlinkage void  close_d_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~DCAEN)), REG_CAHCNF);

}

asmlinkage void  close_w_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~WRBEN)), REG_CAHCNF);

}

/* open flush and close i,d,w cache whole */

asmlinkage void  open_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) | ICAEN |
						DCAEN | WRBEN), REG_CAHCNF);
}

asmlinkage void  close_cache(void){

	__raw_writel((__raw_readl(REG_CAHCNF) & (~(ICAEN |
						DCAEN | WRBEN))), REG_CAHCNF);
}

asmlinkage void  flush_cache(void){

	__raw_writel((__raw_readl(REG_CAHCON) |
				ICAH | DCAH | DRWB | FLHA), REG_CAHCON);
	while(__raw_readl(REG_CAHCON) != 0x0);
}

asmlinkage void  flush_icache(void){

	__raw_writel((__raw_readl(REG_CAHCON) |
						ICAH | FLHA), REG_CAHCON);
	while(__raw_readl(REG_CAHCON) != 0x0);
}

asmlinkage void  flush_dcache(void){

	__raw_writel((__raw_readl(REG_CAHCON) |
					DRWB | DCAH | FLHA), REG_CAHCON);
	while(__raw_readl(REG_CAHCON) != 0x0);
}
#else
/* open i,d,w cache single */

asmlinkage void  open_i_cache(void){
}

asmlinkage void  open_d_cache(void){
}

asmlinkage void  open_w_cache(void){
}

/* close i,d,w cache single */

asmlinkage void  close_i_cache(void){
}

asmlinkage void  close_d_cache(void){
}

asmlinkage void  close_w_cache(void){

}

/* open flush and close i,d,w cache whole */

asmlinkage void  open_cache(void){
}

asmlinkage void  close_cache(void){
}

asmlinkage void  flush_cache(void){
}

asmlinkage void  flush_icache(void){
}

asmlinkage void  flush_dcache(void){
}
#endif

