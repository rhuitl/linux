/*
 * arch/arm/mach-w90x900/include/mach/regs-ebi.h
 *
 * Copyright (c) 2009 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <mach/hardware.h>

#ifndef __ASM_ARCH_REGS_EBI_H
#define __ASM_ARCH_REGS_EBI_H

/* EBI Control Registers */

#define REG_EBICON	(EBI_BA + 0x0000)
#define REG_ROMCON	(EBI_BA + 0x0004)
#define REG_SDRAMBANK	(EBI_BA + 0x0008)
#define REG_EXTIO	(EBI_BA + 0x0018)
#define REG_EXTI1	(EBI_BA + 0x001c)
#define REG_EXTI2	(EBI_BA + 0x0020)
#define REG_EXTI3	(EBI_BA + 0x0024)
#endif /*  __ASM_ARCH_REGS_EBI_H */
