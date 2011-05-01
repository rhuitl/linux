/* linux/drivers/i2c/busses/i2c-nuc700-p0.c
 *
 * Copyright (c) 2011 Nuvoton technology corporation
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This driver based on nuvoton CCwang@nuvoton.com
 * and was modified by Wanzongshun <mcuos.com@gmail.com>.
 */

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/io.h>

#include <mach/i2c.h>

#define I2CCSR		REG_I2CCSR
#define I2CDIVIDER	REG_I2CDIVIDER
#define I2CCMDR		REG_I2CCMDR
#define I2CSWR		REG_I2CSWR
#define I2CRXR		REG_I2CRXR
#define I2CTXR		REG_I2CTXR

/* i2c driver static defination */
static int i2c0_irq;
static struct clk *clk;

enum nuc700_i2c_port0_state {
        STATE_IDLE,
        STATE_START,
        STATE_READ,
        STATE_WRITE,
        STATE_STOP
};

struct nuc700_i2c_port0 {
        spinlock_t		lock;
        wait_queue_head_t	wait;
        struct i2c_msg		*msg;
        unsigned int		msg_num;
        unsigned int		msg_idx;
        unsigned int		msg_ptr;
        enum nuc700_i2c_port0_state	state;
        void __iomem		*regs;
        struct clk			*clk;
        struct device		*dev;
        struct resource		*irq;
        struct resource		*ioarea;
        struct i2c_adapter	adap;
};

/* 
 * default platform data to use if not supplied in the platform_device
*/

static struct nuc700_platform_i2c nuc700_i2c_default_platform = {
        .flags		= 0,
        .slave_addr	= 0x10,
        .bus_freq	= 100,
        .max_freq	= 400,
        .channel	= 0,
        .bus_num	=0,
};

/*
 * get the platform data associated with the given device, or return
 * the default if there is none
*/

static inline struct nuc700_platform_i2c *nuc700_i2c_get_platformdata(struct device *dev) {
        if (dev->platform_data != NULL)
                return (struct nuc700_platform_i2c *)dev->platform_data;

        return &nuc700_i2c_default_platform;
}

/*
 * complete the message and wake up the caller, using the given return code,
 * or zero to mean ok.
*/

static inline void nuc700_i2c_master_complete(struct nuc700_i2c_port0 *i2c, int ret)
{
        dev_dbg(i2c->dev, "master_complete %d\n", ret);

        i2c->msg_ptr = 0;
        i2c->msg = NULL;
        i2c->msg_idx ++;
        i2c->msg_num = 0;
        if (ret)
                i2c->msg_idx = ret;

        wake_up(&i2c->wait);
}

/* irq enable/disable functions */

static inline void nuc700_i2c_disable_irq(struct nuc700_i2c_port0 *i2c)
{
        __raw_writel(0x00, I2CCSR);
}

static inline void nuc700_i2c_enable_irq(struct nuc700_i2c_port0 *i2c)
{
        __raw_writel(0x03, I2CCSR);
}


/*
 * put the start of a message onto the bus
*/

static void nuc700_i2c_message_start(struct nuc700_i2c_port0 *i2c,
                                     struct i2c_msg *msg)
{
        unsigned int addr = (msg->addr & 0x7f) << 1;	/*slave addr*/

        if (msg->flags & I2C_M_RD)
                addr |= 1;

        __raw_writel(addr & 0xff, I2CTXR);	/* send first byte */
        __raw_writel(I2C_CMD_START | I2C_CMD_WRITE, I2CCMDR);
}

static inline void nuc700_i2c_stop(struct nuc700_i2c_port0 *i2c, int ret)
{
        /* stop the transfer */
        i2c->state = STATE_STOP;

        nuc700_i2c_master_complete(i2c, ret);
        nuc700_i2c_disable_irq(i2c);
}

/*
 * process an interrupt and work out what to do
 */

