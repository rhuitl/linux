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

/* Driver thread command */
#define SD_EVENT_NONE			0x00000000
#define SD_EVENT_CMD_OUT		0x00000001
#define SD_EVENT_RSP_IN			0x00000010
#define SD_EVENT_RSP2_IN		0x00000100

#define debug dev_dbg
#define debug_read dev_dbg

static int sd_event=0, sd_state_xfer=0;

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
	struct device	*dev;
	spinlock_t lock;	/* Mutex */
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
	unsigned int timeout;
	int src_buf_count;
	int dest_buf_count;
	int irq;
	int absent;

	dma_addr_t physical_address;
};

static void nuc700_dump_sd_register(void){
printk("#########FMI register################\n");
printk("REG_SDGCR, 0x%x\n",nuc700_sd_read(REG_SDGCR));
printk("REG_SDDSA, 0x%x\n",nuc700_sd_read(REG_SDDSA));
printk("REG_SDBCR, 0x%x\n",nuc700_sd_read(REG_SDBCR));
printk("REG_SDGIER, 0x%x\n",nuc700_sd_read(REG_SDGIER));
printk("REG_SDGISR, 0x%x\n",nuc700_sd_read(REG_SDGISR));
printk("REG_SDBIST, 0x%x\n",nuc700_sd_read(REG_SDBIST));
printk("#########SD register##############\n");
printk("REG_SDICR, 0x%x\n",nuc700_sd_read(REG_SDICR));
printk("REG_SDHIIR, 0x%x\n",nuc700_sd_read(REG_SDHIIR));
printk("REG_SDIIER, 0x%x\n",nuc700_sd_read(REG_SDIIER));
printk("REG_SDIISR, 0x%x\n",nuc700_sd_read(REG_SDIISR));
printk("REG_SDARG, 0x%x\n",nuc700_sd_read(REG_SDARG));
printk("REG_SDRSP0, 0x%x\n",nuc700_sd_read(REG_SDRSP0));
printk("REG_SDRSP1, 0x%x\n",nuc700_sd_read(REG_SDRSP1));
printk("REG_SDBLEN, 0x%x\n",nuc700_sd_read(REG_SDBLEN));
printk("#########################\n");
}

/* get data from sdcard to internal buffer */
static unsigned long get_data_from_sdcard_to_inbuffer(struct nuc700_sd_host *host)
{
	unsigned long val;

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 4));

	if (host->src_buf_index == 0x0)
		val |= SD_WRITE_BUF0;
	else if (host->src_buf_index == 0x1)
		val |= SD_WRITE_BUF1;

	return val;
}

/* put data from internal buffer to sdcard */
static unsigned long put_data_from_inbuffer_to_sdcard(struct nuc700_sd_host *host)
{
	unsigned long val;

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 8));

	if (host->src_buf_index == 0x0)
		val |= SD_READ_BUF0;
	else if (host->src_buf_index == 0x1)
		val |= SD_READ_BUF1;

	return val;
}

/* get data from internal buffer to dmabuff */
static unsigned long get_data_from_inbuffer_to_dmabuf(struct nuc700_sd_host *host)
{
	unsigned long val;

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 8));

	if (host->dest_buf_index == 0x0)
		val |= DMA_READ_BUF0;
	else if (host->dest_buf_index == 0x1)
		val |= DMA_READ_BUF1;

	return val;
}

/* put data from dmabuff to internal buffer*/
static unsigned long put_data_from_dmabuf_to_inbuffer(struct nuc700_sd_host *host)
{
	unsigned long val;

	val = (nuc700_sd_read(REG_SDGCR) & ~(0x07 << 4));

	if (host->dest_buf_index == 0x0)
		val |= DMA_WRITE_BUF0;
	else if (host->dest_buf_index == 0x1)
		val |= DMA_WRITE_BUF1;
	
	return val;
}

