/****************************************************************************
 * 
 * Copyright (c) 2004 - 2009 Nuvoton. Tech. Corp. All rights reserved. 
 *
 ****************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     nuvoton_sc.h
 *
 * VERSION
 *     0.1
 *
 * DESCRIPTION
 *     Smart Card driver common structure and const
 *
 * DATA STRUCTURES
 *
 *
 * FUNCTIONS
 *
 * HISTORY
 *	2006/01/06		Ver 0.1 Created by PC34 QFu
 *
 * REMARK
 *     None
 *************************************************************************/

#ifndef NUVOTON_SC_H
#define NUVOTON_SC_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define SMARTCARD_MAJOR	124

//char MAX_ATR_LEN[33];

struct max_atrlen{
	unsigned char n[33];
};


#define MAX_CMD_LEN	(262)	/* header : 5, data : 256(max), le : 1, plus all 262 */

/* define I/O control command */
#define SMARTCARD_IOC_MAGIC	's'

#define SC_IOC_MAXNR			16

#define SC_IOC_POWERON			_IOW(SMARTCARD_IOC_MAGIC, 0, int)		/* card power on */
#define SC_IOC_POWEROFF		_IO(SMARTCARD_IOC_MAGIC, 1)			/* card power off */

/* this two command is for power saving */
#define SC_IOC_POWERUP			_IO(SMARTCARD_IOC_MAGIC, 2)			/* card power up */
#define SC_IOC_POWERDOWN		_IO(SMARTCARD_IOC_MAGIC, 3)			/* card power down */

#define SC_IOC_ACTIVATE			_IO(SMARTCARD_IOC_MAGIC, 4)			/* activate card and save ATR to buffer,
																			you can use GETATR to retrieve */
#define SC_IOC_DEACTIVATE		_IO(SMARTCARD_IOC_MAGIC, 5)			/* deactivate */
#define SC_IOC_C4C8READ		_IOR(SMARTCARD_IOC_MAGIC, 6, int)		/* for general use */
#define SC_IOC_C4C8WRITE		_IOW(SMARTCARD_IOC_MAGIC, 7, int)
#define SC_IOC_SETPARAMETER	_IOW(SMARTCARD_IOC_MAGIC, 8, struct sc_parameter *)	/* set reader parameter to
																						adjust card */
#define SC_IOC_CLEARFIFO		_IO(SMARTCARD_IOC_MAGIC, 9)			/* clear reception buffer */
#define SC_IOC_GETSTATUS		_IOR(SMARTCARD_IOC_MAGIC, 10, int)		/* check card present */
#define SC_IOC_SELECTCARD		_IO(SMARTCARD_IOC_MAGIC, 11)			/* for multi-card  use */
#define SC_IOC_GETATR			_IOR(SMARTCARD_IOC_MAGIC, 12, struct max_atrlen *)
#define SC_IOC_GETERRORNO		_IOR(SMARTCARD_IOC_MAGIC, 13, int)		/* get error number */
#define SC_IOC_MATCHREADER	_IOR(SMARTCARD_IOC_MAGIC, 14, int)		/* check whether the reader support the baudrate or not */

#define SC_IOC_SETPARITY		_IOW(SMARTCARD_IOC_MAGIC, 15, int)
#define SC_IOC_SETCLOCK			_IOW(SMARTCARD_IOC_MAGIC, 16, int)

/* iso7816 operation class */
#define SC_ISO_OPERATIONCLASS_A	(0x01)
#define SC_ISO_OPERATIONCLASS_B	(0x02)

/* define err_flag */
#define SC_ERR_ID						(0x00)

#define SC_ERR_NOERROR					(0x00)
#define SC_ERR_CARDREMOVED			(0x01 | SC_ERR_ID)
#define SC_ERR_TIMEOUT					(0x02 | SC_ERR_ID)
#define SC_ERR_READ						(0x03 | SC_ERR_ID)
#define SC_ERR_WRITE					(0x04 | SC_ERR_ID)
#define SC_ERR_NOTHISBAUDRATE			(0x05 | SC_ERR_ID)

#define SC_ERR_DEVICE					(0x10 | SC_ERR_ID)

#define SC_ERR_DEV_GENERAL				SC_ERR_DEVICE
#define SC_ERR_DEV_OVERHEAT			(0x01 | SC_ERR_DEVICE)
#define SC_ERR_DEV_PROTECTION			(0x02 | SC_ERR_DEVICE)
#define SC_ERR_DEV_EARLYANSWER		(0x03 | SC_ERR_DEVICE)
#define SC_ERR_DEV_PARITY				(0x04 | SC_ERR_DEVICE)
#define SC_ERR_DEV_OVERRUN			(0x05 | SC_ERR_DEVICE)
#define SC_ERR_DEV_FRAMING			(0x06 | SC_ERR_DEVICE)


struct sc_parameter{
	unsigned int n, t;		/* amount of etu in cycles, and others are in etus */
	unsigned int cwt, bwt;
	unsigned int fi, di;		/* default value is 372 , 1 */
};

#endif	/* _NUVOTON_H_ */

