/*
 * arch/arm/mach-NUC700/include/mach/nuc700_nand.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation
 * All rights reserved.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#ifndef _NUC700_NAND_H_
#define _NUC700_NAND_H_

struct nuc710_nand_port {
	unsigned int gpio_otheruse;
	unsigned int gpio_checkrb;
};

#endif
