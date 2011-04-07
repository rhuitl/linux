/*
 * arch/arm/mach-nuc700/include/mach/regs-timer.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation
 * All rights reserved.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#include <mach/hardware.h>
#ifndef __ASM_ARCH_REGS_TIMER_H
#define __ASM_ARCH_REGS_TIMER_H

/* Cache Registers */

#define REG_CAHCNF	CACHE_BA
#define REG_CAHCON	(CACHE_BA + 0x04)
#define REG_CAHADR	(CACHE_BA + 0x08)

#define ICAEN		0x1
#define DCAEN		0x2
#define WRBEN		0x4

#define ICAH		0x1
#define DCAH		0x2
#define FLHA		0x4
#define DRWB		(0x01 << 7)

extern void open_cache(void);
extern void close_cache(void);
extern void flush_cache(void);

#endif /*  __ASM_ARCH_REGS_TIMER_H */
