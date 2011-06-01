/*
 * Copyright (c) 2011 Nuvoton technology corporation.
 *
 * The first version created by HPchen from Nuvoton.
 *
 * Author: Wan ZongShun <mcuos.com@gmail.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/atmel_pdc.h>
#include <linux/gfp.h>
#include <linux/highmem.h>

#include <linux/mmc/host.h>
#include <linux/mmc/sdio.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/gpio.h>

#include <mach/map.h>
#include <mach/regs-sd.h>
#include <mach/nuc700_sd.h>

#define DRIVER_NAME			"nuc700-sd"

#define FL_SENT_COMMAND			(1 << 0)
#define FL_SENT_STOP			(1 << 1)

#define nuc700_sd_read(reg)		__raw_readl(reg)
#define nuc700_sd_write(reg, val)	__raw_writel((val), (reg))

#define MCI_BLKSIZE			512
#define MCI_MAXBLKSIZE			4095
#define MCI_BLKATONCE			256
#define MCI_BUFSIZE			(MCI_BLKSIZE * MCI_BLKATONCE)

/* Driver thread command */
#define SD_EVENT_NONE			0x00000000
#define SD_EVENT_CMD_OUT		0x00000001
#define SD_EVENT_RSP_IN			0x00000010
#define SD_EVENT_RSP2_IN		0x00000100

static int sd_event=0, sd_state=0, sd_state_xfer=0, sd_send_cmd=0;

static DECLARE_WAIT_QUEUE_HEAD(sd_event_wq);
static DECLARE_WAIT_QUEUE_HEAD(sd_wq);
static DECLARE_WAIT_QUEUE_HEAD(sd_wq_xfer);

/*
 * Low level type for this driver
 */

struct nuc700_sd_host {
	struct mmc_host *mmc;
	struct mmc_command *cmd;
	struct mmc_request *request;
	struct clk *sd_clk;
	struct timer_list timer;
	unsigned int dest_buf_index;
	unsigned int src_buf_index;
	unsigned int dmabusy;
	unsigned int sdbusy;
	unsigned int pwr_pin;
	unsigned int blksz;
	unsigned int blocks;
	unsigned int flags;
	unsigned int dat0_bus_busy;
	unsigned int* buffer;
	int src_buf_count;
	int dest_buf_count;
	int irq;
	int present;

	dma_addr_t physical_address;
};

/* get data from sdcard to internal buffer */
static unsigned long get_data_from_sdcard_to_inbuffer(struct nuc700_sd_host *host)
{
	unsigned long flags, val;

	local_irq_save(flags);

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 4));

	if (host->src_buf_index == 0x0)
		val |= SD_WRITE_BUF0;
	else if (host->src_buf_index == 0x1)
		val |= SD_WRITE_BUF1;
	local_irq_restore(flags);

	return val;
}

/* put data from internal buffer to sdcard */
static unsigned long put_data_from_inbuffer_to_sdcard(struct nuc700_sd_host *host)
{
	unsigned long flags, val;

	local_irq_save(flags);

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 8));

	if (host->src_buf_index == 0x0)
		val |= SD_READ_BUF0;
	else if (host->src_buf_index == 0x1)
		val |= SD_READ_BUF1;
	local_irq_restore(flags);

	return val;
}

/* get data from internal buffer to dmabuff */
static unsigned long get_data_from_inbuffer_to_dmabuf(struct nuc700_sd_host *host)
{
	unsigned long flags, val;

	local_irq_save(flags);

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 8));

	if (host->dest_buf_index == 0x0)
		val |= DMA_READ_BUF0;
	else if (host->dest_buf_index == 0x1)
		val |= DMA_READ_BUF1;
	val |= EN_DMA_READ_BUF;

	local_irq_restore(flags);

	return val;
}

/* put data from dmabuff to internal buffer*/
static unsigned long put_data_from_dmabuf_to_inbuffer(struct nuc700_sd_host *host)
{
	unsigned long flags, val;

	local_irq_save(flags);

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 4));

	if (host->dest_buf_index == 0x0)
		val |= DMA_WRITE_BUF0;
	else if (host->dest_buf_index == 0x1)
		val |= DMA_WRITE_BUF1;
	val |= EN_DMA_WRITE_BUF;

	local_irq_restore(flags);

	return val;
}

