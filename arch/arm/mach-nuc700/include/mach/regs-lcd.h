/*
 * arch/arm/mach-nuc700/include/mach/regs-ps2.h
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

#ifndef __ASM_ARCH_REGS_LCD_H
#define __ASM_ARCH_REGS_LCD_H

/* LCD Controller */
#define REG_LCDCON           (LCD_BA+0x0000)

/* LCD Interrupt Control */
#define REG_LCDINTENB        (LCD_BA+0x0004)
#define REG_LCDINTS          (LCD_BA+0x0008)
#define REG_LCDINTC          (LCD_BA+0x000C)
                            
/* LCD Pre-processing */    
#define REG_OSDUPSCF         (LCD_BA+0x0010)
#define REG_VDUPSCF          (LCD_BA+0x0014)
#define REG_OSDNSCF          (LCD_BA+0x0018)
#define REG_VDDNSCF          (LCD_BA+0x001C)
                            
/* LCD FIFO Control */      
#define REG_FIFOCON          (LCD_BA+0x0020)
#define REG_FIFOSTATUS       (LCD_BA+0x0024)
#define REG_FIFO1PRM         (LCD_BA+0x0028)
#define REG_FIFO2PRM         (LCD_BA+0x002C)
#define REG_F1SADDR          (LCD_BA+0x0030)
#define REG_F2SADDR          (LCD_BA+0x0034)
#define REG_F1DREQCNT        (LCD_BA+0x0038)
#define REG_F2DREQCNT        (LCD_BA+0x003C)
#define REG_F1CURADR         (LCD_BA+0x0040)
#define REG_F2CURADR         (LCD_BA+0x0044)
#define REG_FIFO1RELACOLCNT  (LCD_BA+0x0048)
#define REG_FIFO2RELACOLCNT  (LCD_BA+0x004C)

/* Color Generation */
#define REG_LUTENTY1         (LCD_BA+0x0050)
#define REG_LUTENTY2         (LCD_BA+0x0054)
#define REG_LUTENTY3         (LCD_BA+0x0058)
#define REG_LUTENTY4         (LCD_BA+0x005C)

#define REG_OSDLUTENTY1      (LCD_BA+0x0060)
#define REG_OSDLUTENTY2      (LCD_BA+0x0064)
#define REG_OSDLUTENTY3      (LCD_BA+0x0068)
#define REG_OSDLUTENTY4      (LCD_BA+0x006C)
#define REG_TMDDITHP1        (LCD_BA+0x0070)
#define REG_TMDDITHP2        (LCD_BA+0x0074)
#define REG_TMDDITHP3        (LCD_BA+0x0078)
#define REG_TMDDITHP4        (LCD_BA+0x007C)
#define REG_TMDDITHP5        (LCD_BA+0x0080)
#define REG_TMDDITHP6        (LCD_BA+0x0084)
#define REG_TMDDITHP7        (LCD_BA+0x0088)
                             
/* LCD Post-processing */    
#define REG_DDISPCP          (LCD_BA+0x0090)
#define REG_DISPWINS         (LCD_BA+0x0094)
#define REG_DISPWINE         (LCD_BA+0x0098)
#define REG_OSDWINS          (LCD_BA+0x009C)
#define REG_OSDWINE          (LCD_BA+0x00A0)
#define REG_OSDOVCN          (LCD_BA+0x00A4)
#define REG_OSDKYP           (LCD_BA+0x00A8)
#define REG_OSDKYM           (LCD_BA+0x00AC)

/* LCD Timing Generation */
#define REG_LCDTCON1         (LCD_BA+0x00B0)
#define REG_LCDTCON2         (LCD_BA+0x00B4)
#define REG_LCDTCON3         (LCD_BA+0x00B8)
#define REG_LCDTCON4         (LCD_BA+0x00BC)
#define REG_LCDTCON5         (LCD_BA+0x00C0)
#define REG_LCDTCON6         (LCD_BA+0x00C4)
                             
/* Look Up Table SRAM */     
#define REG_LUTADDR          (LCD_BA+0x0100)

/* NUC700 LCD register Bits Definition */

/* LCD Controller */

#define LCDTYPE_TFT		(1 << 16)
#define TFTTYPE_SYNCT		(1 << 17)

#define LCD_BUS_24BIT		(0x0 << 8)
#define LCD_BUS_18BIT		(0x1 << 8)
#define LCD_BUS_8BIT		(0x2 << 8)

#define PIXSEQ_R1G1B2R2G3R3	(0x0 << 18)
#define PIXSEQ_R1G2B3R4G5B6	(0x1 << 18)
#define PIXSEQ_R1G1B1R2G2B2	(0x2 << 18)

#define LCD_LCDCON_RGBSEQ_RGB	(0x0 << 10)
#define LCD_LCDCON_RGBSEQ_BGR	(0x1 << 10)
#define LCD_LCDCON_RGBSEQ_GBR	(0x2 << 10)    
#define LCD_LCDCON_RGBSEQ_RBG	(0x3 << 10)

#define LCD_LCDCON_BPP_01	(0x00 << 0)
#define LCD_LCDCON_BPP_02	(0x01 << 0)
#define LCD_LCDCON_BPP_04	(0x02 << 0)
#define LCD_LCDCON_BPP_08	(0x03 << 0)
#define LCD_LCDCON_BPP_12	(0x04 << 0)
#define LCD_LCDCON_BPP_16	(0x05 << 0)
#define LCD_LCDCON_BPP_18	(0x06 << 0)
#define LCD_LCDCON_BPP_24	(0x07 << 0)

#define LCD_LCDCON_VDLUTEN	(0x01 << 3)
#define LCD_LCDCON_TVEN		0x00100000

#define LCD_FIFO1PRM_F1BURSTY_04DATABURST	0x00000000
#define LCD_FIFO1PRM_F1BURSTY_08DATABURST	0x00000004 
#define LCD_FIFO1PRM_F1BURSTY_16DATABURST	0x00000008 

#define LCD_FIFO1PRM_F1TRANSZ_1BYTE		0x0
#define LCD_FIFO1PRM_F1TRANSZ_2BYTE		0x1
#define LCD_FIFO1PRM_F1TRANSZ_4BYTE		0x2

#define LCD_LCDINTENB_VSEN			0x00000010
#define LCD_LCDINTC_VSIC			0x00000010
#define LCD_LCDINTS_VSIS			0x00000010

#define LCD_LCDCON_LCDCEN			0x01000000
#define LCD_FIFOCON_FIFOEN_FIFO1		0x00000001
#define LCD_FIFOCON_FIFOEN_FIFO2		0x00000002
#define LCD_LCDCON_LCDRST			0x10000000 
#define LCD_LCDCON_LUTWREN			0x04000000
#endif /*  __ASM_ARCH_REGS_LCD_H */
