/*
 * arch/arm/mach-w90x900/include/mach/regs-gpio.h
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

#ifndef __ASM_ARCH_REGS_GPIO_H
#define __ASM_ARCH_REGS_GPIO_H

/* GPIO Control Registers */

#define GPIO_BASE 		GPIO_BA
#define GPIO_CFG		(0x00)
#define GPIO_DIR		(0x04)
#define GPIO_OUT		(0x08)
#define GPIO_IN			(0x0C)
#define GROUPINERV		(0x10)
#define GPIO_GPIO(Nb)		(0x00000001 << (Nb))
#define to_nuc700_gpio_chip(c) container_of(c, struct nuc700_gpio_chip, chip)

#define NUC700_GPIO_CHIP(name, base_gpio, nr_gpio)			\
	{								\
		.chip = {						\
			.label		  = name,			\
			.direction_input  = nuc700_dir_input,		\
			.direction_output = nuc700_dir_output,		\
			.get		  = nuc700_gpio_get,		\
			.set		  = nuc700_gpio_set,		\
			.base		  = base_gpio,			\
			.ngpio		  = nr_gpio,			\
		}							\
	}

extern void nuc700_init_gpio_port(void);

#endif /*  __ASM_ARCH_REGS_EBI_H */