static void sd_start_op(struct nuc700_sd_host *host, 
				int sd, int sdmode, int dma, int dmamode)
{
	dma_addr_t current_dma_address;
	unsigned long val, enable_data_tr;
	
	if (dma) {

		if(dmamode == READ) {	/* read */
			val = get_data_from_inbuffer_to_dmabuf(host);
			enable_data_tr = DI_EN;
		} else {	/* write */
			val = put_data_from_dmabuf_to_inbuffer(host);
			enable_data_tr = DO_EN;
		}

		current_dma_address = host->physical_address + host->dest_buf_count * host->blksz;

		host->dest_buf_index ^= 1;
		host->dest_buf_count++;

		host->dmabusy = 0x01;
		
		nuc700_sd_write(REG_SDDSA, current_dma_address);
		nuc700_sd_write(REG_SDBCR, host->blksz);

		nuc700_sd_write(REG_SDGCR, val);
		nuc700_sd_write(REG_SDICR, nuc700_sd_read(REG_SDICR) | enable_data_tr);
	}

	if (sd) {
		if (sdmode == READ)	/* read */
			val = get_data_from_sdcard_to_inbuffer(host);
		else	/* write */
			val = put_data_from_inbuffer_to_sdcard(host);

		host->src_buf_index ^= 1;
		host->src_buf_count--;

		host->sdbusy = 0x01;

		nuc700_sd_write(REG_SDBLEN,host->blksz - 1);
		nuc700_sd_write(REG_SDGCR, val);
	}
}

static void sd_start_sd(struct nuc700_sd_host *host, int mode)
{
	sd_start_op(host, 1, mode, 0,0);
}

static void sd_start_dma(struct nuc700_sd_host *host, int mode)
{
	sd_start_op(host, 0,0, 1, mode);
}

static int nuc700_sd_card_detect(struct mmc_host *mmc)
{
	struct nuc700_sd_host *host = mmc_priv(mmc);
	int ret;

	host->present = nuc700_sd_read(REG_SDIISR) & (SD_CD);
	ret = host->present ? 0 : 1;
	return ret;
}

static void nuc700_sd_timeout_timer(unsigned long data)
{
	struct nuc700_sd_host *host = (void*)data;

	nuc700_sd_card_detect(host->mmc);
}

/*
 * Copy from sg to a dma block - used for transfers
 */
static inline void nuc700_sd_sg_to_dma(struct nuc700_sd_host *host,
							struct mmc_data *data)
{
	unsigned int len, i, size;
	unsigned *dmabuf = host->buffer;

	size = data->blksz * data->blocks;
	len = data->sg_len;

	/*
	 * Just loop through all entries. Size might not
	 * be the entire list though so make sure that
	 * we do not transfer too much.
	 */
	for (i = 0; i < len; i++) {
		struct scatterlist *sg;
		int amount;
		unsigned int *sgbuffer;

		sg = &data->sg[i];

		sgbuffer = kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
		amount = min(size, sg->length);
		size -= amount;

		{
			char *tmpv = (char *)dmabuf;
			memcpy(tmpv, sgbuffer, amount);
			tmpv += amount;
			dmabuf = (unsigned *)tmpv;
		}

		kunmap_atomic(sgbuffer, KM_BIO_SRC_IRQ);
		data->bytes_xfered += amount;

		if (size == 0)
			break;
	}

	/*
	 * Check that we didn't get a request to transfer
	 * more data than can fit into the SG list.
	 */
	BUG_ON(size != 0);
}

/*
 * Handle after a dma read
 */
