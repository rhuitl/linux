/*
 * arch/arm/mach-nuc700/include/mach/regs-i2c.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <mach/hardware.h>

#ifndef __ASM_ARCH_REGS_I2C_H
#define __ASM_ARCH_REGS_I2C_H

/* I2C Control Registers */
#define I2C1_BA			I2C_BA + 0x100

/* i2c0 */
#define REG_I2CCSR		(I2C_BA + 0x00)
#define REG_I2CDIVIDER		(I2C_BA + 0x04)
#define REG_I2CCMDR		(I2C_BA + 0x08)
#define REG_I2CSWR		(I2C_BA + 0x0C)
#define REG_I2CRXR		(I2C_BA + 0x10)
#define REG_I2CTXR		(I2C_BA + 0x14)

/* i2c1 */
#define REG_I2CCSR1		(I2C1_BA + 0x00)
#define REG_I2CDIVIDER1		(I2C1_BA + 0x04)
#define REG_I2CCMDR1		(I2C1_BA + 0x08)
#define REG_I2CSWR1		(I2C1_BA + 0x0C)
#define REG_I2CRXR1		(I2C1_BA + 0x10)
#define REG_I2CTXR1		(I2C1_BA + 0x14)

#endif /*  __ASM_ARCH_REGS_I2C_H */
