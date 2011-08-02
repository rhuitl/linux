/****************************************************************************
 *
 * Copyright (c) 2004 - 2009 Nuvoton Tech. Corp. All rights reserved.
 *
 ****************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     nuc700_sc.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     NUC700 Smart Card driver
 *
 * DATA STRUCTURES
 *
 *
 * FUNCTIONS
 *	all functions, if they has return value, return 0 if they success, others failed.
 *    ( see below for detail )
 *
 * HISTORY
 *	2005/05/09		Ver 1.0 Created by PC34 QFu
 *
 * REMARK
 *     None
 *************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include <asm/uaccess.h>

#include <mach/hardware.h>
#include "nuc700_sc.h"

#define USE_IRQ


//#define SC_DEBUG
//#define SC_DEBUG_PRINT_LINE
//#define SC_DEBUG_ENABLE_ENTER_LEAVE
//#define SC_DEBUG_ENABLE_MSG
//#define SC_DEBUG_ENABLE_MSG2

#ifdef SC_DEBUG
#define PDEBUG(fmt, arg...)		printk(fmt, ##arg)
#else
#define PDEBUG(fmt, arg...)
#endif

#ifdef SC_DEBUG_PRINT_LINE
#define PRN_LINE()				PDEBUG("[%-20s %6d] : %d\n", __FUNCTION__, jiffies/HZ, __LINE__)
#else
#define PRN_LINE()
#endif

#ifdef SC_DEBUG_ENABLE_ENTER_LEAVE
#define ENTER()					PDEBUG("[%-20s %6d] : Enter...\n", __FUNCTION__, jiffies/HZ)
#define LEAVE()					PDEBUG("[%-20s %6d] : Leave...\n", __FUNCTION__, jiffies/HZ)
#else
#define ENTER()
#define LEAVE()
#endif

#ifdef SC_DEBUG_ENABLE_MSG
#define MSG(msg)				PDEBUG("[%-20s %6d] : %s\n", __FUNCTION__, jiffies/HZ, msg)
#else
#define MSG(msg)
#endif

#ifdef SC_DEBUG_ENABLE_MSG2
#define MSG2(fmt, arg...)			PDEBUG("[%-20s %6d] : "fmt, __FUNCTION__, jiffies/HZ, ##arg)
#define PRNBUF(buf, count)		{int i;MSG2("Data: ");for(i=0;i<count;i++)\
									PDEBUG("%02x ", buf[i]);PDEBUG("\n");}
#else
#define MSG2(fmt, arg...)
#define PRNBUF(buf, count)
#endif

/* smart card I/O function */
#define sc_outl(dev, byte, addr)		WRITE_REGISTER_WORD(dev->base + addr, byte)
#define sc_inl(dev, addr)				READ_REGISTER_WORD(dev->base + addr)

#define WRITE_REGISTER_WORD(port,value) 	*((unsigned int volatile*)(port))=value
#define READ_REGISTER_WORD(port)			*((unsigned int volatile*)(port))

#define sc_is_present(dev)		(sc_inl(dev, ISR) & 0x20)

#define sc_is_deactive(dev)		((dev)->bCardRemoved)

#define sc_is_data_arrive(dev)	(sc_inl(dev, SCSR) & 0x01)
#define sc_is_data_send(dev)		(sc_inl(dev, SCSR) & 0x40)

/* read one byte from card */
#define sc_get_byte(dev)			({u32 reg = sc_inl(dev, RBR);\
								MSG2("<- %02x\n", reg & 0xff); reg;})

/* write one byte to card */
#define sc_put_byte(data, dev)	do{sc_outl(dev, data, TBR); \
								MSG2("->%02x\n", data & 0xff);}while(0)

/* global variable for SmartCard Device */
static sc_dev sc_device[SMARTCARD_NUM];
const static int sc_base[] = { SC0_BASE_ADDR, SC1_BASE_ADDR};
const static int sc_irq[] = {IRQ_SC0, IRQ_SC1};


