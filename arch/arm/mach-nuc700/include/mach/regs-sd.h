/*
 * arch/arm/mach-nuc700/include/mach/regs-i2c.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <mach/hardware.h>

#ifndef __ASM_ARCH_REGS_I2C_H
#define __ASM_ARCH_REGS_I2C_H

/* SD Control Registers */

#define REG_SDGCR		(SD_BA + 0x00)
#define REG_SDDSA		(SD_BA + 0x04)
#define REG_SDBCR		(SD_BA + 0x08)
#define REG_SDGIER		(SD_BA + 0x0C)
#define REG_SDGISR		(SD_BA + 0x10)
#define REG_SDBIST		(SD_BA + 0x14)

#define SD0_BA			(SD_BA + 0x300)

#define REG_SDICR		(SD0_BA + 0x00)
#define REG_SDHIIR		(SD0_BA + 0x04)
#define REG_SDIIER		(SD0_BA + 0x08)
#define REG_SDIISR		(SD0_BA + 0x0C)
#define REG_SDARG		(SD0_BA + 0x10)
#define REG_SDRSP0		(SD0_BA + 0x14)
#define REG_SDRSP1		(SD0_BA + 0x18)
#define REG_SDBLEN		(SD0_BA + 0x1C)

/* Flash buffer 0 registers */
#define FB0_BASE_ADDR		(SD_BA + 0x400)
#define FB0_SIZE		0x200

/* REG_SDIISR */
#define DAT0_VAL		(0x01 << 9)
#define DAT0_STS		(0x01 << 8)
#define SD_CD			(0x01 << 7)
#define CRC_7			(0X01 << 3)
#define CRC_16			(0X01 << 4)
#define R2_CRC_7		(0X01 << 6)
#define CD_IS			(0X01 << 2)
#define DO_IS			(0X01 << 1)
#define DI_IS			(0X01 << 0)

/* REG_SDGCR */
#define EN_SDEN			(0x01 << 0)
#define SD_WRITE_BUF0		(0x03 << 4)
#define SD_WRITE_BUF1		(0x07 << 4)

#define SD_READ_BUF0		(0x03 << 8)
#define SD_READ_BUF1		(0x07 << 8)

#define DMA_READ_BUF0		(0x00 << 8)
#define DMA_READ_BUF1		(0x04 << 8)

#define DMA_WRITE_BUF0		(0x00 << 4)
#define DMA_WRITE_BUF1		(0x04 << 4)

#define EN_DMA_READ_BUF		(0x01 << 2)
#define EN_DMA_WRITE_BUF	(0x01 << 3)

/* REG_SDGIER */
#define EN_ALLINT		(0x3f << 0)

/* REG_SDHIIR */
#define SD_CLK			(80000/400 - 1)
#define SD_BUSW4B		(0x01 << 8)
#define SD_BUSW1B		(0x00 << 8)

/* REG_SDIIER */
#define EN_SDINT		(EN_DIEN | EN_DOEN | EN_CDEN)
#define EN_DIEN			(0x01 << 0)
#define EN_DOEN			(0x01 << 1)
#define EN_CDEN			(0x01 << 2)
#define EN_DAT0EN		(0x01 << 3)

/* REG_SDICR */
#define CO_EN			(0x01)
#define RI_EN			(0x01 << 1)
#define DI_EN			(0x01 << 2)
#define DO_EN			(0x01 << 3)
#define R2_EN			(0x01 << 4)
#define CLK74_OE		(0x01 << 5)
#define CLK8_OE			(0x01 << 6)
#define CLK_KEEP		(0x01 << 7)
#define SD_CS			(0x01 << 15)

#endif /*  __ASM_ARCH_REGS_I2C_H */
