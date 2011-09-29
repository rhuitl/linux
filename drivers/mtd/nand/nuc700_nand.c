/*
 * Copyright © 2011 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/gpio.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include <asm/gpio.h>
#include <mach/nuc700_nand.h>

#define REG_EXTI2	0x0020
#define REG_EXTI3	0x0024
#define NUC_CONFIGEXT	0xE0004491

#define EBI_BASE_ADDR	0xF0000000
#define REG_SMCMD		(EBI_BASE_ADDR + 0x04)
#define REG_SMADDR		(EBI_BASE_ADDR + 0x08)
#define REG_SMDATA		(EBI_BASE_ADDR + 0x0)

#define ENTER()		printk("[%-10s] : Vincent debug Enter\n", __FUNCTION__)
#define LEAVE()		printk("[%-10s] : Vincent debug Leave\n", __FUNCTION__)

#define read_data_reg()		\
	__raw_readb(REG_SMDATA)

#define write_data_reg(val)	\
	__raw_writeb((val), REG_SMDATA)

#define write_cmd_reg(val)		\
	__raw_writeb((val), REG_SMCMD)

#define write_addr_reg(val)	\
	__raw_writeb((val), REG_SMADDR)

struct nuc700_nand {
	struct mtd_info mtd;
	struct nand_chip chip;
	void __iomem *reg;
	spinlock_t lock;
	struct nuc710_nand_port *pdata;
};

static const struct mtd_partition partitions[] = {

	{ 
		.name = "nuc700 512B NAND PAR1",
	  	.offset = 0,
	  	.size = 16*1024*1024 
	},
	{ 
		.name = "nuc700 512B NAND PAR2",
	  	.offset = 16*1024*1024,
	  	.size = 16*1024*1024 
	},
};

static unsigned char nuc700_nand_read_byte(struct mtd_info *mtd)
{
	unsigned char ret;
	struct nuc700_nand *nand;

	nand = container_of(mtd, struct nuc700_nand, mtd);

	ret = (unsigned char)read_data_reg();

	return ret;
}

static void nuc700_nand_read_buf(struct mtd_info *mtd,
				 unsigned char *buf, int len)
{
	int i;
	struct nuc700_nand *nand;

	nand = container_of(mtd, struct nuc700_nand, mtd);

	for (i = 0; i < len; i++)
		buf[i] = (unsigned char)read_data_reg();
}

static void nuc700_nand_write_buf(struct mtd_info *mtd,
				  const unsigned char *buf, int len)
{
	int i;
	struct nuc700_nand *nand;

	nand = container_of(mtd, struct nuc700_nand, mtd);

	for (i = 0; i < len; i++)
		write_data_reg(buf[i]);
}

static int nuc700_verify_buf(struct mtd_info *mtd,
			     const unsigned char *buf, int len)
{
	int i;
	struct nuc700_nand *nand;

	nand = container_of(mtd, struct nuc700_nand, mtd);

	for (i = 0; i < len; i++) {
		if (buf[i] != (unsigned char)read_data_reg())
			return -EFAULT;
	}

	return 0;
}

static int nuc700_check_rb(struct nuc700_nand *nand)
{
	int val;
	spin_lock(&nand->lock);

	val = gpio_get_value(nand->pdata->gpio_checkrb);

	spin_unlock(&nand->lock);

	return val;
}

static void  nuc700_nand_reset(struct nuc700_nand *nand)
{
	int i;

	ENTER();

	write_cmd_reg(0xff);

	for (i=100; i>0; i--);

	while(!nuc700_check_rb(nand));

	LEAVE();
}

static int nuc700_nand_devready(struct mtd_info *mtd)
{
	struct nuc700_nand *nand;
	int ready;
	
	ENTER();
	
	nand = container_of(mtd, struct nuc700_nand, mtd);

	ready = (nuc700_check_rb(nand)) ? 1 : 0;

	LEAVE();

	return ready;
}

static void nuc700_nand_command (struct mtd_info *mtd, unsigned command, 
		int column, int page_addr)
{

	register struct nand_chip *this = mtd->priv;
	struct nuc700_nand *nand;
	nand = container_of(mtd, struct nuc700_nand, mtd);

	if (command == NAND_CMD_SEQIN) {
		int readcmd;

		if (column >= mtd->writesize) {
			/* OOB area */
			column -= mtd->writesize;
			readcmd = NAND_CMD_READOOB;
		} else if (column < 256) {
			/* First 256 bytes --> READ0 */
			readcmd = NAND_CMD_READ0;
		} else {
			column -= 256;
			readcmd = NAND_CMD_READ1;
		}
		write_cmd_reg(readcmd);
	}

	write_cmd_reg(command);

	if (column != -1 || page_addr != -1) {

		/* Serially input address */
		if (column != -1)
			write_addr_reg(column);
		if (page_addr != -1){
			write_addr_reg((unsigned char)(page_addr )& 0xff);	
			write_addr_reg((unsigned char) ((page_addr>>8 )& 0xff));
		}	
			/* One more address cycle for higher density devices */
		if (mtd->size & 0x0c000000) {
			//write_addr_reg((unsigned char) ((page_addr >> 16) & 0x0f));
		}
	}

	switch (command) {
			
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_STATUS:
		break;
	case NAND_CMD_RESET:
		write_cmd_reg(command);

		nuc700_nand_reset(nand);
		break;
	default:
		udelay(this->chip_delay);
		break;
}
	while(!this->dev_ready(mtd));
}