static int nuc700_sd_post_dma_read(struct nuc700_sd_host *host)
{
	struct mmc_command *cmd;
	struct mmc_data *data;
	unsigned int len, i, size;
	unsigned *dmabuf = host->buffer;


	cmd = host->cmd;
	if (!cmd) {
		dev_warn(host->mmc->parent, "nuc700 dma read, no command\n");
		goto out;
	}

	data = cmd->data;
	if (!data) {
		dev_warn(host->mmc->parent, "nuc700 dma read, no data\n");
		goto out;
	}

	size = data->blksz * data->blocks;
	len = data->sg_len;

	for (i = 0; i < len; i++) {
		struct scatterlist *sg;
		int amount;
		unsigned int *sgbuffer;

		sg = &data->sg[i];

		sgbuffer = kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
		amount = min(size, sg->length);
		size -= amount;

		{
			char *tmpv = (char *)dmabuf;
			memcpy(sgbuffer, tmpv, amount);
			tmpv += amount;
			dmabuf = (unsigned *)tmpv;
		}

		flush_kernel_dcache_page(sg_page(sg));
		kunmap_atomic(sgbuffer, KM_BIO_SRC_IRQ);
		data->bytes_xfered += amount;
		if (size == 0)
			break;
	}
out:
	return 0;
}

/*
 * Handle transmitted data
 */
static void nuc700_sd_handle_transmitted(struct nuc700_sd_host *host)
{
}

/*
 * Update bytes tranfered count during a write operation
 */
static void nuc700_sd_update_bytes_xfered(struct nuc700_sd_host *host)
{
	struct mmc_data *data;

	/* always deal with the effective request (and not the current cmd) */

	if (host->request->cmd && host->request->cmd->error != 0)
		return;

	if (host->request->data) {
		data = host->request->data;
		if (data->flags & MMC_DATA_WRITE) {
			/* card is in IDLE mode now */
			data->bytes_xfered = data->blksz * data->blocks;
		}
	}
}

/*
 * Send a command
 */
static void nuc700_sd_send_command(struct nuc700_sd_host *host,
						struct mmc_command *cmd)
{
	struct mmc_data *data = cmd->data;
	unsigned int csr;

	host->cmd = cmd;
	sd_state = 0;
	sd_state_xfer = 0;

	csr = ((nuc700_sd_read(REG_SDICR) | CLK_KEEP) & (~SD_CS));

	/* set command code and enable command out */
	csr = csr | (cmd->opcode << 8) | CO_EN;
	sd_event |= SD_EVENT_CMD_OUT;

	if (mmc_resp_type(cmd) != MMC_RSP_NONE) {
		/* set 136 bit response for R2, 48 bit response otherwise */
		if (mmc_resp_type(cmd) == MMC_RSP_R2) {
			csr |= R2_EN;
			sd_event |= SD_EVENT_RSP2_IN;
		} else {
			csr |= RI_EN;
			sd_event |= SD_EVENT_RSP_IN;
		}
	}

	if (data) {
		host->blksz = data->blksz;
		host->blocks = data->blocks;
		data->bytes_xfered = 0x0;
		host->dest_buf_index = 0x0;
		host->src_buf_index = 0x0;
		host->src_buf_count = host->blocks;
		host->dest_buf_count = 0x0;

		if (data->flags & MMC_DATA_WRITE) {
			/*
			 * Handle a write
			 */
			nuc700_sd_sg_to_dma(host, data);

			nuc700_sd_write(REG_SDDSA, host->physical_address);
			nuc700_sd_write(REG_SDBCR, host->blksz);
		}

	} else {
		host->blksz = 0;
		host->blocks = 0;
	}

	/*
	 * Set the arguments and send the command
	 */
	nuc700_sd_write(REG_SDARG, cmd->arg);
	nuc700_sd_write(REG_SDICR, csr);
	sd_send_cmd = 1;
	wake_up_interruptible(&sd_event_wq);

	if (!(host->flags & FL_SENT_STOP))
		wait_event_interruptible(sd_wq, (sd_state != 0));

	if (data) {
		if (data->flags & MMC_DATA_WRITE)
			nuc700_sd_update_bytes_xfered(host);
	}

	mmc_request_done(host->mmc, host->request);
}

/*
 * Process the request
 */
