/*
 * Copyright (c) 2011 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/uaccess.h>

#define REG_WTCR		0x1c
#define WTCLK			(0x01 << 10)
#define WTE			(0x01 << 7)	/*wdt enable*/
#define WTIS			(0x03 << 4)
#define WTIF			(0x01 << 3)
#define WTRF			(0x01 << 2)
#define WTRE			(0x01 << 1)
#define WTR			(0x01 << 0)
/*
 * The watchdog time interval can be calculated via following formula:
 * WTIS		real time interval (formula)
 * 0x00		((2^ 14 ) * ((external crystal freq) / 256))seconds
 * 0x01		((2^ 16 ) * ((external crystal freq) / 256))seconds
 * 0x02		((2^ 18 ) * ((external crystal freq) / 256))seconds
 * 0x03		((2^ 20 ) * ((external crystal freq) / 256))seconds
 *
 * The external crystal freq is 15Mhz in the nuc700 evaluation board.
 * So 0x00 = +-0.28 seconds, 0x01 = +-1.12 seconds, 0x02 = +-4.48 seconds,
 * 0x03 = +- 16.92 seconds..
 */
#define WDT_HW_TIMEOUT		0x02
#define WDT_TIMEOUT		(HZ/2)
#define WDT_HEARTBEAT		15

static int heartbeat = WDT_HEARTBEAT;
module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat, "Watchdog heartbeats in seconds. "
	"(default = " __MODULE_STRING(WDT_HEARTBEAT) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started "
	"(default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

struct nuc700_wdt {
	struct resource  *res;
	struct clk	 *wdt_clock;
	struct platform_device *pdev;
	void __iomem	 *wdt_base;
	char		 expect_close;
	struct timer_list timer;
	spinlock_t       wdt_lock;
	unsigned long next_heartbeat;
};

static unsigned long nuc700wdt_busy;
struct nuc700_wdt *nuc700_wdt;

static inline void nuc700_wdt_keepalive(void)
{
	unsigned int val;

	spin_lock(&nuc700_wdt->wdt_lock);

	val = __raw_readl(nuc700_wdt->wdt_base + REG_WTCR);
	val |= (WTR | WTIF);
	__raw_writel(val, nuc700_wdt->wdt_base + REG_WTCR);

	spin_unlock(&nuc700_wdt->wdt_lock);
}

static inline void nuc700_wdt_start(void)
{
	unsigned int val;

	spin_lock(&nuc700_wdt->wdt_lock);

	val = __raw_readl(nuc700_wdt->wdt_base + REG_WTCR);
	val |= (WTRE | WTE | WTR | WTCLK | WTIF);
	val &= ~WTIS;
	val |= (WDT_HW_TIMEOUT << 0x04);
	__raw_writel(val, nuc700_wdt->wdt_base + REG_WTCR);

	spin_unlock(&nuc700_wdt->wdt_lock);

	nuc700_wdt->next_heartbeat = jiffies + heartbeat * HZ;
	mod_timer(&nuc700_wdt->timer, jiffies + WDT_TIMEOUT);
}

static inline void nuc700_wdt_stop(void)
{
	unsigned int val;

	del_timer(&nuc700_wdt->timer);

	spin_lock(&nuc700_wdt->wdt_lock);

	val = __raw_readl(nuc700_wdt->wdt_base + REG_WTCR);
	val &= ~WTE;
	__raw_writel(val, nuc700_wdt->wdt_base + REG_WTCR);

	spin_unlock(&nuc700_wdt->wdt_lock);
}

static inline void nuc700_wdt_ping(void)
{
	nuc700_wdt->next_heartbeat = jiffies + heartbeat * HZ;
}

static int nuc700_wdt_open(struct inode *inode, struct file *file)
{

	if (test_and_set_bit(0, &nuc700wdt_busy))
		return -EBUSY;

	nuc700_wdt_start();

	return nonseekable_open(inode, file);
}

static int nuc700_wdt_close(struct inode *inode, struct file *file)
{
	if (nuc700_wdt->expect_close == 42)
		nuc700_wdt_stop();
	else {
		dev_crit(&nuc700_wdt->pdev->dev,
			"Unexpected close, not stopping watchdog!\n");
		nuc700_wdt_ping();
	}

	nuc700_wdt->expect_close = 0;
	clear_bit(0, &nuc700wdt_busy);
	return 0;
}

static const struct watchdog_info nuc700_wdt_info = {
	.identity	= "nuc700 watchdog",
	.options	= WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING |
						WDIOF_MAGICCLOSE,
};

static long nuc700_wdt_ioctl(struct file *file,
					unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_value;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &nuc700_wdt_info,
				sizeof(nuc700_wdt_info)) ? -EFAULT : 0;
	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, p);

	case WDIOC_KEEPALIVE:
		nuc700_wdt_ping();
		return 0;

	case WDIOC_SETTIMEOUT:
		if (get_user(new_value, p))
			return -EFAULT;

		heartbeat = new_value;
		nuc700_wdt_ping();

		return put_user(new_value, p);
	case WDIOC_GETTIMEOUT:
		return put_user(heartbeat, p);
	default:
		return -ENOTTY;
	}
}