void sc_apply_power(int bIs5V, int smartcard_port)
{


#if defined(REAL_CHIP) && !defined(REAL_CHIP_V2)

 unsigned int uiValue;

  if(smartcard_port == 0)		//smartcard0
  {
	//set PT1CFG5 = 0 => GPIO25(SC0_PWR) , set PT2CFG0 = 0 => GPIO42
	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG1);
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, uiValue & 0xFFFFF3FF);

	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG2);
	WRITE_REGISTER_WORD(REG_GPIO_CFG2, uiValue & 0xFFFFFFFC);

	//set GPIO25 and GPIO42 on output mode
	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR1);
	WRITE_REGISTER_WORD(REG_GPIO_DIR1, uiValue | 0x20);

	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR2);
	WRITE_REGISTER_WORD(REG_GPIO_DIR2, uiValue | 0x01);

	if(bIs5V)
	{
		//set GPIO25 = 1
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT1);
		WRITE_REGISTER_WORD(REG_GPIO_OUT1, uiValue | 0x20);
		//set GPIO42 = 0
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT2);
		WRITE_REGISTER_WORD(REG_GPIO_OUT2, uiValue & 0xFFFFFFFE);
	}
	else
	{
		//set GPIO25 = 0
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT1);
		WRITE_REGISTER_WORD(REG_GPIO_OUT1, uiValue & 0xFFFFFFDF);
		//set GPIO42 = 1
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT2);
		WRITE_REGISTER_WORD(REG_GPIO_OUT2, uiValue | 0x01);
	}
  }
  else if(smartcard_port == 1) //smartcard1
  {
	//set PT1CFG1 = 0 => GPIO20(SC1_PWR) , set PT2CFG0 = 0 => GPIO42
	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG1);
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, uiValue & 0xFFFFFFFC);

	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG2);
	WRITE_REGISTER_WORD(REG_GPIO_CFG2, uiValue & 0xFFFFFFFC);

	//set GPIO20 and GPIO42 on output mode
	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR1);
	WRITE_REGISTER_WORD(REG_GPIO_DIR1, uiValue | 0x01);

	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR2);
	WRITE_REGISTER_WORD(REG_GPIO_DIR2, uiValue | 0x01);

	if(bIs5V)
	{
		//set GPIO20 = 1
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT1);
		WRITE_REGISTER_WORD(REG_GPIO_OUT1, uiValue | 0x01);
		//set GPIO42 = 0
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT2);
		WRITE_REGISTER_WORD(REG_GPIO_OUT2, uiValue & 0xFFFFFFFE);
	}
	else
	{
		//set GPIO20 = 0
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT1);
		WRITE_REGISTER_WORD(REG_GPIO_OUT1, uiValue & 0xFFFFFFFE);
		//set GPIO42 = 1
		uiValue = READ_REGISTER_WORD(REG_GPIO_OUT2);
		WRITE_REGISTER_WORD(REG_GPIO_OUT2, uiValue | 0x01);
	}
  }

#endif

//turn GPIO mode to smartcard mode
#ifdef REAL_CHIP
#ifndef VERSION1
    //all pins become smartcard pins
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, 0x000AAAAA);
#else
    //GPIO20(SC1_PWR) and GPIO25(SC2_PWR) for power control
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, 0x000AA2A8);
#endif
#endif

}

