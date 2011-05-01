/*linux/include/asm-arm/arch-nuc700/i2c.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
 
 
#ifndef __ASM_ARCH_IIC_H
#define __ASM_ARCH_IIC_H

#include <mach/regs-i2c.h>
 
struct nuc700_platform_i2c {
	unsigned int    flags;
	unsigned int    slave_addr;	/* slave address for controller */
	unsigned long   bus_freq;	/* standard bus frequency */
	unsigned long   max_freq;	/* max frequency for the bus */
	unsigned long   min_freq;	/* min frequency for the bus */
	unsigned int	 channel;	/* i2c channel number */
	unsigned int	 bus_num;	/* i2c bus number */
};

/* bit map in CMDR */
#define I2C_CMD_START		0x10
#define I2C_CMD_STOP		0x08
#define I2C_CMD_READ		0x04
#define I2C_CMD_WRITE		0x02
#define I2C_CMD_NACK		0x01

/* for transfer use */
#define I2C_WRITE		0x00
#define I2C_READ		0x01

#define I2C_STATE_NOP		0x00
#define I2C_STATE_READ		0x01
#define I2C_STATE_WRITE		0x02
#define I2C_STATE_PROBE		0x03
 
#endif //end __ASM_ARCH_IIC_H 