static void nuc700_nand_enable(struct nuc700_nand *nand)
{
	unsigned int ret;
	spin_lock(&nand->lock);

	#ifdef __USE_EBI2__
	__raw_writel(NUC_CONFIGEXT, (nand->reg + REG_EXTI2));
	#else
	__raw_writel(NUC_CONFIGEXT, (nand->reg + REG_EXTI3));
	#endif

	ret = gpio_request(nand->pdata->gpio_checkrb, "checkrb");
	if (ret)
		printk(KERN_WARNING "NUC700 NAND GPIO9 request failed!\n");

	ret = gpio_direction_input(nand->pdata->gpio_checkrb);
	if (ret) {
		gpio_free(nand->pdata->gpio_checkrb);
		printk(KERN_WARNING "NUC700 NAND GPIO9 request failed!\n");
	}
		
	ret = gpio_request(nand->pdata->gpio_otheruse, "otheruse");
	if (ret < 0)
		printk(KERN_WARNING "NUC700 NAND GPIO10 request failed!\n");

	ret = gpio_direction_output(nand->pdata->gpio_otheruse, 1);
	if (ret) {
		gpio_free(nand->pdata->gpio_otheruse);
		printk(KERN_WARNING "NUC700 NAND GPIO9 request failed!\n");
	}	
	spin_unlock(&nand->lock);
}

static int __devinit nuc700_nand_probe(struct platform_device *pdev)
{
	struct nuc700_nand *nuc700_nand;
	struct nand_chip *chip;
	int retval;
	struct resource *res;

	ENTER();
	
	retval = 0;

	nuc700_nand = kzalloc(sizeof(struct nuc700_nand), GFP_KERNEL);
	if (!nuc700_nand)
		return -ENOMEM;
	chip = &(nuc700_nand->chip);

	nuc700_nand->mtd.priv	= chip;
	nuc700_nand->mtd.owner	= THIS_MODULE;
	nuc700_nand->pdata = pdev->dev.platform_data;
	spin_lock_init(&nuc700_nand->lock);

	chip->cmdfunc		= nuc700_nand_command;
	chip->dev_ready		= nuc700_nand_devready;
	chip->read_byte		= nuc700_nand_read_byte;
	chip->write_buf		= nuc700_nand_write_buf;
	chip->read_buf		= nuc700_nand_read_buf;
	chip->verify_buf	= nuc700_verify_buf;
	chip->chip_delay	= 50;
	chip->options		= 0;
	chip->ecc.mode		= NAND_ECC_SOFT;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		retval = -ENXIO;
		goto fail1;
	}

	if (!request_mem_region(res->start, resource_size(res), pdev->name)) {
		retval = -EBUSY;
		goto fail1;
	}

	nuc700_nand->reg = ioremap(res->start, resource_size(res));
	if (!nuc700_nand->reg) {
		retval = -ENOMEM;
		goto fail2;
	}

	nuc700_nand_enable(nuc700_nand);

	if (nand_scan(&(nuc700_nand->mtd), 1)) {
		retval = -ENXIO;
		goto fail3;
	}

	add_mtd_partitions(&(nuc700_nand->mtd), partitions,
						ARRAY_SIZE(partitions));

	platform_set_drvdata(pdev, nuc700_nand);
	
	LEAVE();

	return retval;

fail3:	iounmap(nuc700_nand->reg);
fail2:	release_mem_region(res->start, resource_size(res));
fail1:	kfree(nuc700_nand);
	return retval;
}

static int __devexit nuc700_nand_remove(struct platform_device *pdev)
{
	struct nuc700_nand *nuc700_nand = platform_get_drvdata(pdev);
	struct resource *res;

	iounmap(nuc700_nand->reg);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));

	gpio_free(nuc700_nand->pdata->gpio_otheruse);
	gpio_free(nuc700_nand->pdata->gpio_checkrb);
	
	kfree(nuc700_nand);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver nuc700_nand_driver = {
	.probe		= nuc700_nand_probe,
	.remove		= __devexit_p(nuc700_nand_remove),
	.driver		= {
		.name	= "nuc700-nand",
		.owner	= THIS_MODULE,
	},
};

static int __init nuc700_nand_init(void)
{
	return platform_driver_register(&nuc700_nand_driver);
}

static void __exit nuc700_nand_exit(void)
{
	platform_driver_unregister(&nuc700_nand_driver);
}

module_init(nuc700_nand_init);
module_exit(nuc700_nand_exit);

MODULE_AUTHOR("Wan ZongShun <mcuos.com@gmail.com>");
MODULE_DESCRIPTION("NUC700 nand driver!");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:nuc700-nand");