void sc_power_down(int smartcard_port)
{
#if defined(REAL_CHIP) && !defined(REAL_CHIP_V2)

  unsigned int uiValue;

  if(smartcard_port == 0)		//smartcard0
  {
	//set PT1CFG5 = 0 => GPIO25(SC0_PWR) , set PT2CFG0 = 0 => GPIO42
	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG1);
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, uiValue & 0xFFFFF3FF);

	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG2);
	WRITE_REGISTER_WORD(REG_GPIO_CFG2, uiValue & 0xFFFFFFFC);

	//set GPIO25 and GPIO42 on output mode
	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR1);
	WRITE_REGISTER_WORD(REG_GPIO_DIR1, uiValue | 0x20);

	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR2);
	WRITE_REGISTER_WORD(REG_GPIO_DIR2, uiValue | 0x01);


	//set GPIO25 = 0
	uiValue = READ_REGISTER_WORD(REG_GPIO_OUT1);
	WRITE_REGISTER_WORD(REG_GPIO_OUT1, uiValue & 0xFFFFFFDF);
	//set GPIO42 = 0
	uiValue = READ_REGISTER_WORD(REG_GPIO_OUT2);
	WRITE_REGISTER_WORD(REG_GPIO_OUT2, uiValue & 0xFFFFFFFE);
  }
  else if(smartcard_port == 1) //smartcard1
  {
	//set PT1CFG1 = 0 => GPIO20(SC1_PWR) , set PT2CFG0 = 0 => GPIO42
	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG1);
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, uiValue & 0xFFFFFFFC);

	uiValue = READ_REGISTER_WORD(REG_GPIO_CFG2);
	WRITE_REGISTER_WORD(REG_GPIO_CFG2, uiValue & 0xFFFFFFFC);

	//set GPIO20 and GPIO42 on output mode
	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR1);
	WRITE_REGISTER_WORD(REG_GPIO_DIR1, uiValue | 0x01);

	uiValue = READ_REGISTER_WORD(REG_GPIO_DIR2);
	WRITE_REGISTER_WORD(REG_GPIO_DIR2, uiValue | 0x01);

	//set GPIO20 = 0
	uiValue = READ_REGISTER_WORD(REG_GPIO_OUT1);
	WRITE_REGISTER_WORD(REG_GPIO_OUT1, uiValue & 0xFFFFFFFE);
	//set GPIO42 = 0
	uiValue = READ_REGISTER_WORD(REG_GPIO_OUT2);
	WRITE_REGISTER_WORD(REG_GPIO_OUT2, uiValue & 0xFFFFFFFE);
  }

#endif

//turn GPIO mode to smartcard mode
#ifdef REAL_CHIP
#ifndef VERSION1
    //all pins become smartcard pins
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, 0x000AAAAA);
#else
    //GPIO20(SC1_PWR) and GPIO25(SC2_PWR) for power control
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, 0x000AA2A8);
#endif
#endif

}

/* chech if reader support special baudrate */
static int sc_match_reader(int fi, int di)
{
	int  t;

	t = fi/di;

	if( (t % 31) == 0)
		return 0;
	else if( (t % 32) == 0)
			return 0;
	else
		return -1;

}

static void sc_set_baudrate(sc_dev *dev, int fi, int di)
{
	u32 reg;
	int t, psc, pdr;

	t = fi / di;

	if((t % 31) == 0){
		psc = 31;
		pdr = t / 31;
	}
	else{
		psc = 32;
		pdr = t / 32;
	}

#if( !defined(EIGHT_TIMES_FREQUENCY) && !defined(TWO_TIMES_FREQUENCY) )
	/* nothing to do */
#else
#ifdef EIGHT_TIMES_FREQUENCY
	psc <<= 3;		/* psc x 8 */
#else
	psc <<= 1;		/* psc x 2 */
#endif
#endif

	MSG2("pdr : %d, psc : %d\n", pdr, psc);

	sc_outl(dev, pdr, CBR);

	/* set BDLAB = 1 */
	reg = sc_inl(dev, SCCR);
	reg |= 0x80;
	sc_outl(dev, reg, SCCR);

	/* set Baud rate divisor */
	sc_outl(dev, psc & 0xff, BLL);
	sc_outl(dev, (psc >> 8) & 0xff, BLH);

	/* set BDLAB = 0 */
	reg &= 0x7f;
	sc_outl(dev, reg, SCCR);

}

static void sc_set_reader_parameter(sc_dev *dev, struct sc_parameter *p)
{
	u32 reg;

	/* set reader baud rate and update dev->clock , so we can calcute etu */
	ENTER();

	MSG2("fi : %d, di : %d, n : %d\n", p->fi, p->di, p->n);

	sc_set_baudrate(dev, p->fi, p->di);

	reg = sc_inl(dev, SCCR);
	reg |= (p->t == 0? 0: 0x04);
	sc_outl(dev, reg, SCCR);
	// avoid retransmit error
	if(p->t ==0 && p->n < 3)
	  p->n = 3;
	sc_outl(dev, p->n, GTR);

	LEAVE();

}

static int sc_set_reference_clock(sc_dev *dev, int clock)
{
	u32 d;

	switch(clock) {
		case 80000:	d = 0x00;	break;
		case 40000:	d = 0x10;	break;
		case 20000:	d = 0x20;	break;
		case 10000:	d = 0x30;	break;
		case 5000:	d = 0x40;	break;
		case 2500:	d = 0x50;	break;
		case 1250:	d = 0x60;	break;

		default:
			return -EPERM;
	}

	dev->clock = d;

	return 0;
}