static void sd_start_op(struct nuc700_sd_host *host, 
				int sd, int sdmode, int dma, int dmamode)
{
	dma_addr_t current_dma_address;
	unsigned long val = 0x0, enable_data_tr;
	unsigned long flags;
	spin_lock_irqsave(&host->lock, flags);
	
	if (dma) {

		if(dmamode == READ) {	/* read */
			val |= get_data_from_inbuffer_to_dmabuf(host);
			enable_data_tr = EN_DMA_READ_BUF;
			debug_read(host->mmc->parent, "(5) CMD %d, DMAwill read data from host->dest_buf_index= %d, val = 0x%x,host->dest_buf_count=%d\n",host->cmd->opcode, host->dest_buf_index, val,host->dest_buf_count);
		} else {	/* write */
			val |= put_data_from_dmabuf_to_inbuffer(host);
			enable_data_tr = EN_DMA_WRITE_BUF;
		}

		current_dma_address = host->physical_address + host->dest_buf_count * host->blksz;

		host->dest_buf_index ^= 1;
		host->dest_buf_count++;

		host->dmabusy = 0x01;
		
		nuc700_sd_write(REG_SDDSA, current_dma_address);
		nuc700_sd_write(REG_SDBCR, host->blksz);

		val |=  enable_data_tr;

	}

	if (sd) {
		if (sdmode == READ) {	/* read */
			val |= get_data_from_sdcard_to_inbuffer(host);
			debug_read(host->mmc->parent, "(2) CMD %d, sd will read data to host->src_buf_index= %d, val = 0x%x,host->src_buf_count=%d\n",host->cmd->opcode, host->src_buf_index, val,host->src_buf_count);
			enable_data_tr = DI_EN;
		} else {/* write */
			val |= put_data_from_inbuffer_to_sdcard(host);
			enable_data_tr = DO_EN;
		}

		host->src_buf_index ^= 1;

		host->src_buf_count--;
		host->sdbusy = 0x01;

		nuc700_sd_write(REG_SDBLEN, host->blksz - 1);
		nuc700_sd_write(REG_SDICR, nuc700_sd_read(REG_SDICR) | enable_data_tr);
		
	}

	nuc700_sd_write(REG_SDGCR, val);
	spin_unlock_irqrestore(&host->lock, flags);
	
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

	host->absent = nuc700_sd_read(REG_SDIISR) & (SD_CD);
	ret = host->absent ? 0 : 1;
	return ret;
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
		debug(host->mmc->parent,  "CMD %d, will write bytes_xfered = %d\n", host->cmd->opcode, data->bytes_xfered);

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
 * Handle transmitted data
 */
static void nuc700_sd_handle_transmitted(struct nuc700_sd_host *host)
{
	nuc700_sd_update_bytes_xfered(host);
}

/* sd send command */
static int nuc700_sd_wait_cmd(void)
{
	unsigned long start = jiffies, timeout = 50;

	while(time_is_after_jiffies(start + timeout)){
		if((nuc700_sd_read(REG_SDICR) & CO_EN) == 0)
			return 0;
	}

	return -1;
}

static int nuc700_sd_response(int sd_event)
{

	unsigned long start = jiffies, timeout = 50, resp_enbale;

	if (sd_event == SD_EVENT_RSP2_IN) {
		resp_enbale = R2_EN;
		nuc700_sd_write(REG_SDGCR, (nuc700_sd_read(REG_SDGCR)&& 0xffffff83) | 0x30);
	} else
		resp_enbale = RI_EN;

	nuc700_sd_write(REG_SDICR,  resp_enbale);

	while(time_is_after_jiffies(start + timeout)){
		if((nuc700_sd_read(REG_SDICR) & resp_enbale) == 0)
			return 0;
	}
	
	nuc700_sd_write(REG_SDICR, ( ~resp_enbale));
	return -1;
}

static int nuc700_sd_command(int cmd, int arg)
{
	int retval;

	nuc700_sd_write(REG_SDARG, arg);
	nuc700_sd_write(REG_SDICR,  (cmd  << 8) | CO_EN);

	/* wait command complete */
	retval = nuc700_sd_wait_cmd();

	return retval;
}

static int nuc700_sd_cmd_and_resp(int cmd, int arg, int sd_event)
{
	int retval;

	retval = nuc700_sd_command(cmd, arg);

	if(retval)
		return retval;
	
	retval = nuc700_sd_response(sd_event);

	return retval;

}
static void nuc700_mmc_send_command(struct nuc700_sd_host *host,
						struct mmc_command *cmd);

static void nuc700_mmc_handle_data_done(struct nuc700_sd_host *host) {

	if (host->cmd->data->flags & MMC_DATA_WRITE) {
		nuc700_sd_handle_transmitted(host);
	} else if (host->cmd->data->flags & MMC_DATA_READ) {
		nuc700_sd_post_dma_read(host);
	}
}

/*
 * Reset the controller and restore most of the state
 */
static void nuc700_sd_reset_host(struct nuc700_sd_host *host)
{
        unsigned long flags;

        local_irq_save(flags);

	 nuc700_sd_write(REG_SDGCR, (nuc700_sd_read(REG_SDGCR) & ~EN_SDEN));
	 nuc700_sd_write(REG_SDIISR, 0x07);
	 nuc700_sd_write(REG_SDHIIR, ((nuc700_sd_read(REG_SDHIIR)) & (~SD_BUSW4B)) | SD_BUSW1B);
	 gpio_direction_output(host->pwr_pin, 1);
	/* enable the peripheral clock */
	 clk_disable(host->sd_clk);

        local_irq_restore(flags);
}

/*
 * Process the next step in the request
 */
static void nuc700_mmc_process_next(struct nuc700_sd_host *host)
{
	if (!(host->flags & FL_SENT_COMMAND)) {
		host->flags |= FL_SENT_COMMAND;
		nuc700_mmc_send_command(host, host->request->cmd);
	}
	else if ((!(host->flags & FL_SENT_STOP)) && host->request->stop) {
		host->flags |= FL_SENT_STOP;
		nuc700_mmc_send_command(host, host->request->stop);
	} else {
		del_timer(&host->timer);
		mmc_request_done(host->mmc, host->request);
	}
}

/*
 * Handle a command that has been completed
 */
void nuc700_sd_completed_command(struct nuc700_sd_host *host,
							unsigned int status)
{
	struct mmc_command *cmd = host->cmd;
	struct mmc_data *data = cmd->data;
	unsigned int err;

	err = nuc700_sd_read(REG_SDIISR);

	if (status & SD_EVENT_RSP_IN) {
		/* if not R2 */
		cmd->resp[0] = (nuc700_sd_read(REG_SDRSP0) << 8) |
					(nuc700_sd_read(REG_SDRSP1) & 0xff);

		cmd->resp[1] = cmd->resp[2] = cmd->resp[3] = 0;

		debug(host->mmc->parent, "CMD %d read reponse R1,R3...value = 0x%x\n",
						cmd->opcode, cmd->resp[0]);
	} else if (status & SD_EVENT_RSP2_IN) {
		/* if R2 */
		unsigned int tmpBuf[5], i;
		#define Swap32(val) ((val << 24) | ((val << 8) & 0xff0000) |\
					((val >> 8) & 0xff00) | (val >> 24))

		for (i=0; i<5; i++)
			tmpBuf[i] = Swap32(nuc700_sd_read(FB0_BASE_ADDR+i*4));

		for (i=0; i<4; i++)
				 cmd->resp[i] = ((tmpBuf[i] & 0x00ffffff)<<8) | ((tmpBuf[i+1] & 0xff000000)>>24);

		
		debug(host->mmc->parent, "CMD %d read reponse R2...value = 0x%x,0x%x,0x%x,0x%x \n",
				 cmd->opcode, cmd->resp[0],cmd->resp[1],
						cmd->resp[2],cmd->resp[3]);

		if (err & R2_CRC_7) {
			debug(host->mmc->parent, "R2 CRC Check status ok!\n");
			goto out;
		}
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
out:
	if (data) {
		debug(host->mmc->parent, "CMD %d there is data = %d?\n",
						cmd->opcode, cmd->data);
				
		data->bytes_xfered = 0;
		if (data->flags & MMC_DATA_READ) {
			debug_read(host->mmc->parent, "(1) CMD %d , sd_start_sd(host, READ), host->dest_buf_count = %d, host->src_buf_count=%d\n",cmd->opcode, host->dest_buf_count,host->src_buf_count);
			sd_start_sd(host, READ);
		} else if (data->flags & MMC_DATA_WRITE) {
			sd_start_dma(host, WRITE);
		}

		wait_event_interruptible(sd_wq_xfer, (sd_state_xfer != 0));
	}
	nuc700_mmc_process_next(host);
}

/*
 * Send a command
 */
static void nuc700_mmc_send_command(struct nuc700_sd_host *host,
						struct mmc_command *cmd)
{
	struct mmc_data *data = cmd->data;
	int ret;

	sd_state_xfer = 0;
	host->cmd = cmd;

	if (mmc_resp_type(cmd) != MMC_RSP_NONE) {
		/* set 136 bit response for R2, 48 bit response otherwise */
		if (mmc_resp_type(cmd) == MMC_RSP_R2) {
			sd_event = SD_EVENT_RSP2_IN;
			ret = nuc700_sd_cmd_and_resp(cmd->opcode, cmd->arg, sd_event);
		} else {
			sd_event = SD_EVENT_RSP_IN;
			ret = nuc700_sd_cmd_and_resp(cmd->opcode, cmd->arg, sd_event);
		}
	} else {
		ret = nuc700_sd_command(cmd->opcode, cmd->arg);
		sd_event = 0x0;
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
			debug(host->mmc->parent, "CMD %d , there is data to write\n",cmd->opcode);
			/*
			 * Handle a write
			 */
			nuc700_sd_sg_to_dma(host, data);

		} else
			debug(host->mmc->parent, "CMD %d , there is data to read, host->src_buf_index = %d,host->src_buf_count=%d\n",cmd->opcode, host->src_buf_index, host->src_buf_count);

	} else {
		host->blksz = 0;
		host->blocks = 0;
	}

	if (ret < 0x0) {
		dev_warn(host->mmc->parent, "CMD %d maybe timeout\n",cmd->opcode);
		host->cmd->error = -ETIMEDOUT;
		del_timer(&host->timer);
		mmc_request_done(host->mmc, host->request);
	} else {
		debug(host->mmc->parent, "CMD %d finished response and cmd\n",cmd->opcode);

		nuc700_sd_completed_command(host, sd_event);
	}
}

