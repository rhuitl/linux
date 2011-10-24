/*
 * Altera University Program PS2 controller driver
 *
 * Copyright (C) 2011 Wan ZongShun <mcuos.com@gmail.com>
 *
 * Based on sa1111ps2.c, which is:
 * Copyright (C) 2002 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/serio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/sched.h>

#include <mach/regs-ps2.h>

#define DRV_NAME "nuc700-ps2"
static DECLARE_WAIT_QUEUE_HEAD(ps2_wq);
#define nuc700_ps2_read(reg)		__raw_readl(reg)
#define nuc700_ps2_write(val, reg)	__raw_writel((val), (reg))

//#define __PS2_DEBUG__

#ifdef __PS2_DEBUG__
	#define DBG	dev_info
#else
	#define DBG	dev_dbg
#endif

struct ps2if {
	struct serio *io;
	struct resource *iomem_res;
	struct clk *ps2_clk;
	void __iomem *base;
	unsigned irq;
	unsigned int ps2_tx_finished;
};

/*
 * Read all bytes waiting in the PS2 port.
 */
static irqreturn_t nuc700_ps2_rxint(int irq, void *dev_id)
{
	struct ps2if *ps2if = dev_id;
	unsigned int status, scancode;

	status = nuc700_ps2_read(ps2if->base + 0x04);

	if (status & RX_IRQ) {
		nuc700_ps2_write(RX_IRQ, ps2if->base + 0x04);
		scancode = nuc700_ps2_read(ps2if->base + 0x08);

		DBG(ps2if->io->dev.parent, "nuc700_ps2_rxint read scancode = 0x%x!!\n", scancode);
		
		if(scancode & 0x100)	//Extend (0xE0)
			serio_interrupt(ps2if->io, 0xe0, 0);
			
		if(scancode & 0x200)	//release (0xF0)
			serio_interrupt(ps2if->io, 0xf0, 0);
						
		serio_interrupt(ps2if->io, scancode & 0xff, 0);
	}

	if (status & TX_IRQ) {
		nuc700_ps2_write(TX_IRQ, ps2if->base + 0x04);
		ps2if->ps2_tx_finished = 1;

		DBG(ps2if->io->dev.parent, "nuc700_ps2_rxint write irq!!\n");

               	wake_up_interruptible(&ps2_wq);
	}
	if (status & TX_ERR) {
		dev_err(ps2if->io->dev.parent, "NUC700 ps2 TX error!!!\n");
		nuc700_ps2_write(TX_ERR, ps2if->base + 0x04);
	}

	return IRQ_HANDLED;
}

/*
 * Write a byte to the PS2 port.
 */
static int nuc700_write_ps2(struct serio *io, unsigned char val)
{
	struct ps2if *ps2if = io->port_data;
	unsigned int write_val;
	DBG(ps2if->io->dev.parent, "nuc700_write_ps2\n");
	ps2if->ps2_tx_finished  = 0x0;
	write_val = ((unsigned int)val | ENCMD);
	
	writel(write_val, ps2if->base);

	wait_event_interruptible(ps2_wq, (ps2if->ps2_tx_finished  != 0));
	DBG(ps2if->io->dev.parent, "nuc700_write_ps2 finished\n");
	return 0;
}

static int nuc700_ps2_open(struct serio *io)
{
	struct ps2if *ps2if = io->port_data;

	DBG(ps2if->io->dev.parent, "nuc700_ps2_open\n");

	/* clear irq, fifo */
	nuc700_ps2_write(0x0, ps2if->base + 0x04);
	nuc700_ps2_write(0x0, ps2if->base + 0x08);

	clk_enable(ps2if->ps2_clk);
	return 0;
}

static void nuc700_ps2_close(struct serio *io)
{
	struct ps2if *ps2if = io->port_data;

	clk_disable(ps2if->ps2_clk);
}

/*
 * Add one device to this driver.
 */
static int __devinit nuc700_ps2_probe(struct platform_device *pdev)
{
	struct ps2if *ps2if;
	struct serio *serio;
	int error, irq;

	ps2if = kzalloc(sizeof(struct ps2if), GFP_KERNEL);
	serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!ps2if || !serio) {
		error = -ENOMEM;
		goto err_free_mem;
	}

	serio->id.type		= SERIO_8042;
	serio->write		= nuc700_write_ps2;
	serio->open		= nuc700_ps2_open;
	serio->close		= nuc700_ps2_close;
	strlcpy(serio->name, dev_name(&pdev->dev), sizeof(serio->name));
	strlcpy(serio->phys, dev_name(&pdev->dev), sizeof(serio->phys));
	serio->port_data	= ps2if;
	serio->dev.parent	= &pdev->dev;
	ps2if->io		= serio;

	ps2if->iomem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (ps2if->iomem_res == NULL) {
		error = -ENOENT;
		goto err_free_mem;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		error = -ENXIO;
		goto err_free_mem;
	}
	ps2if->irq = irq;

	if (!request_mem_region(ps2if->iomem_res->start,
				resource_size(ps2if->iomem_res), pdev->name)) {
		error = -EBUSY;
		goto err_free_mem;
	}

	ps2if->base = ioremap(ps2if->iomem_res->start,
			      resource_size(ps2if->iomem_res));
	if (!ps2if->base) {
		error = -ENOMEM;
		goto err_free_res;
	}

	/* get Clock */
	ps2if->ps2_clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(ps2if->ps2_clk)) {
		error = -ENODEV;
		dev_err(&pdev->dev, "no ps2if->ps2_clk?\n");
		goto err_unmap;
	}

	error = request_irq(ps2if->irq, nuc700_ps2_rxint, 0, pdev->name, ps2if);
	if (error) {
		dev_err(&pdev->dev, "could not allocate IRQ %d: %d\n",
			ps2if->irq, error);
		goto err_clk;
	}

	dev_info(&pdev->dev, "base %p, irq %d\n", ps2if->base, ps2if->irq);

	serio_register_port(ps2if->io);
	platform_set_drvdata(pdev, ps2if);

	return 0;

 err_clk:
	clk_disable(ps2if->ps2_clk);
	clk_put(ps2if->ps2_clk);
 err_unmap:
	iounmap(ps2if->base);
 err_free_res:
	release_mem_region(ps2if->iomem_res->start,
			   resource_size(ps2if->iomem_res));
 err_free_mem:
	kfree(ps2if);
	kfree(serio);
	return error;
}

/*
 * Remove one device from this driver.
 */
static int __devexit nuc700_ps2_remove(struct platform_device *pdev)
{
	struct ps2if *ps2if = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	serio_unregister_port(ps2if->io);
	free_irq(ps2if->irq, ps2if);
	clk_disable(ps2if->ps2_clk);
	clk_put(ps2if->ps2_clk);
	iounmap(ps2if->base);
	release_mem_region(ps2if->iomem_res->start,
			   resource_size(ps2if->iomem_res));
	kfree(ps2if);

	return 0;
}

/*
 * Our device driver structure
 */
static struct platform_driver nuc700_ps2_driver = {
	.probe		= nuc700_ps2_probe,
	.remove		= __devexit_p(nuc700_ps2_remove),
	.driver	= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init nuc700_ps2_init(void)
{
	return platform_driver_register(&nuc700_ps2_driver);
}

static void __exit nuc700_ps2_exit(void)
{
	platform_driver_unregister(&nuc700_ps2_driver);
}

module_init(nuc700_ps2_init);
module_exit(nuc700_ps2_exit);

MODULE_DESCRIPTION("Nuvoton NUC700 PS2 controller driver");
MODULE_AUTHOR("Wan ZongShun <mcuos.com@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