/* this routine is actually a supply condition selector
    PowerOn and PowerOff may be modified in future  */
static void sc_power_on(sc_dev *dev, int opclass)
{
	int bIs5V = 0;
	u32 reg;

	ENTER();

	dev->voltage = opclass;

	/* select power supply */
	switch(opclass){
		case SC_ISO_OPERATIONCLASS_A:		/* 5v */
			bIs5V = 1;
			break;

		default:
		case SC_ISO_OPERATIONCLASS_B:		/* 3v */
			break;
	}

	sc_apply_power(bIs5V, dev->num);

	reg = sc_inl(dev, IER);
	reg &= 0x77f;
	sc_outl(dev, reg, IER);

	//======== Write IER again..... [yachen. July 31, 2006]
	mdelay(10);
	sc_outl(dev, reg, IER);
	//========

	LEAVE();

}

static void sc_power_off(sc_dev *dev)
{
	//GPIO5 => 0V
	u32 reg;

	ENTER();

	reg = sc_inl(dev, IER);
	reg |= 0x80;
	sc_outl(dev, reg, IER);

	sc_power_down(dev->num);

	LEAVE();

}

static void sc_clear_FIFO(sc_dev *dev)
{
//	sc_outl(dev, sc_inl(dev, SCFR) | 0x06, SCFR);
	while(sc_is_data_arrive(dev))sc_get_byte(dev);
	dev->snd_start = 0;
	dev->snd_end = 0;
	dev->rcv_start = 0;
	dev->rcv_end = 0;
}

static void sc_reset_reader(sc_dev *dev)	//modified by qfu
{
	u32 reg;

	ENTER();

	reg = sc_inl(dev, SCCR);
	reg &= 0x7f;
	sc_outl(dev, reg, SCCR);

	sc_outl(dev, dev->clock, ECR);

	sc_outl(dev, 0x7e, SCFR);		/* set for FIFO threshold level = 1
								    and reset, retry times = 0 */

	sc_outl(dev, 12, CBR);		/* sampling rate 12 */

	reg = sc_inl(dev, SCCR);		/* set BDLAB = 1 */
	reg |= 0x80;
	sc_outl(dev, reg, SCCR);


#if( !defined(EIGHT_TIMES_FREQUENCY) && !defined(TWO_TIMES_FREQUENCY) )
    sc_outl(dev, 31, BLL);
#else
#ifdef EIGHT_TIMES_FREQUENCY
    sc_outl(dev, 248, BLL);
#else
    sc_outl(dev, 62, BLL);
#endif
#endif
	sc_outl(dev, 0, BLH);

	reg = sc_inl(dev, SCCR);		/* set BDLAB = 0 */
	reg &= 0x7f;
	sc_outl(dev, reg, SCCR);

	sc_outl(dev, /*2*/3, GTR);		/* guard time */

	reg = sc_inl(dev, IER);

#ifdef USE_IRQ
	reg |= 0x60d;		/* enable status, read, tof1 and tof2 */
#else
	reg |= 0x60c;		/* enable status, tof1 and tof2 */
#endif

#ifndef VERSION1
		reg = reg & 0x7BF;			//for 8024 interface
#else
		reg = reg | 0x040;			//for power switch
#endif


//	sc_outl(dev, reg | 0x80, IER);
	sc_outl(dev, reg, IER);

	sc_outl(dev, 0xc0, BTOR);		/* enable buffer timeout interrupt */

	/* init sc_dev structure */
	dev->para.cwt = 9600;
	dev->para.bwt = 9600;
	dev->snd_end = 0;
	dev->snd_start = 0;

	LEAVE();

}

static void sc_start_counter24(sc_dev *dev, u32 timeout)
{
	u32 reg;

	sc_outl(dev, timeout & 0xffffff, TOIR2);
	reg = sc_inl(dev, TOC);
	reg &= (~0x700);
	reg |= (0x01 << 8);
	sc_outl(dev, reg, TOC);

}

static void sc_stop_counter24(sc_dev *dev)
{
  u32 reg;

	reg = sc_inl(dev, TOC);
	reg &= (~0x700);
	sc_outl(dev, reg, TOC);
}