/* nuc700, this driver doesn't support sdio, so mark cmd5,cmd52 */
static int nuc700_judge_sd_mmc_sdio(struct nuc700_sd_host *host)
{
	struct mmc_command	*cmd;
	cmd = host->request->cmd;

	if ((cmd->opcode == SD_IO_RW_DIRECT) ||
					(cmd->opcode == SD_IO_SEND_OP_COND)) {

	#ifdef CONFIG_MMC_DEBUG
		pr_info("%s:do workaround for nuc700 at cmd%d\n",
				mmc_hostname(host->mmc ), cmd->opcode);
	#endif

		cmd->error = -EBADRQC;
		mmc_request_done(host->mmc, host->request);

		return -1;
	}

	return 0;
}

/*
 * Handle an MMC request
 */
static void nuc700_sd_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct nuc700_sd_host *host = mmc_priv(mmc);	
	int ret;
	
	host->request = mrq;
	host->flags = 0;
	
	ret = nuc700_judge_sd_mmc_sdio(host);
	
	if (!ret) {
		mod_timer(&host->timer, jiffies +  msecs_to_jiffies(5000));
		nuc700_mmc_process_next(host);
	}
}

/*
 * Set the IOS
 */

void nuc700_sd_enable(void) {
	nuc700_sd_write(REG_SDGIER, EN_ALLINT);
	nuc700_sd_write(REG_SDIIER, EN_SDINT);
}