static int i2c_irq_nextbyte(struct nuc700_i2c_port0 *i2c, unsigned long iiccsr)
{
        unsigned char byte;
        int ret = 0;

        if (iiccsr & 0x100) /*transfer in pregress ?*/
                goto out;

        switch (i2c->state) {

        case STATE_START:
                if (i2c->msg->flags & I2C_M_RD)
                        i2c->state = STATE_READ;
                else
                        i2c->state = STATE_WRITE;

                if (i2c->state == STATE_READ)
                        goto prepare_read;


        case STATE_WRITE:
retry_write:
                if (i2c->msg_ptr < i2c->msg->len) {
                        byte = i2c->msg->buf[i2c->msg_ptr++];
                        __raw_writel(byte, I2CTXR);
                        if (i2c->msg_ptr == i2c->msg->len &&  i2c->msg_idx == i2c->msg_num - 1)	/*end data ?*/
                                __raw_writel( I2C_CMD_WRITE | I2C_CMD_STOP, I2CCMDR);
                        else
                                __raw_writel( I2C_CMD_WRITE, I2CCMDR);
                } else if (i2c->msg_idx < i2c->msg_num - 1) { /*restart a new transmittion*/
                        i2c->msg_ptr = 0;
                        i2c->msg_idx ++;
                        i2c->msg++;
                        /* check to see if we need to do another message */
                        if (i2c->msg->flags & I2C_M_NOSTART) {
                                if (i2c->msg->flags & I2C_M_RD) {
                                        /* cannot do this, the controller
                                         * forces us to send a new START
                                         * when we change direction */

                                        nuc700_i2c_stop(i2c, -EINVAL);
                                }
                                goto retry_write;
                        } else {
                                /* send the new start */
                                nuc700_i2c_message_start(i2c, i2c->msg);
                                i2c->state = STATE_START;
                        }

                } else if (i2c->msg->len == 0) {
                        __raw_writel( I2C_CMD_STOP, I2CCMDR);
                        nuc700_i2c_stop(i2c, 0);
                } else
                        nuc700_i2c_stop(i2c, 0);

                break;

        case STATE_READ:
                byte = __raw_readl(I2CRXR) & 0xff; /*skip first read*/
                i2c->msg->buf[i2c->msg_ptr-1] = byte;
prepare_read:
                i2c->msg_ptr ++;
                if (i2c->msg_ptr < i2c->msg->len + 1) {
                        if (i2c->msg_ptr == i2c->msg->len) /*reach the end data*/
                                __raw_writel( I2C_CMD_READ |I2C_CMD_STOP |
						I2C_CMD_NACK, I2CCMDR);
                        else
                                __raw_writel( I2C_CMD_READ, I2CCMDR);
                } else
                        nuc700_i2c_stop(i2c, 0);

                break;

        case STATE_IDLE:
        case STATE_STOP:
		break;
        }
out:
        return ret;
}

/*
 * top level IRQ servicing routine
*/

static irqreturn_t nuc700_i2c_irq(int irqno, void *dev_id)
{
        struct nuc700_i2c_port0 *i2c = dev_id;
        unsigned long status;
                
        status = __raw_readl(I2CCSR);

        /* mark interrupt flag */
        __raw_writel(  status | 0x04, I2CCSR);

	/*deal with arbitration loss*/
        if (status & 0x200) { 
                dev_warn(i2c->dev, "deal with arbitration loss\n");
                goto out;
        }

	/*deal with NACK*/
        if ((status & 0x800) && (i2c->state == STATE_WRITE ||
						i2c->state == STATE_START)) {
                dev_warn(i2c->dev, "deal with nack\n");
                __raw_writel( I2C_CMD_STOP, I2CCMDR);
                i2c->state = STATE_STOP;
                nuc700_i2c_disable_irq(i2c);
                goto out;
        }

        if (i2c->state == STATE_IDLE) {
                dev_warn(i2c->dev, "IRQ: error i2c->state == IDLE\n");
                goto out;
        }

        /* pretty much this leaves us with the fact that we've
         * transmitted or received whatever byte we last sent */

        i2c_irq_nextbyte(i2c, status);

out:
        return IRQ_HANDLED;
}