static void sc_start_counter16(sc_dev *dev, u32 timeout)
{
	u32 reg;

	sc_outl(dev, timeout & 0xffff, TOIR1);
	reg = sc_inl(dev, TOC);
	reg &= (~0x70);
	reg |= (0x02 << 4);
	sc_outl(dev, reg, TOC);

}

static void sc_stop_counter16(sc_dev *dev)
{
  u32 reg;

	reg = sc_inl(dev, TOC);
	reg &= (~0x70);
	sc_outl(dev, reg, TOC);
}

static void sc_start_counter(sc_dev *dev)
{

	dev->errno = 0;

	sc_start_counter24(dev, dev->para.bwt);
	sc_start_counter16(dev, dev->para.cwt);
}

static void sc_stop_counter(sc_dev *dev)
{

	sc_stop_counter16(dev);
	sc_stop_counter24(dev);

}

/* raw write/read to/from card */
static void sc_raw_write(sc_dev *dev)
{

	ENTER();

	dev->errno = 0;

	for(; dev->snd_end <dev->snd_start; dev->snd_end++) {
		while(!sc_is_data_send(dev) && !dev->bCardRemoved && !dev->errno);
		if (dev->bCardRemoved || dev->errno || dev->bCardChanged)
			return;

		sc_put_byte(dev->snd_buf[dev->snd_end], dev);
	}

	while(!sc_is_data_send(dev) && !dev->bCardRemoved && !dev->errno);

	LEAVE();
}

#ifdef USE_IRQ

static int sc_raw_read(sc_dev *dev, int count)
{
	ENTER();

	sc_start_counter(dev);

	wait_event_interruptible(dev->wq,
		dev->errno || dev->bCardRemoved ||
		(((dev->rcv_start - dev->rcv_end + MAX_BUF_LEN) % MAX_BUF_LEN) >= count));

	sc_stop_counter(dev);

	if (dev->bCardRemoved || dev->bCardChanged)
		return 0;

	if (dev->errno)
		return (dev->rcv_start - dev->rcv_end + MAX_BUF_LEN) % MAX_BUF_LEN;

	LEAVE();

	return count;
}

#else

static int sc_raw_read(sc_dev *dev, int count)
{
	u32 cnt;

	ENTER();

	sc_start_counter(dev);

	cnt = 0;
	dev->errno = 0;
	while(!dev->errno) {
		if (cnt >= count ||dev->bCardRemoved || dev->bCardChanged )
			break;
		if (sc_is_data_arrive(dev)) {
			dev->rcv_buf[dev->rcv_start++] = sc_get_byte(dev);
			cnt++;
		}
	}

	sc_stop_counter(dev)

	LEAVE();

	return cnt;
}

#endif

static int sc_activate(sc_dev *dev)
{
	int reg;

	ENTER();

	sc_reset_reader(dev);	/* reset reader */

	if (dev->parity != 0)
		sc_outl(dev, 0x10, SCCR);
	else
		sc_outl(dev, 0x00, SCCR);

	reg = sc_inl(dev, TMR);
	reg |= 0x02;
	sc_outl(dev, reg, TMR);

	sc_outl(dev, /*2*/ 3, GTR);

	reg = sc_inl(dev, SCCR);
	reg |= 0x80;
	sc_outl(dev, reg, SCCR);

#if( !defined(EIGHT_TIMES_FREQUENCY) && !defined(TWO_TIMES_FREQUENCY) )
    sc_outl(dev, 31, BLL);
#else
#ifdef EIGHT_TIMES_FREQUENCY
    sc_outl(dev, 248, BLL);
#else
    sc_outl(dev, 62, BLL);
#endif
#endif
	sc_outl(dev, 0, BLH);

	sc_outl(dev, /*2*/ 3, GTR);

	reg &= 0x7f;
	sc_outl(dev, reg, SCCR);

	//dev->bCardRemoved = 0;
	dev->bCardChanged = 0;

	/* read ATR */
	dev->atr_len = sc_raw_read(dev, 33);

	memcpy(dev->atr, dev->rcv_buf + dev->rcv_end, dev->atr_len);

	dev->rcv_start = 0;
	dev->rcv_end = 0;

#ifdef SC_DEBUG
	MSG2("ATR : Length : %d\n", dev->atr_len);
	for( j = 0; j < dev->atr_len; j ++){
		PDEBUG("%02x ", dev->atr[j]);
	}
	PDEBUG("\n");
#endif

	if(dev->bCardRemoved)
		return -1;

	LEAVE();

	return 0;

}