static void nuc700_sd_send_request(struct nuc700_sd_host *host)
{

	if (!(host->flags & FL_SENT_COMMAND)) {
		host->flags |= FL_SENT_COMMAND;
		nuc700_sd_send_command(host, host->request->cmd);
	} else if ((!(host->flags & FL_SENT_STOP)) && host->request->stop) {
		host->flags |= FL_SENT_STOP;
		nuc700_sd_send_command(host, host->request->stop);
	} else {
		sd_state = 1;
		wake_up_interruptible(&sd_wq);
		del_timer(&host->timer);
	}
}

/*
 * Handle a command that has been completed
 */
static void nuc700_sd_completed_command(struct nuc700_sd_host *host,
							unsigned int status)
{
	struct mmc_command *cmd = host->cmd;
	struct mmc_data *data = cmd->data;
	unsigned int i, j, tmp[5], err;
	unsigned char *ptr;

	err = nuc700_sd_read(REG_SDIISR);

	if (status & SD_EVENT_RSP_IN) {
		/* if not R2 */
		cmd->resp[0] = (nuc700_sd_read(REG_SDRSP0) << 8) |
					(nuc700_sd_read(REG_SDRSP1) & 0xff);
		cmd->resp[1] = cmd->resp[2] = cmd->resp[3] = 0;
	} else if (status & SD_EVENT_RSP2_IN) {
		/* if R2 */
		ptr = (unsigned char *)FB0_BASE_ADDR;
		for (i=0, j=0; j<5; i+=4, j++)
			tmp[j] = (*(ptr+i)<<24)|(*(ptr+i+1)<<16) |
						(*(ptr+i+2)<<8)|(*(ptr+i+3));
		for (i=0; i<4; i++)
			cmd->resp[i] = ((tmp[i] & 0x00ffffff)<<8) |
						((tmp[i+1] & 0xff000000)>>24);
	}

	if ((err & CRC_7) == 0) {
		if (!(mmc_resp_type(cmd) & MMC_RSP_CRC)) {
			cmd->error = 0;
			nuc700_sd_write(REG_SDIISR, CRC_7);
		} else {
			cmd->error = -EIO;
		}
	} else if ((err & R2_CRC_7) == 0) {
			cmd->error = -EIO;
	} else {
		cmd->error = 0;
	}

	if (data) {
		data->bytes_xfered = 0;

		if (data->flags & MMC_DATA_READ) {
			sd_start_sd(host, READ);
		} else if (data->flags & MMC_DATA_WRITE) {
			sd_start_dma(host, WRITE);
		}

		wait_event_interruptible(sd_wq_xfer, (sd_state_xfer != 0));
	}

	nuc700_sd_send_request(host);
}

/*
 * Handle an MMC request
 */
static void nuc700_sd_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct nuc700_sd_host *host = mmc_priv(mmc);
	host->request = mrq;
	host->flags = 0;

	if (nuc700_sd_card_detect(mmc) == 0) {
		host->request->cmd->error = -ENOMEDIUM;
		mmc_request_done(host->mmc, host->request);
	} else
		nuc700_sd_send_request(host);
}

/*
 * Set the IOS
 */

static void nuc700_sd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	int clkdiv;
	struct nuc700_sd_host *host = mmc_priv(mmc);
	unsigned long nuc700_sd_master_clock = 80000;

	/* maybe switch power to the card */
	switch (ios->power_mode) {
	case MMC_POWER_OFF:
		gpio_direction_output(host->pwr_pin, 1);
		break;
	case MMC_POWER_UP:
		gpio_direction_output(host->pwr_pin, 0);
		nuc700_sd_write(REG_SDICR, nuc700_sd_read(REG_SDICR) | CLK74_OE);

		while (nuc700_sd_read(REG_SDICR) & CLK74_OE);
		break;
	case MMC_POWER_ON:
		break;
	default:
		WARN_ON(1);
	}

	if (ios->clock == 0) {
		/* Disable the sd controller */
		nuc700_sd_write(REG_SDGCR, (nuc700_sd_read(REG_SDGCR) & ~EN_SDEN));
		clkdiv = 0;
	} else {
		/* Enable the sd controller */
		nuc700_sd_write(REG_SDGCR, EN_SDEN);

		if ((nuc700_sd_master_clock % ios->clock) == 0)
			clkdiv = (nuc700_sd_master_clock / ios->clock) - 1;
		else
			clkdiv = nuc700_sd_master_clock / ios->clock;
	}
	if (ios->bus_width == MMC_BUS_WIDTH_4)
		nuc700_sd_write(REG_SDHIIR, clkdiv | SD_BUSW4B);
	else
		nuc700_sd_write(REG_SDHIIR, clkdiv | (~SD_BUSW4B) | SD_BUSW1B);
}