void nuc700_sd_disable(void) {

	nuc700_sd_write(REG_SDGCR, (nuc700_sd_read(REG_SDGCR) | (SWEST) )& (~EN_SDEN));
	nuc700_sd_write(REG_SDGIER, 0x0);
	nuc700_sd_write(REG_SDIIER, 0x0);
}

static void nuc700_sd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	int clkdiv;
	struct nuc700_sd_host *host = mmc_priv(mmc);
	unsigned long nuc700_sd_master_clock = 80000000;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);

	if (ios->clock == 0) {
		/* Disable the sd controller */
		nuc700_sd_write(REG_SDGCR, (nuc700_sd_read(REG_SDGCR) & ~EN_SDEN));
		nuc700_sd_write(REG_SDIISR, 0x07);
		clkdiv = 0;
		/* enable the peripheral clock */
		clk_disable(host->sd_clk);
	} else {
		/* Enable the sd controller */
		
		nuc700_sd_write(REG_SDGCR, EN_SDEN);
		clkdiv = (nuc700_sd_master_clock / ios->clock) - 1;
		nuc700_sd_write(REG_SDHIIR, clkdiv);

		/* enable the peripheral clock */
		clk_enable(host->sd_clk);
	}
	if (ios->bus_width == MMC_BUS_WIDTH_4)
		nuc700_sd_write(REG_SDHIIR, (nuc700_sd_read(REG_SDHIIR)) | SD_BUSW4B);
	else
		nuc700_sd_write(REG_SDHIIR, ((nuc700_sd_read(REG_SDHIIR)) & (~SD_BUSW4B)) | SD_BUSW1B);
	/* maybe switch power to the card */
	switch (ios->power_mode) {
	case MMC_POWER_OFF:
		gpio_direction_output(host->pwr_pin, 1);	
		break;
	case MMC_POWER_UP:
		gpio_direction_output(host->pwr_pin, 0);
		break;
	case MMC_POWER_ON:
		nuc700_sd_write(REG_SDIISR, 0x07);
		nuc700_sd_write(REG_SDICR, nuc700_sd_read(REG_SDICR) | CLK74_OE);
		while (nuc700_sd_read(REG_SDICR) & CLK74_OE){
			host->absent = nuc700_sd_read(REG_SDIISR) & (SD_CD);
			if (host->absent)
				BUG_ON(1);
		}

		break;
	default:
		WARN_ON(1);
	}

	spin_unlock_irqrestore(&host->lock, flags);

}