static void sc_deactivate(sc_dev *dev)
{
	u32 reg;

	ENTER();

	//dev->bCardRemoved = 1;	/* disable read/write/ioctl */

	reg = sc_inl(dev, TMR);
	reg &= 0xfd;
	sc_outl(dev, reg, TMR);

	sc_inl(dev, IER);
	reg &= 0xf8;			/* disable all int except card present */
	reg |= 0x80;			/* power down */
	sc_outl(dev, reg, IER);

	LEAVE();

}

static irqreturn_t  sc_interrupt(int irq, void * dev_id)
{
	u32 isr, scsr, id;
	sc_dev *dev = (sc_dev *)dev_id;

	ENTER();

	isr = sc_inl(dev, ISR);

//	MSG2("INT : ISR : [%04x]  SCSR : [%04x]\n", isr & 0xffff, scsr & 0xffff);

	if(isr & 0x01)		/* no interupt pending */
		return IRQ_NONE;

	if((isr & 0x10)){		/* tag card removed */

		MSG2("Card Present Int : ISR : [%02x]\n", isr);

		if( isr & 0x20){
			MSG("Card Inserted\n");
			dev->bCardChanged = 1;
			dev->bCardRemoved = 0;
		}
		else{
			dev->bCardRemoved = 1;
			dev->bCardChanged = 0;
			MSG("Card Removed\n");
			//if(dev->openflag){
			sc_deactivate(dev);
			sc_power_off(dev);
			if(dev->openflag) {
				dev->errno = SC_ERR_CARDREMOVED;
				goto quit_wake_up;
			}
		}
	}

	id = (isr & 0x0f);

	if (id == 0x0e) {
		scsr = sc_inl(dev, SCSR);
		MSG("Set dev->errno = SC_ERR_TIMEOUT\n");
		dev->errno = SC_ERR_TIMEOUT;
		goto quit_wake_up;
	}else if (id == 0x06) {
		scsr = sc_inl(dev, SCSR);
		if(scsr & 0x02){
			MSG("Over Run\n");
			sc_get_byte(dev);
			dev->errno = SC_ERR_DEV_OVERRUN;
		}else if(scsr & 0x04){
			MSG("Parity Error\n");
			sc_outl(dev, 0x7e, SCFR);  //reset FIFO
			dev->errno = SC_ERR_DEV_PARITY;
		}else if(scsr & 0x08){
			MSG("No stop bit error\n");
			dev->errno = SC_ERR_DEV_FRAMING;
		}else
			dev->errno = SC_ERR_DEV_GENERAL;
		goto quit_wake_up;
	}else if (id == 0x0c || id == 0x04) {

#ifdef USE_IRQ
		while(sc_is_data_arrive(dev)){
			dev->rcv_buf[dev->rcv_start] = sc_get_byte(dev);
			dev->rcv_start = (dev->rcv_start + 1) % MAX_BUF_LEN;
		}
		goto quit_wake_up;
#endif

	}

	LEAVE();

	return IRQ_HANDLED;

quit_wake_up:

	MSG("Just Wake up dev->wq\n");

	wake_up_interruptible(&dev->wq);

	return IRQ_HANDLED;
}




static int sc_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	sc_dev *dev=(sc_dev *)filp->private_data;

	ENTER();

	if(dev->bCardRemoved)
		return -EIO;

	dev->errno = 0;

	if(count == 0)
		return 0;

	if( count > (MAX_BUF_LEN - dev->rcv_start ))
		count = MAX_BUF_LEN - dev->rcv_start;

	sc_raw_read(dev, count);

	if(dev->bCardRemoved || dev->errno || dev->bCardChanged)
		return -EIO;

	if ( count > 0) {
		if(copy_to_user(buf, dev->rcv_buf + dev->rcv_end, count))
			return -EFAULT;
	}

	dev->rcv_end += count;

	LEAVE();

	return count;
}

