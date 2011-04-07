/*
 * linux/arch/arm/mach-w90x900/irq.c
 *
 * based on linux/arch/arm/plat-s3c24xx/irq.c by Ben Dooks
 *
 * Copyright (c) 2008 Nuvoton technology corporation
 * All rights reserved.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ptrace.h>
#include <linux/sysdev.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>

#include <mach/regs-irq.h>

/*Cpu IRQ mode and priority*/

#define LOWLEVEL	0
#define HIGHLEVEL	1
#define NEGDE		2
#define PEGDE		3

int nuc700_set_aic_scr(unsigned int mode, unsigned int priority,
							unsigned int irqnum){

	unsigned int val;
	int ret = 0;

	if (priority < 0 || priority >7 || irqnum <= 0) {
		ret = -1;
		goto out;
	}

	val = (mode << 6 | priority);
	__raw_writel(val, (REG_AIC_SCR + (irqnum << 2)));

out:
	return ret;
}

static void nuc700_irq_mask(struct irq_data *d)
{
	__raw_writel(1 << d->irq, REG_AIC_MDCR);
}

/*
 * By the w90p910 spec,any irq,only write 1
 * to REG_AIC_EOSCR for ACK
 */

static void nuc700_irq_ack(struct irq_data *d)
{
	__raw_writel(0x01, REG_AIC_EOSCR);
}

static void nuc700_irq_unmask(struct irq_data *d)
{
	__raw_writel(1 << d->irq, REG_AIC_MECR);
}

static struct irq_chip nuc700_irq_chip = {
	.irq_ack	= nuc700_irq_ack,
	.irq_mask	= nuc700_irq_mask,
	.irq_unmask	= nuc700_irq_unmask,
};

void __init nuc700_init_irq(void)
{
	int irqno;

	__raw_writel(0x7FFFE, REG_AIC_MDCR);

	for (irqno = IRQ_WDT; irqno <= NR_IRQS; irqno++) {
		set_irq_chip(irqno, &nuc700_irq_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

	nuc700_set_aic_scr(HIGHLEVEL, 1, IRQ_TIMER0);
	nuc700_set_aic_scr(HIGHLEVEL, 1, IRQ_UART0);
}
