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
#include <linux/gpio.h>

#ifndef __ASM_ARCH_REGS_GPIO_H
#define __ASM_ARCH_REGS_GPIO_H

struct nuc700_gpio_chip {
	struct gpio_chip	chip;
	void __iomem		*regbase;	/* Base of group register*/
	spinlock_t 		gpio_lock;
	unsigned int 		reserve;
};

/* GPIO Control Registers */

#define GPIO_BASE 		GPIO_BA
#define GPIO_CFG		(0x00)
#define GPIO_DIR		(0x04)
#define GPIO_OUT		(0x08)
#define GPIO_IN			(0x0C)
#define GROUPINERV		(0x10)
#define GPIO_GPIO(Nb)		(0x00000001 << (Nb))
#define to_nuc700_gpio_chip(c) container_of(c, struct nuc700_gpio_chip, chip)

#define NUC700_GPIO_CHIP(name, base_gpio, nr_gpio, _reserve)		\
	{								\
		.chip = {						\
			.label		  = name,			\
			.direction_input  = nuc700_dir_input,		\
			.direction_output = nuc700_dir_output,		\
			.get		  = nuc700_gpio_get,		\
			.set		  = nuc700_gpio_set,		\
			.base		  = base_gpio,			\
			.ngpio		  = nr_gpio,			\
		},							\
		.reserve = _reserve,					\
	}
extern void nuc700_init_gpio_port(struct nuc700_gpio_chip *gpio_chip, int num);
extern void nuc700_gpio_init(struct nuc700_gpio_chip *gpio_chip, int num);
extern int nuc700_gpio_get(struct gpio_chip *chip, unsigned offset);
extern void nuc700_gpio_set(struct gpio_chip *chip, unsigned offset, int val);
extern int nuc700_dir_input(struct gpio_chip *chip, unsigned offset);
extern int nuc700_dir_output(struct gpio_chip *chip, unsigned offset, int val);

#endif /*  __ASM_ARCH_REGS_EBI_H */