static int sc_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	sc_dev *dev=(sc_dev *)filp->private_data;

	ENTER();

	if(dev->bCardRemoved)
		return -EIO;

	if(count == 0)
		return 0;

	if( count > (MAX_BUF_LEN - dev->snd_start))
		count = (MAX_BUF_LEN - dev->snd_start);

	if(copy_from_user(dev->snd_buf + dev->snd_start, buf, count))
		return -EFAULT;

	dev->snd_start += count;
	dev->errno = 0;

	sc_raw_write(dev);

	if(dev->bCardRemoved || dev->errno || dev->bCardChanged)
		return -EIO;

	LEAVE();

	return count;
}
static long sc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	int err = 0, tmp;

	sc_dev *dev = (sc_dev *)file->private_data;

	ENTER();

	if(_IOC_TYPE(cmd) != SMARTCARD_IOC_MAGIC) return -ENOTTY;
	if(_IOC_NR(cmd) > SC_IOC_MAXNR) return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if(err)
		return -EFAULT;

	dev->errno = 0;

	switch(cmd){
		case SC_IOC_POWERON:
			sc_power_on(dev, arg);
			break;
		case SC_IOC_POWEROFF:
			sc_power_off(dev);
			break;

		case SC_IOC_ACTIVATE:
			if(sc_activate(dev))
				err = -EIO;
			break;

		case SC_IOC_DEACTIVATE:
			sc_deactivate(dev);
			break;

		case SC_IOC_CLEARFIFO:
			MSG("SC_IOC_CLEARFIFO\n");
			sc_clear_FIFO(dev);
			break;

		case SC_IOC_SETPARAMETER:
			if(copy_from_user((void *)&dev->para, (const void *)arg, sizeof(struct sc_parameter)))
				return -EFAULT;

			sc_set_reader_parameter(dev, &dev->para);

			break;

		case SC_IOC_GETATR:

			//MSG2("IOC : GET ATR : ARG : %08x\n", arg);

			if(copy_to_user((void *)arg, (void *)&(dev->atr_len), 1))
				return -EFAULT;

			if(copy_to_user((void *)(arg + 1), (void *)dev->atr, dev->atr_len))
				return -EFAULT;

			break;

		case SC_IOC_GETSTATUS:

			if (dev->bCardChanged != 0)
				dev->bCardRemoved = !sc_is_present(dev);

			tmp = dev->bCardRemoved?0:0x80000000;
			tmp |= dev->bCardChanged;

			if(put_user(tmp, (int *)arg))
				err = -EFAULT;
			break;

		case SC_IOC_MATCHREADER:	/* bit 16 - bit31 = fi  bit15 - bit 0 = di */
			if(sc_match_reader((arg>>16) & 0xffff, arg & 0xffff))
				err = -EPERM;
			break;

		case SC_IOC_GETERRORNO:
			if (put_user(dev->errno, (int *)arg))
				err = -EFAULT;
			break;

		case SC_IOC_SETPARITY:
			dev->parity = arg;
			break;

		case SC_IOC_SETCLOCK:
			if (sc_set_reference_clock(dev, arg))
				err = -EPERM;
			break;

		default:
			return -ENOTTY;
	}

	LEAVE();

	return err;
}

static int sc_release(struct inode *inode, struct file *flip)
{
	sc_dev *dev = flip->private_data;

	ENTER();

	sc_deactivate(dev);
	sc_power_off(dev);
	//sc_reset_reader(dev);

	sc_outl(dev, 0x80, IER);	/* disable all interrupt */

	kfree(dev->snd_buf);
	kfree(dev->rcv_buf);

	free_irq(sc_irq[dev->num], dev);

	//MOD_DEC_USE_COUNT;

//	MSG2("ISR : [%02x]\n", sc_inl(dev, ISR));

	dev->openflag = 0;

	LEAVE();

	return 0;
}