/*
 * Handle CO, RI, and R2 event
 */
static int sd_event_thread(void *_host)
{
	struct nuc700_sd_host *host = _host;

	int event = 0;
	int completed = 0;

	daemonize("sdioeventd");

	for (;;) {
		wait_event_interruptible(sd_event_wq,
				(sd_event != SD_EVENT_NONE) && (sd_send_cmd));

		completed = 0;
		event = sd_event;
		sd_event = SD_EVENT_NONE;
		sd_send_cmd = 0;
		if (event & SD_EVENT_CMD_OUT) {
			while (1) {
				if (!(nuc700_sd_read(REG_SDICR) & CO_EN)) {
					completed = 1;
					break;
				}
			}
		}

		if (event & SD_EVENT_RSP_IN) {
			while (1) {
				if (!(nuc700_sd_read(REG_SDICR) & RI_EN)) {
					completed = 1;
					break;
				}
			}
		}

		if (event & SD_EVENT_RSP2_IN) {
			while (1) {
				if (!(nuc700_sd_read(REG_SDICR) & R2_EN)) {
					completed = 1;
					break;
				}
			}
		}

		if (completed)
			nuc700_sd_completed_command(host, event);
	}
	return 0;
}

static void transferdone(struct nuc700_sd_host *host) {

	if (host->cmd->data->flags & MMC_DATA_WRITE) {
		nuc700_sd_handle_transmitted(host);
	} else if (host->cmd->data->flags & MMC_DATA_READ) {
		nuc700_sd_post_dma_read(host);
	}
	sd_state_xfer = 1;

	host->blksz = 0x0;
	host->dest_buf_count = 0x0;
	host->dest_buf_index = 0x0;
	host->src_buf_index = 0x0;
	host->sdbusy = 0x0;
	host->src_buf_count = 0x0;
	host->blocks = 0x0;
	host->dmabusy = 0x0;

	wake_up_interruptible(&sd_wq_xfer);
}

static unsigned int check_dat0_busy(void)
{
	return (nuc700_sd_read(REG_SDIISR) & DAT0_VAL)? 0:1;
}

static void sd_enable_bus_monitor(void)
{
	nuc700_sd_write(REG_SDIIER, nuc700_sd_read(REG_SDIIER) | EN_DAT0EN);
}

static void sd_disable_bus_monitor(void)
{
	nuc700_sd_write(REG_SDIIER, nuc700_sd_read(REG_SDIIER) & (~EN_DAT0EN));
}

