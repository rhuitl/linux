/*
 * arch/arm/mach-w90x900/include/mach/regs-gcr.h
 *
 * Copyright (c) 2010 Nuvoton technology corporation
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

#ifndef __ASM_ARCH_REGS_GCR_H
#define __ASM_ARCH_REGS_GCR_H

/* Global control registers */

#define REG_PDID	GCR_BA
#define REG_ARBCON	(GCR_BA + 0x04)
#define REG_PLLCON0	(GCR_BA + 0x08)
#define REG_CLKSEL	(GCR_BA + 0x0c)
#define REG_PLLCON1	(GCR_BA + 0x10)
#define REG_I2SCKCON	(GCR_BA + 0x14)
#define REG_IRQWAKECON	(GCR_BA + 0x20)
#define REG_IRQWAKEFLG	(GCR_BA + 0x24)
#define REG_PMCON	(GCR_BA + 0x28)
#define REG_USBTXCON	(GCR_BA + 0x30)

#endif /*  __ASM_ARCH_REGS_GCR_H */
