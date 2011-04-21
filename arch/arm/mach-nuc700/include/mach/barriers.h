/*
 * Barriers redefined for nuc700 arm7 platforms with a strange cache
 * controller to work around hardware errata causing the outer_sync()
 * operation to deadlock the system, by Wan zongsun mark.
 *
 * arch/arm/mach-nuc700/mach/include/barriers.h
 *
 * Copyright (c) 2011 Nuvoton corporation
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

 #ifdef CONFIG_ARCH_NUC700
 extern asmlinkage void  flush_dcache(void);
 extern asmlinkage void  flush_cache(void);
 
#define mb()		flush_cache()

#define rmb()	do{flush_dcache();	\
				barrier();}while(0)
#define wmb()	do{flush_dcache();	\
				barrier();}while(0)
#endif