static ssize_t nuc700_wdt_write(struct file *file, const char __user *data,
						size_t len, loff_t *ppos)
{
	if (!len)
		return 0;

	/* Scan for magic character */
	if (!nowayout) {
		size_t i;

		nuc700_wdt->expect_close = 0;

		for (i = 0; i < len; i++) {
			char c;
			if (get_user(c, data + i))
				return -EFAULT;
			if (c == 'V') {
				nuc700_wdt->expect_close = 42;
				break;
			}
		}
	}

	nuc700_wdt_ping();
	return len;
}

static void nuc700_wdt_timer_ping(unsigned long data)
{
	if (time_before(jiffies, nuc700_wdt->next_heartbeat)) {
		nuc700_wdt_keepalive();
		mod_timer(&nuc700_wdt->timer, jiffies + WDT_TIMEOUT);
	} else
		dev_warn(&nuc700_wdt->pdev->dev, "Will reset the machine !\n");
}

static const struct file_operations nuc700wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.unlocked_ioctl	= nuc700_wdt_ioctl,
	.open		= nuc700_wdt_open,
	.release	= nuc700_wdt_close,
	.write		= nuc700_wdt_write,
};

static struct miscdevice nuc700wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &nuc700wdt_fops,
};

static int __devinit nuc700wdt_probe(struct platform_device *pdev)
{
	int ret = 0;

	nuc700_wdt = kzalloc(sizeof(struct nuc700_wdt), GFP_KERNEL);
	if (!nuc700_wdt)
		return -ENOMEM;

	nuc700_wdt->pdev = pdev;

	spin_lock_init(&nuc700_wdt->wdt_lock);

	nuc700_wdt->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (nuc700_wdt->res == NULL) {
		dev_err(&pdev->dev, "no memory resource specified\n");
		ret = -ENOENT;
		goto err_get;
	}

	if (!request_mem_region(nuc700_wdt->res->start,
				resource_size(nuc700_wdt->res), pdev->name)) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto err_get;
	}

	nuc700_wdt->wdt_base = ioremap(nuc700_wdt->res->start,
					resource_size(nuc700_wdt->res));
	if (nuc700_wdt->wdt_base == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() region\n");
		ret = -EINVAL;
		goto err_req;
	}

	nuc700_wdt->wdt_clock = clk_get(&pdev->dev, NULL);
	if (IS_ERR(nuc700_wdt->wdt_clock)) {
		dev_err(&pdev->dev, "failed to find watchdog clock source\n");
		ret = PTR_ERR(nuc700_wdt->wdt_clock);
		goto err_map;
	}

	clk_enable(nuc700_wdt->wdt_clock);

	setup_timer(&nuc700_wdt->timer, nuc700_wdt_timer_ping, 0);

	if (misc_register(&nuc700wdt_miscdev)) {
		dev_err(&pdev->dev, "err register miscdev on minor=%d (%d)\n",
			WATCHDOG_MINOR, ret);
		goto err_clk;
	}

	return 0;

err_clk:
	clk_disable(nuc700_wdt->wdt_clock);
	clk_put(nuc700_wdt->wdt_clock);
err_map:
	iounmap(nuc700_wdt->wdt_base);
err_req:
	release_mem_region(nuc700_wdt->res->start,
					resource_size(nuc700_wdt->res));
err_get:
	kfree(nuc700_wdt);
	return ret;
}

static int __devexit nuc700wdt_remove(struct platform_device *pdev)
{
	misc_deregister(&nuc700wdt_miscdev);

	clk_disable(nuc700_wdt->wdt_clock);
	clk_put(nuc700_wdt->wdt_clock);

	iounmap(nuc700_wdt->wdt_base);

	release_mem_region(nuc700_wdt->res->start,
					resource_size(nuc700_wdt->res));

	kfree(nuc700_wdt);

	return 0;
}

static struct platform_driver nuc700wdt_driver = {
	.probe		= nuc700wdt_probe,
	.remove		= __devexit_p(nuc700wdt_remove),
	.driver		= {
		.name	= "nuc700-wdt",
		.owner	= THIS_MODULE,
	},
};

static int __init nuc700_wdt_init(void)
{
	return platform_driver_register(&nuc700wdt_driver);
}

static void __exit nuc700_wdt_exit(void)
{
	platform_driver_unregister(&nuc700wdt_driver);
}

module_init(nuc700_wdt_init);
module_exit(nuc700_wdt_exit);

MODULE_AUTHOR("Wan ZongShun <mcuos.com@gmail.com>");
MODULE_DESCRIPTION("Watchdog driver for NUC700");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS("platform:nuc700-wdt");