static void transferdone(struct nuc700_sd_host *host) {

	debug(host->mmc->parent, "CMD %d, data transferdone\n",host->cmd->opcode);
	nuc700_mmc_handle_data_done(host);
	nuc700_sd_write(REG_SDIISR, 0x07);
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
		debug_read(host->mmc->parent, "(6) CMD %d,sd_dma_interrupt !!! read interbuffer to DMA buffer, src_buf_count=%d, dest_buf_count=%d, host->blocks=%d\n",host->cmd->opcode, host->src_buf_count, host->dest_buf_count, host->blocks);
		if (host->src_buf_count <= 0x0) {
			if (host->dest_buf_count >= host->blocks)
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

void mmc_start_host(struct mmc_host *host);

static void sd_host_interrupt(struct nuc700_sd_host *host)
{
	unsigned int sd_int_status;

	host->sdbusy = 0x0;

	sd_int_status = nuc700_sd_read(REG_SDIISR);
	
	/* read sd card to internal buffer finished interrupt */
	if (sd_int_status & DI_IS) {
		nuc700_sd_write(REG_SDIISR, DI_IS);
		debug_read(host->mmc->parent, "(3) CMD %d, sd_host_interrupt!!! read sd card to internal buffer finished, host->src_buf_count = %d \n",host->cmd->opcode, host->src_buf_count);
		if (host->src_buf_count > 0x0) {
			if(!host->dmabusy) {
				debug_read(host->mmc->parent, "(4) CMD %d, sd_host_interrupt!!! start new  sd_start_op(host, 1, READ, 1, READ) \n",host->cmd->opcode);
				sd_start_dma(host, READ);
			}
		}  else {
			debug_read(host->mmc->parent, "(7) CMD %d, sd_host_interrupt read all buffer finished, host->src_buf_count=%d, host->dest_buf_count=%d\n",host->cmd->opcode, host->src_buf_count, host->dest_buf_count);
			sd_start_dma(host, READ);
		}
	}

	/* write sd card from internal buffer finished interrupt */
	if (sd_int_status & DO_IS) {
		nuc700_sd_write(REG_SDIISR, DO_IS);
		if (host->src_buf_count > 0x0)
			sd_start_dma(host, WRITE);
		else
			transferdone(host);
	}
	/* DAT0 level change interrupt */
	if (sd_int_status & DAT0_STS) {
		nuc700_sd_write(REG_SDIISR, DAT0_STS);
		if ((!check_dat0_busy()) && (host->dat0_bus_busy == 0x01)) {
			host->dat0_bus_busy = 0x0;
			sd_disable_bus_monitor();
			sd_start_sd(host, WRITE);	
		}
	}
	/* card detect interrupt change */
	if (sd_int_status & CD_IS) {
		nuc700_sd_write(REG_SDIISR, CD_IS);
		nuc700_sd_reset_host(host);
		mmc_detect_change(host->mmc, 0);
	}
}

/*
 * Handle an interrupt
 */
static irqreturn_t nuc700_sd_irq(int irq, void *devid)
{
	struct nuc700_sd_host *host = devid;
	unsigned int sd_int_status;

	sd_int_status = nuc700_sd_read(REG_SDGISR);

	if (!(sd_int_status & (0x01 << 0))) {
		dev_err(host->mmc->parent, "strange, no interupt but it occurs\n");
		return IRQ_NONE;
	}

	/* sd host interrupt */
	if (sd_int_status & (0x01 << 3)) {

		sd_host_interrupt(host);
	}

	/* dma write interrupt */
	if (sd_int_status & (0x01 << 4)) {
		sd_dma_interrupt(host, READ);
	}

	/* dma read interrupt */
	if (sd_int_status & (0x01 << 5)) {
		sd_dma_interrupt(host, WRITE);
	}

	/* bus error interrupt */
	if (sd_int_status & (0x01 << 6))
		dev_err(host->mmc->parent, "!!!bus error interrupt!!!\n");


	nuc700_sd_write(REG_SDGISR, sd_int_status);

	return IRQ_HANDLED;
}

static int nuc700_sd_get_ro(struct mmc_host *mmc)
{
	/* no write protect */
	return -ENOSYS;
}

static void nuc700_timeout_timer(unsigned long data)
{
	struct nuc700_sd_host *host;

	host = (struct nuc700_sd_host *)data;

	if (host->request) {
		dev_err(host->mmc->parent, "Timeout waiting end of packet\n");

		if (host->cmd && host->cmd->data) {
			host->cmd->data->error = -ETIMEDOUT;
		} else {
			if (host->cmd)
				host->cmd->error = -ETIMEDOUT;
			else
				host->request->cmd->error = -ETIMEDOUT;
		}
		nuc700_dump_sd_register();
		mmc_request_done(host->mmc, host->request);
	}
}


static const struct mmc_host_ops nuc700_sd_ops = {
	.request	= nuc700_sd_request,
	.set_ios	= nuc700_sd_set_ios,
	.get_ro		= nuc700_sd_get_ro,
	.get_cd		= nuc700_sd_card_detect,
};

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

	mmc->max_blk_size  = 512;
	mmc->max_blk_count = 256;
	mmc->max_req_size  = (mmc->max_blk_size * mmc->max_blk_count);
	mmc->max_segs      = mmc->max_blk_count;
	mmc->max_seg_size  = mmc->max_req_size;

	host = mmc_priv(mmc);
	host->mmc = mmc;
	host->pwr_pin = pdata->pwr_pin;
	mmc->caps |= MMC_CAP_4_BIT_DATA;
	host->dev = &pdev->dev;
	
	host->buffer = dma_alloc_coherent(&pdev->dev, mmc->max_req_size,
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

	/* allocate the MCI interrupt*/
	host->irq = platform_get_irq(pdev, 0);
	ret = request_irq(host->irq, nuc700_sd_irq, IRQF_DISABLED, mmc_hostname(mmc), host);
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

	setup_timer(&host->timer, nuc700_timeout_timer, (unsigned long)host);

	platform_set_drvdata(pdev, mmc);

	nuc700_sd_enable();
	
	spin_lock_init(&host->lock);
	
	/* Add host to MMC layer */
	mmc_add_host(mmc);
	return 0;
fail0:
	free_irq(host->irq, host);
fail1:
	clk_disable(host->sd_clk);
	clk_put(host->sd_clk);
fail2:
	dma_free_coherent(&pdev->dev, mmc->max_req_size,
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

	dma_free_coherent(&pdev->dev, mmc->max_req_size,
			host->buffer, host->physical_address);

	del_timer_sync(&host->timer);
	nuc700_sd_disable();
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
MODULE_DESCRIPTION("NUC700 SD/MMC driver!");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:nuc700-mmc");
