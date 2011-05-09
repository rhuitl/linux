/*
 * linux/arch/arm/mach-w90x900/gpio.c
 *
 * Generic nuc700 GPIO handling
 *
 *  Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/gpio.h>

#include <mach/regs-gpio.h>

int nuc700_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct nuc700_gpio_chip *nuc700_gpio = to_nuc700_gpio_chip(chip);
	void __iomem *pio = nuc700_gpio->regbase + GPIO_IN;
	unsigned int regval;

	regval = __raw_readl(pio);
	regval &= GPIO_GPIO(offset);

	return (regval != 0);
}

void nuc700_gpio_set(struct gpio_chip *chip, unsigned offset, int val)
{
	struct nuc700_gpio_chip *nuc700_gpio = to_nuc700_gpio_chip(chip);
	void __iomem *pio = nuc700_gpio->regbase + GPIO_OUT;
	unsigned int regval;
	unsigned long flags;

	spin_lock_irqsave(&nuc700_gpio->gpio_lock, flags);

	regval = __raw_readl(pio);

	if (val)
		regval |= GPIO_GPIO(offset);
	else
		regval &= ~GPIO_GPIO(offset);

	__raw_writel(regval, pio);

	spin_unlock_irqrestore(&nuc700_gpio->gpio_lock, flags);
}

int nuc700_dir_input(struct gpio_chip *chip, unsigned offset)
{
	struct nuc700_gpio_chip *nuc700_gpio = to_nuc700_gpio_chip(chip);
	void __iomem *pio = nuc700_gpio->regbase + GPIO_DIR;
	unsigned int regval;
	unsigned long flags;

	spin_lock_irqsave(&nuc700_gpio->gpio_lock, flags);

	regval = __raw_readl(pio);
	regval &= ~GPIO_GPIO(offset);
	__raw_writel(regval, pio);

	spin_unlock_irqrestore(&nuc700_gpio->gpio_lock, flags);

	return 0;
}

int nuc700_dir_output(struct gpio_chip *chip, unsigned offset, int val)
{
	struct nuc700_gpio_chip *nuc700_gpio = to_nuc700_gpio_chip(chip);
	void __iomem *outreg = nuc700_gpio->regbase + GPIO_OUT;
	void __iomem *pio = nuc700_gpio->regbase + GPIO_DIR;
	unsigned int regval;
	unsigned long flags;

	spin_lock_irqsave(&nuc700_gpio->gpio_lock, flags);

	regval = __raw_readl(pio);
	regval |= GPIO_GPIO(offset);
	__raw_writel(regval, pio);

	regval = __raw_readl(outreg);

	if (val)
		regval |= GPIO_GPIO(offset);
	else
		regval &= ~GPIO_GPIO(offset);

	__raw_writel(regval, outreg);

	spin_unlock_irqrestore(&nuc700_gpio->gpio_lock, flags);

	return 0;
}

void __init nuc700_init_gpio_port(struct nuc700_gpio_chip *gpio_chip, int num)
{
	unsigned i;

	for (i = 0; i < num; i++, gpio_chip++) {
		if(gpio_chip->reserve) continue;
		spin_lock_init(&gpio_chip->gpio_lock);
		gpio_chip->regbase = (unsigned int*)(GPIO_BASE + i * GROUPINERV);
		gpiochip_add(&gpio_chip->chip);
	}
}
