/****************************************************************************
 * 
 * Copyright (c) 2004 - 2009 Nuvuton Tech. Corp. All rights reserved. 
 *
 ****************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     nuc700_sc.h
 *
 * VERSION
 *     1.1
 *
 * DESCRIPTION
 *     Smart Card driver supported for nuc700.
 *
 * DATA STRUCTURES
 *
 *
 *
 * FUNCTIONS
 *	all functions, if they has return value, return 0 if they success, others failed.
 *    ( see below for detail )
 *
 * HISTORY
 *	2006/01/06		Move some structure and const to "nuvoton_sc.h"
 *	2005/05/09		Ver 1.0 Created by PC34 QFu
 *
 * REMARK
 *     None
 *************************************************************************/

#ifndef _NUC700_SC_H_
#define _NUC700_SC_H_

#include <linux/types.h>
#include <linux/ioctl.h>

#include <mach/regs-sc.h>	/* for some structure and const */

#define REAL_CHIP_V2
//#define TWO_TIMES_FREQUENCY 

#define REAL_CHIP
#define VERSION1 //power switch (power control interface)
//#define EIGHT_TIMES_FREQUENCY

#define REG_GPIO_CFG0		0xFFF83000
#define REG_GPIO_DIR0		0xFFF83004
#define REG_GPIO_OUT0		0xFFF83008
#define REG_GPIO_IN0		0xFFF8300C
#define REG_GPIO_CFG1		0xFFF83010
#define REG_GPIO_DIR1		0xFFF83014
#define REG_GPIO_OUT1		0xFFF83018
#define REG_GPIO_IN1		0xFFF8301C
#define REG_GPIO_CFG2		0xFFF83020
#define REG_GPIO_DIR2		0xFFF83024
#define REG_GPIO_OUT2		0xFFF83028
#define REG_GPIO_IN2		0xFFF8302C
#define REG_GPIO_CFG3		0xFFF83030
#define REG_GPIO_DIR3		0xFFF83034
#define REG_GPIO_OUT3		0xFFF83038
#define REG_GPIO_IN3		0xFFF8303C

#define SC0_BASE_ADDR		0xFFF85000
#define SC1_BASE_ADDR		0xFFF85800

#define SC_IOMEM_SIZE		0x48

#define SMARTCARD_NUM	2      // we have 2 cards

/* register map for SCHI on nuc700 */
/* BDLAB = 0 */
#define RBR			0x00
#define TBR 			0x00
#define IER			0x04
#define ISR			0x08
#define SCFR			0x08
#define SCCR			0x0c
#define CBR			0x10
#define SCSR			0x14
#define GTR			0x18
#define ECR			0x1c
#define TMR			0x20
/* version B new */
#define TOC			0x28
#define TOIR0		0x2c
#define TOIR1		0x30
#define TOIR2		0x34
#define TODR0		0x38
#define TODR1		0x3c
#define TODR2		0x40
#define BTOR			0x44
/* BDLAB = 1 */
#define BLL			0x00
#define BLH			0x04
#define SCIDNR		0x08

#define MAX_BUF_LEN	500		/* just fit for kmalloc */

typedef struct{
	int base, num;
	volatile u32 errno;
	u32 openflag;
	u32 clock;
	u32 parity;
	u32 voltage;

	struct sc_parameter para;

	char atr[33];
	int atr_len;

	char *snd_buf, *rcv_buf;
	volatile int snd_start, snd_end, snd_len;
	volatile int rcv_start, rcv_end, rcv_len;
	volatile u32 jiffies, bCardRemoved, bCardChanged, fix_count;

	wait_queue_head_t wq;
	struct semaphore sem;
}sc_dev;

#endif	/* _NUC700_SC_H_ */