static void sd_dma_interrupt(struct nuc700_sd_host *host, int mode)
{

	host->dmabusy = 0x0;

	if (mode == READ) {
		if (host->src_buf_count <= 0x0) {
			if (host->blocks >= host->dest_buf_count)
				transferdone(host);
		} else {
			if (!host->sdbusy)
				sd_start_sd(host, READ);
		}
	}

	if (mode == WRITE) {
		if (!host->sdbusy) {
			if (check_dat0_busy()) {
				host->dat0_bus_busy = 0x01;
				sd_enable_bus_monitor();
				
			} else {
				sd_start_sd(host, WRITE);
			}
		}
	}
}
static void sd_host_interrupt(struct nuc700_sd_host *host)
{
	unsigned int sd_int_status;

	host->sdbusy = 0x0;

	sd_int_status = nuc700_sd_read(REG_SDIISR);
	/* read sd card to internal buffer finished interrupt */
	if (sd_int_status & DI_IS) {
		if (host->src_buf_count > 0x0) {
			if(!host->dmabusy) {
				sd_start_dma(host, READ);
				sd_start_sd(host, READ);
			}
		} 
	} else {
		sd_start_dma(host, READ);
	}

	/* write sd card from internal buffer finished interrupt */
	if (sd_int_status & DO_IS) {
		if (host->src_buf_count > 0x0)
			sd_start_dma(host, WRITE);
		else
			transferdone(host);
	}
	/* DAT0 level change interrupt */
	if (sd_int_status & DAT0_STS) { 
		if ((!check_dat0_busy()) && (host->dat0_bus_busy == 0x01)) {
			host->dat0_bus_busy = 0x0;
			sd_disable_bus_monitor();
			sd_start_sd(host, WRITE);	
		}
	}
	/* card detect interrupt change */
	if (sd_int_status & CD_IS)
		mod_timer(&host->timer, jiffies + msecs_to_jiffies(25));

	nuc700_sd_write(REG_SDIISR, sd_int_status);
}

/*
 * Handle an interrupt
 */
static irqreturn_t nuc700_sd_irq(int irq, void *devid)
{
	struct nuc700_sd_host *host = devid;
	unsigned int sd_int_status;

	sd_int_status = nuc700_sd_read(REG_SDGISR);

	if (!(sd_int_status & (0x01 << 1))) {
		dev_err(host->mmc->parent, "strange, no interupt but it occurs\n");
		return IRQ_NONE;
	}

	/* sd host interrupt */
	if (sd_int_status & (0x01 << 3))
		sd_host_interrupt(host);

	/* dma write interrupt */
	if (sd_int_status & (0x01 << 4))
		sd_dma_interrupt(host, READ);

	/* dma read interrupt */
	if (sd_int_status & (0x01 << 5))
		sd_dma_interrupt(host, WRITE);

	/* bus error interrupt */
	if (sd_int_status & (0x01 << 6))
		dev_err(host->mmc->parent, "!!!bus error interrupt!!!\n");


	nuc700_sd_write(REG_SDGISR, sd_int_status);

	return IRQ_HANDLED;
}

static int nuc700_sd_get_ro(struct mmc_host *mmc)
{
	/* no write protect */
	return 0;
}

static const struct mmc_host_ops nuc700_sd_ops = {
	.request	= nuc700_sd_request,
	.set_ios	= nuc700_sd_set_ios,
	.get_ro		= nuc700_sd_get_ro,
	.get_cd		= nuc700_sd_card_detect,
};

void nuc700_sd_enable(void) {

	nuc700_sd_write(REG_SDGCR, EN_SDEN);
	nuc700_sd_write(REG_SDGIER, EN_ALLINT);
	nuc700_sd_write(REG_SDIIER, EN_SDINT);
}

void nuc700_sd_disable(void) {

	nuc700_sd_write(REG_SDGCR, nuc700_sd_read(REG_SDGCR) & (~EN_SDEN));
	nuc700_sd_write(REG_SDGIER, 0x0);
	nuc700_sd_write(REG_SDIIER, 0x0);
}

/*
 * Probe for the device
 */
