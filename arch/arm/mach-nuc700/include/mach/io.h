/*
 * arch/arm/mach-w90x900/include/mach/io.h
 *
 * Copyright (c) 2008 Nuvoton technology corporation
 * All rights reserved.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * Based on arch/arm/mach-s3c2410/include/mach/io.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#define IO_SPACE_LIMIT	0xffffffff

/*
 * 1:1 mapping for ioremapped regions.
 */

#define __mem_pci(a)	(a)
#define __io(a)		__typesafe_io(a)

static inline void __iomem *__arch_ioremap(unsigned long cookie, size_t size,
					   unsigned int flags)
{
	return (void __iomem *)(cookie | 0x80000000);
}

#define __arch_ioremap		__arch_ioremap
#define __arch_iounmap(cookie)	do { } while (0)
#endif
