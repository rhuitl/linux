/*
 * arch/arm/mach-nuc700/include/mach/mfp.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * Based on arch/arm/mach-s3c2410/include/mach/map.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#ifndef __ASM_ARCH_MFP_H
#define __ASM_ARCH_MFP_H

extern void nuc700_mfp_config(unsigned long *mfp_cfgs, int num);

#define GPIO_CFG0_DEFAULT	0x00000000
#define GPIO_CFG1_DEFAULT	0x00000000
#define GPIO_CFG2_DEFAULT	0x00000000
#define GPIO_CFG3_DEFAULT	0x00005555
#define GPIO_CFG4_DEFAULT	0x00155555
#define GPIO_CFG5_DEFAULT	0x00000000
#define GPIO_CFG6_DEFAULT	0x00000000

#define FN0	0x00
#define FN1	0x01
#define FN2	0x02
#define FN3	0x03

#define GPIO_PT0_OFFSET	0x00
#define GPIO_PT1_OFFSET	0x10
#define GPIO_PT2_OFFSET	0x20
#define GPIO_PT3_OFFSET	0x30
#define GPIO_PT4_OFFSET	0x40
#define GPIO_PT5_OFFSET	0x50
#define GPIO_PT6_OFFSET	0x60

/* 0-7: save fn; 8-15: save x; 16-23: save offset */

#define CFG(x, fn, offset)	((fn) | ((x) << 8) | ((offset) << 16))

#define ARRAY_AND_SIZE(x)	(x), ARRAY_SIZE(x)

#define GET_GPIO_CFG(x)		((((x) >> 8)&0xff) << 1)
#define GET_GPIO_VAL(x)		((x)&0xff)
#define GET_GPIO_PT(x)		(((x) >> 16)&0xff)

#endif /* __ASM_ARCH_MFP_H */