static int sc_open(struct inode *inode, struct file *filp)
{
	sc_dev *dev;
	u32 num = MINOR(inode->i_rdev), reg;
	int retval;

	ENTER();

	if( num >= SMARTCARD_NUM )
		return -ENODEV;

	dev = (sc_dev *)&sc_device[num];

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	retval = -EBUSY;

	if( dev->openflag != 0 )		/* a card slot can open only once */
		goto open_quit;

	retval = -ENOMEM;
	dev->snd_buf = kmalloc(MAX_BUF_LEN, GFP_KERNEL);
	if ( dev->snd_buf == NULL)
		goto open_quit;

	dev->rcv_buf = kmalloc(MAX_BUF_LEN, GFP_KERNEL);
	if ( dev->rcv_buf == NULL){
		kfree(dev->snd_buf);
		goto open_quit;
	}

	if ((retval = request_irq(sc_irq[num], sc_interrupt, 0, "smartcard", &sc_device[num])) < 0) {

		printk("Can not request irq %d\n", sc_irq[num]);

		kfree(dev->snd_buf);
		kfree(dev->rcv_buf);
		goto open_quit;
	}

	/* set parameter to default */
#ifdef REAL_CHIP
#if(!defined(EIGHT_TIMES_FREQUENCY) && !defined(TWO_TIMES_FREQUENCY))
	sc_set_reference_clock(dev, 2500);	//for system clock 80MHz
#else
#ifdef EIGHT_TIMES_FREQUENCY
	sc_set_reference_clock(dev, 20000);	//for system clock 80MHz, and output frequency will divid 8
#else
	sc_set_reference_clock(dev, 5000);	//for system clock 80MHz, and output frequency will divid 2
#endif
#endif
#else
	sc_set_reference_clock(dev, 10000);   //for system clock 15MHz
#endif
	dev->parity = 1;	/* even parity */

	dev->openflag = 1;

	//MOD_INC_USE_COUNT;

	reg = sc_inl(dev, IER);
	reg |= 0x08;	/* enable card insert/remove */
	sc_outl(dev, reg, IER);

	if(sc_is_present(dev)){
		dev->bCardRemoved = 0;
		dev->bCardChanged = 1;
	}
	else{
		dev->bCardRemoved = 1;
		dev->bCardChanged = 0;
	}

	filp->private_data = dev;

	retval = 0;

open_quit:
	up(&dev->sem);
	LEAVE();

	return retval;
}




struct file_operations sc_fops =
{
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= sc_ioctl,
	.open			= sc_open,
	.release		= sc_release,
	.write			= sc_write,
	.read			= sc_read,
};

static int __init sc_device_init(void)
{
	int i;
	sc_dev *dev;

#ifdef REAL_CHIP_V2
	WRITE_REGISTER_WORD(REG_GPIO_CFG0, READ_REGISTER_WORD(REG_GPIO_CFG0) & 0x3c3);
	WRITE_REGISTER_WORD(REG_GPIO_DIR0, READ_REGISTER_WORD(REG_GPIO_DIR0) | 0x6);
	WRITE_REGISTER_WORD(REG_GPIO_OUT0, ((READ_REGISTER_WORD(REG_GPIO_OUT0) & 0x1b) | 0x2));
#endif

//turn GPIO mode to smartcard mode
#ifdef REAL_CHIP
#ifndef VERSION1
    //all pins become smartcard pins
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, 0x000AAAAA);
#else
    //GPIO20(SC1_PWR) and GPIO25(SC2_PWR) for power control
	WRITE_REGISTER_WORD(REG_GPIO_CFG1, 0x000AA2A8);
#endif
#endif

	for(i = 0; i < SMARTCARD_NUM; i ++){
		memset(&sc_device[i], 0, sizeof(sc_dev));
		dev = &sc_device[i];
		dev->num = i;
		dev->base = sc_base[i];

		init_waitqueue_head(&dev->wq);
		sema_init(&dev->sem, 1);

//		sc_reset_reader(dev);

	}

	return 0;

}

static int __init smartcard_init(void)
{
	int result = -ENODEV;

	if(sc_device_init())
		goto failed;

	/* every things ok, now, we can register char device safely */

	result = register_chrdev(SMARTCARD_MAJOR, "smartcard", &sc_fops);
	if( result < 0){
		unregister_chrdev(SMARTCARD_MAJOR, "smartcard");
		printk("can't get major %d\n", SMARTCARD_MAJOR);
		return result;
	}

	printk("SmartCard driver has been installed successfully!\n");

failed:

	return result;

}

static void smartcard_exit(void)
{
	unregister_chrdev(SMARTCARD_MAJOR, "smartcard");
}


module_init(smartcard_init);
module_exit(smartcard_exit);

