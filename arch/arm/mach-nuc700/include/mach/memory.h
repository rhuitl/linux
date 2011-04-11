/*
 * arch/arm/mach-w90x900/include/mach/memory.h
 *
 * Copyright (c) 2008 Nuvoton technology corporation
 * All rights reserved.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * Based on arch/arm/mach-s3c2410/include/mach/memory.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#include <mach/hardware.h>

/*
 * Physical DRAM offset.
 */
#define PHYS_OFFSET	UL(0x00000000)

#ifndef __ASSEMBLY__

#if defined(CONFIG_ARCH_NUC700)
#define MM_SIZE						(0x80000000 - 1)
#define NUC700_MM_V_CACHE_START 	0x00000000
#define NUC700_MM_V_CACHE_END	(NUC700_MM_V_CACHE_START + MM_SIZE)

#define NUC700_MM_P_NOCACHE_START (NUC700_MM_V_CACHE_END + 1)
#define NUC700_MM_P_NOCACHE_END   (NUC700_MM_P_NOCACHE_START + MM_SIZE)

static inline dma_addr_t __virt_to_lbus(unsigned long x)
{
	return (dma_addr_t)(x + NUC700_MM_P_NOCACHE_START);
}

static inline unsigned long __lbus_to_virt(dma_addr_t x)
{
	return (unsigned long)(x - NUC700_MM_P_NOCACHE_START);
}

#define __is_lbus_dma(a)				\
	((a) >= NUC700_MM_P_NOCACHE_START && (a) < NUC700_MM_P_NOCACHE_END)

#define __is_lbus_virt(a)				\
	((a) >= NUC700_MM_V_CACHE_START && (a) < NUC700_MM_V_CACHE_END)

/* Device is an lbus device if it is on the platform bus of the IOP13XX */
#define is_lbus_device(dev) 				\
	(dev && strncmp(dev->bus->name, "platform", 8) == 0)

#define __arch_dma_to_virt(dev, addr)					\
	({								\
		unsigned long __virt;					\
		dma_addr_t __dma = addr;				\
		if (is_lbus_device(dev) && __is_lbus_dma(__dma))	\
			__virt = __lbus_to_virt(__dma);			\
		else							\
			__virt = __phys_to_virt(__dma);			\
		(void*)__virt;						\
	})

#define __arch_virt_to_dma(dev, addr)					\
	({								\
		unsigned long __virt = (unsigned long)addr;		\
		dma_addr_t __dma;					\
		if (is_lbus_device(dev) && __is_lbus_virt(__virt))	\
			__dma = __virt_to_lbus(__virt);			\
		else							\
			__dma = __virt_to_phys(__virt);			\
		__dma;							\
	})

#define __arch_pfn_to_dma(dev, pfn)					\
	({								\
		__arch_virt_to_dma(dev, (unsigned long)__pfn_to_phys(pfn));	\
	})

#define __arch_dma_to_pfn(dev, addr)					\
	({								\
		unsigned long __virt;					\
		__virt = (unsigned long)__arch_dma_to_virt(dev, addr);	\
		(unsigned long)__phys_to_pfn(__virt);				\
	})

#endif /* CONFIG_ARCH_NUC700 */
#endif /* !ASSEMBLY */

#endif