/*
 * get the i2c bus for a master transaction
*/

static int nuc700_i2c_set_master(struct nuc700_i2c_port0 *i2c)
{
        int timeout = 400;

        while (timeout-- > 0) {
		if ((__raw_readl(I2CSWR) & 0x18) == 0x18 &&  /*SDR and SCR keep high*/
				(__raw_readl(I2CCSR) & 0x0400) == 0) { /*I2C_BUSY is false*/
                        return 0;
                }
                msleep(1);
        }
        dev_dbg(i2c->dev, "timeout\n");

        return -ETIMEDOUT;
}

/*
 * this starts an i2c transfer
*/

static int nuc700_i2c_doxfer(struct nuc700_i2c_port0 *i2c, struct i2c_msg *msgs, int num)
{
        unsigned long timeout;
        int ret;

	/*check bus*/
        ret = nuc700_i2c_set_master(i2c);
        if (ret != 0) {
                dev_err(i2c->dev, "cannot get bus (error %d)\n", ret);
                ret = -EAGAIN;
                goto out;
        }

        spin_lock_irq(&i2c->lock);

        i2c->msg     = msgs;
        i2c->msg_num = num;
        i2c->msg_ptr = 0;
        i2c->msg_idx = 0;
        i2c->state   = STATE_START;

        nuc700_i2c_enable_irq(i2c);
        nuc700_i2c_message_start(i2c, msgs);
        spin_unlock_irq(&i2c->lock);

        timeout = wait_event_timeout(i2c->wait, i2c->msg_num == 0, HZ * 2);

        ret = i2c->msg_idx;

        /* having these next two as dev_err() makes life very
         * noisy when doing an i2cdetect */

        if (timeout == 0) {
                dev_dbg(i2c->dev, "timeout\n");
                ret = -EIO;
        } else if (ret != num) {
                dev_dbg(i2c->dev, "incomplete xfer (%d)\n", ret);
                ret = -EIO;
        }

out:
        return ret;
}

/*
 * first port of call from the i2c bus code when an message needs
 * transferring across the i2c bus.
*/

static int nuc700_i2c_xfer(struct i2c_adapter *adap,
                           struct i2c_msg *msgs, int num)
{
        struct nuc700_i2c_port0 *i2c = (struct nuc700_i2c_port0 *)adap->algo_data;
        int retry;
        int ret;

        for (retry = 0; retry < adap->retries; retry++) {

                ret = nuc700_i2c_doxfer(i2c, msgs, num);

                if (ret != -EAGAIN)
                        return ret;

                dev_dbg(i2c->dev, "Retrying transmission (%d)\n", retry);

        }

        return -EREMOTEIO;
}

/* declare our i2c functionality */

