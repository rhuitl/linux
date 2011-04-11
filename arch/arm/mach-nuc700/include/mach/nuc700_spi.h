/*
 * arch/arm/mach-nuc700/include/mach/nuc700_spi.h
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

#ifndef __ASM_ARCH_SPI_H
#define __ASM_ARCH_SPI_H

struct nuc700_spi_info {
	unsigned int num_cs;
	unsigned int lsb;
	unsigned int txneg;
	unsigned int rxneg;
	unsigned int divider;
	unsigned int sleep;
	unsigned int txnum;
	unsigned int txbitlen;
	int bus_num;
};

struct nuc900_spi_chip {
	unsigned char bits_per_word;
};

#endif /* __ASM_ARCH_SPI_H */