static int __devinit nuc700_sd_probe(struct platform_device *pdev)
{
	struct nuc700_sd_port *pdata = pdev->dev.platform_data;
	struct mmc_host *mmc;
	struct nuc700_sd_host *host;
	struct resource *res;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENXIO;

	if (!request_mem_region(res->start, resource_size(res), DRIVER_NAME))
		return -EBUSY;

	mmc = mmc_alloc_host(sizeof(struct nuc700_sd_host), &pdev->dev);
	if (!mmc) {
		ret = -ENOMEM;
		dev_dbg(&pdev->dev, "couldn't allocate mmc host\n");
		goto fail6;
	}

	mmc->ops = &nuc700_sd_ops;
	mmc->f_min = 400000;
	mmc->f_max = 50000000;
	mmc->ocr_avail = MMC_VDD_27_28|MMC_VDD_28_29|MMC_VDD_29_30|
		MMC_VDD_30_31|MMC_VDD_31_32|MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->caps = 0;

	mmc->max_blk_size  = MCI_MAXBLKSIZE;
	mmc->max_blk_count = MCI_BLKATONCE;
	mmc->max_req_size  = MCI_BUFSIZE;
	mmc->max_segs      = MCI_BLKATONCE;
	mmc->max_seg_size  = MCI_BUFSIZE;

	host = mmc_priv(mmc);
	host->mmc = mmc;
	host->pwr_pin = pdata->pwr_pin;
	mmc->caps |= MMC_CAP_4_BIT_DATA;

	host->buffer = dma_alloc_coherent(&pdev->dev, MCI_BUFSIZE,
					  &host->physical_address, GFP_KERNEL);
	if (!host->buffer) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "Can't allocate transmit buffer\n");
		goto fail5;
	}

	/* get Clock */
	host->sd_clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(host->sd_clk)) {
		ret = -ENODEV;
		dev_dbg(&pdev->dev, "no sd_clk?\n");
		goto fail2;
	}

	/* enable the peripheral clock */
	clk_enable(host->sd_clk);

	/* allocate the MCI interrupt*/
	host->irq = platform_get_irq(pdev, 0);
	ret = request_irq(host->irq, nuc700_sd_irq, 0, mmc_hostname(mmc), host);
	if (ret) {
		dev_dbg(&pdev->dev, "request MCI interrupt failed\n");
		goto fail1;
	}

	if (host->pwr_pin) {
		ret = gpio_request(host->pwr_pin, "mmc_power");
		if (ret < 0) {
			dev_dbg(&pdev->dev, "couldn't claim card power pin\n");
			goto fail0;
		}
	}

	/* add a thread to check CO, RI, and R2 */
	kernel_thread(sd_event_thread, host, 0x0);

	setup_timer(&host->timer, nuc700_sd_timeout_timer, (unsigned long)host);

	platform_set_drvdata(pdev, mmc);

	/* enable SD interrupt & select GPIO detect */
	host->present = nuc700_sd_read(REG_SDIISR) & (SD_CD);
	nuc700_sd_enable();
	nuc700_sd_write(REG_SDHIIR, SD_CLK | (~SD_BUSW4B) | SD_BUSW1B);
	/* Add host to MMC layer */
	mmc_add_host(mmc);
	return 0;
fail0:
	free_irq(host->irq, host);
fail1:
	clk_disable(host->sd_clk);
	clk_put(host->sd_clk);
fail2:
	dma_free_coherent(&pdev->dev, MCI_BUFSIZE,
			host->buffer, host->physical_address);
fail5:
	mmc_free_host(mmc);
fail6:
	release_mem_region(res->start, resource_size(res));
	dev_err(&pdev->dev, "probe failed, err %d\n", ret);
	return ret;
}

/*
 * Remove a device
 */
static int __devexit nuc700_sd_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct nuc700_sd_host *host;

	host = mmc_priv(mmc);

	dma_free_coherent(&pdev->dev, MCI_BUFSIZE,
			host->buffer, host->physical_address);

	nuc700_sd_disable();
	del_timer_sync(&host->timer);
	mmc_remove_host(mmc);
	free_irq(host->irq, host);

	clk_disable(host->sd_clk);
	clk_put(host->sd_clk);

	mmc_free_host(mmc);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver nuc700_sd_driver = {
	.remove		= __devexit_p(nuc700_sd_remove),
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init nuc700_sd_init(void)
{
	return platform_driver_probe(&nuc700_sd_driver, nuc700_sd_probe);
}

static void __exit nuc700_sd_exit(void)
{
	platform_driver_unregister(&nuc700_sd_driver);
}

module_init(nuc700_sd_init);
module_exit(nuc700_sd_exit);

MODULE_AUTHOR("Wan ZongShun <mcuos.com@gmail.com>");
MODULE_DESCRIPTION("NUC900 SD/MMC driver!");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:nuc700-mmc");