static u32 nuc700_i2c_func(struct i2c_adapter *adap)
{
        return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

/* i2c bus registration info */

static struct i2c_algorithm nuc700_i2c_algorithm = {
        .master_xfer		= nuc700_i2c_xfer,
        .functionality		= nuc700_i2c_func,
};

static struct nuc700_i2c_port0 nuc700_i2c = {
        .lock	= SPIN_LOCK_UNLOCKED,
        .wait	= __WAIT_QUEUE_HEAD_INITIALIZER(nuc700_i2c.wait),
        .adap	= {
                .name			= "nuc700-i2c-p0",
                .owner			= THIS_MODULE,
                .algo			= &nuc700_i2c_algorithm,
                .retries		= 2,
                .class			= I2C_CLASS_HWMON,
        },
};

#define I2C_INPUT_CLOCK		80000 /* 80Mhz */

/* initialise the controller, set the IO lines and frequency */

static int nuc700_i2c_init(struct nuc700_i2c_port0 *i2c)
{
        struct nuc700_platform_i2c *pdata;
        unsigned int reg;

        /* get the plafrom data */
        pdata = nuc700_i2c_get_platformdata(i2c->adap.dev.parent);

        /* set the i2c speed */
        reg = I2C_INPUT_CLOCK /(pdata->bus_freq * 5) - 1;
        __raw_writel(reg & 0xffff, I2CDIVIDER);
        dev_info(i2c->dev, "bus frequency set to %ld KHz\n", pdata->bus_freq);

        return 0;
}

static void nuc700_i2c_free(struct nuc700_i2c_port0 *i2c)
{
        free_irq(i2c0_irq, i2c);
	clk_disable(clk);
	clk_put(clk);
}

/* called by the bus driver when a suitable device is found */

static int nuc700_i2c_probe(struct platform_device *pdev)
{
        struct nuc700_i2c_port0 *i2c = &nuc700_i2c;
        int ret;

        /* setup info block for the i2c core */
        i2c->dev = &pdev->dev;
        i2c->adap.algo_data = i2c;
        i2c->adap.dev.parent = &pdev->dev;
        i2c->adap.retries = 2;
        i2c->adap.class   = I2C_CLASS_HWMON | I2C_CLASS_SPD;

        /* enable i2c clock */
        clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed to get i2c0 clock\n");
		return PTR_ERR(clk);
	}
        clk_enable(clk);

        /* initialise the i2c controller */
        ret = nuc700_i2c_init(i2c);
        if (ret != 0)
                goto out;

	i2c0_irq = platform_get_irq(pdev, 0);
	if (i2c0_irq < 0) {
		dev_err(&pdev->dev, "failed to get i2c0 irq\n");
		return -ENXIO;
	}
        ret = request_irq(i2c0_irq, nuc700_i2c_irq,
			IRQF_DISABLED, pdev->name, i2c);
        if (ret != 0) {
                dev_err(&pdev->dev, "cannot claim IRQ\n");
                goto out;
        }

        i2c->adap.nr = 0;

        ret = i2c_add_numbered_adapter(&i2c->adap);
        if (ret < 0) {
                dev_err(&pdev->dev, "failed to add bus to i2c core\n");
                goto out;
        }

        platform_set_drvdata(pdev, i2c);
        dev_info(&pdev->dev, "Add nuc700 I2C port0 adapter\n");

out:
        if (ret < 0)
                nuc700_i2c_free(i2c);

        return ret;
}

/*
 * called when device is removed from the bus
*/

static int nuc700_i2c_remove(struct platform_device *pdev)
{
        struct nuc700_i2c_port0 *i2c = platform_get_drvdata(pdev);

        if (i2c != NULL) {
                nuc700_i2c_free(i2c);
                platform_set_drvdata(pdev, NULL);
        }
        clk_disable(clk);
	clk_put(clk);
        return 0;
}

/* device driver for platform bus bits */

static struct platform_driver nuc700_i2c_driver = {
        .probe		= nuc700_i2c_probe,
        .remove		= nuc700_i2c_remove,
        .driver		= {
                .owner	= THIS_MODULE,
                .name	= "nuc700-i2c-p0",
        },
};

static int __init i2c_adap_nuc700_init(void)
{
        int ret;

        ret = platform_driver_register(&nuc700_i2c_driver);

        return ret;
}

static void __exit i2c_adap_nuc700_exit(void)
{
        platform_driver_unregister(&nuc700_i2c_driver);
}

module_init(i2c_adap_nuc700_init);
module_exit(i2c_adap_nuc700_exit);

MODULE_AUTHOR("Wan ZongShun <mcuos.com@gmail.com>");
MODULE_DESCRIPTION("nuc700 I2C0 driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:nuc700-i2c-p0");
