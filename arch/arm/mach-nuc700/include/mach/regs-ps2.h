/*
 * arch/arm/mach-nuc700/include/mach/regs-ps2.h
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

#ifndef __ASM_ARCH_REGS_PS2_H
#define __ASM_ARCH_REGS_PS2_H

/* PS2 Control Registers */

#define REG_PS2CMD		(PS2_BA + 0x00)
#define REG_STS			(PS2_BA + 0x04)
#define REG_SCANCODE		(PS2_BA + 0x08)
#define REG_ASCII		(PS2_BA + 0x0C)

/* REG_PS2CMD */
#define ENCMD			(0x01 << 8)

/* REG_STS */
#define RX_IRQ			(0x01 << 0)
#define TX_IRQ			(0x01 << 4)
#define TX_ERR			(0x01 << 5)

#endif /*  __ASM_ARCH_REGS_PS2_H */
