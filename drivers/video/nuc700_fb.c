/* 
 * Copyright (c) 2011 Nuvoton technology corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *  Author:
 *    Wan ZongShun <mcuos.com@gmail.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/clk.h>

#include <mach/map.h>
#include <mach/nuc700_fb.h>
#include <mach/regs-lcd.h>

#define nuc700_lcd_read(reg)		__raw_readl(reg)
#define nuc700_lcd_write(reg, val)	__raw_writel((val), (reg))
#define nuc700_writelut(entry, value) nuc700_lcd_write(REG_LUTADDR + ((entry) << 2),  (value))

struct nuc700_fbdrv {
	struct device		*dev;
	struct clk		*clk;
	struct resource *res;
	struct nuc700fb_platformdata *pdata;
	struct nuc700fb_display *current_display;
	void __iomem		*io;
	void __iomem		*irq_base;
	unsigned long		clk_rate;
	unsigned int		irqnum;
	/* keep these registers in case we need to re-write palette */
	u32			palette_buffer[256];
	u32			pseudo_pal[16];
};

/*
 *	Check the video params of 'var'.
 */
static int nuc700fb_check_var(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	struct nuc700_fbdrv *fbdrv = info->par;
	struct nuc700fb_display *default_display = fbdrv->pdata->displays +
						   fbdrv->pdata->default_display;

	dev_dbg(fbdrv->dev, "check_var(var=%p, info=%p)\n", var, info);

	/* validate x/y resolution */
	/* choose default mode if possible */
	if (var->xres != default_display->xres ||
	    var->yres != default_display->yres ||
	    var->bits_per_pixel != default_display->bpp) 
	    	fbdrv->current_display = NULL;
	else
		 fbdrv->current_display = default_display;

	if (!fbdrv->current_display) {
		printk(KERN_ERR "wrong resolution or depth %dx%d at %d bit per pixel\n",
			var->xres, var->yres, var->bits_per_pixel);
		return -EINVAL;
	}

	/* it should be the same size as the display */
	var->xres_virtual	= fbdrv->current_display->xres;
	var->yres_virtual	= fbdrv->current_display->yres;
	var->height		= fbdrv->current_display->height;
	var->width		= fbdrv->current_display->width;

	var->transp.offset	= 0;
	var->transp.length	= 0;

	/* set R/G/B possions */
	switch (var->bits_per_pixel) {
	case 1:
	case 2:
	case 4:
	case 8:
	default:
		var->red.offset 	= 0;
		var->red.length 	= var->bits_per_pixel;
		var->green 		= var->red;
		var->blue		= var->red;
		break;
	case 12:
		var->red.length		= 4;
		var->green.length	= 4;
		var->blue.length	= 4;
		var->red.offset		= 8;
		var->green.offset	= 4;
		var->blue.offset	= 0;
		break;
	case 16:
		var->red.length		= 5;
		var->green.length	= 6;
		var->blue.length	= 5;
		var->red.offset		= 11;
		var->green.offset	= 5;
		var->blue.offset	= 0;
		break;
	case 18:
		var->red.length		= 6;
		var->green.length	= 6;
		var->blue.length	= 6;
		var->red.offset		= 12;
		var->green.offset	= 6;
		var->blue.offset	= 0;
		break;
	case 32:
		var->red.length		= 8;
		var->green.length	= 8;
		var->blue.length	= 8;
		var->red.offset		= 16;
		var->green.offset	= 8;
		var->blue.offset	= 0;
		break;
	}

	return 0;
}

static void nuc700_set_count(unsigned int bpp, unsigned int width, unsigned int height) 
{

	switch (bpp) {
	case 32:
	case 24:
		/* FIFO1 transfer data count register */
		nuc700_lcd_write(REG_F1DREQCNT, (width << 16) | height);
		/* FIFO 1 real column count register */
		nuc700_lcd_write(REG_FIFO1RELACOLCNT, width);
		break;
	case 16:
		 /* FIFO1 transfer data count register */
		nuc700_lcd_write(REG_F1DREQCNT, ((width << 16) >> 1) | height);  
		 /* FIFO 1 real column count register */
		nuc700_lcd_write(REG_FIFO1RELACOLCNT, width >> 1);
		break;
	case 8:
		/* FIFO1 transfer data count register */
		nuc700_lcd_write(REG_F1DREQCNT, ((width << 16) >> 2) | height);   
		/* FIFO 1 real column count register */
		nuc700_lcd_write(REG_FIFO1RELACOLCNT, width >> 2);
		break;
	case 4:
		/* FIFO1 transfer data count register */
		nuc700_lcd_write(REG_F1DREQCNT, ((width << 16) >> 3) | height); 
		/* FIFO 1 real column count register */
		nuc700_lcd_write(REG_FIFO1RELACOLCNT, width >> 3);
		 /* Setting palette */ 
		nuc700_lcd_write(REG_LUTENTY2, 0x07060504);
		nuc700_lcd_write(REG_LUTENTY3, 0x0B0A0908);
		nuc700_lcd_write(REG_LUTENTY4, 0x0F0E0D0C);	
		break;
	case 2:
		/* FIFO1 transfer data count register */
		nuc700_lcd_write(REG_F1DREQCNT, ((width << 16) >> 4) | height); 
		/* FIFO 1 real column count register */
		nuc700_lcd_write(REG_FIFO1RELACOLCNT, width >> 4);
		/* Setting palette */ 
		nuc700_lcd_write(REG_LUTENTY1, 0x03020100); 
		break;
	case 1:
		 /* FIFO1 transfer data count register */
		nuc700_lcd_write(REG_F1DREQCNT, ((width << 16) >> 5) | height); 
		 /* FIFO 1 real column count register */
		nuc700_lcd_write(REG_FIFO1RELACOLCNT, width >> 5);
		 /* Setting palette */   
		nuc700_lcd_write(REG_LUTENTY1, 0x00000100);
		break;
	default:
		break;
	}

}

/*
 *	Activate (set) the controller from the given framebuffer
 *	information
 */
static void nuc700fb_activate_var(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;

	nuc700_set_count(var->bits_per_pixel, var->xres, var->yres);
	/* set lcd address pointers */
	nuc700_lcd_write(REG_F1SADDR, info->fix.smem_start); 

}

/*
 *      Alters the hardware state.
 *
 */
static int nuc700fb_set_par(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;

	switch (var->bits_per_pixel) {
	case 32:
	case 24:
	case 18:
	case 16:
	case 12:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;
	case 1:
		info->fix.visual = FB_VISUAL_MONO01;
		break;
	default:
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	}

	info->fix.line_length = (var->xres_virtual * var->bits_per_pixel) / 8;

	/* activate this new configuration */
	nuc700fb_activate_var(info);
	return 0;
}

static void nuc700fb_setpalettereg(u_int regno, u_int red, u_int green, u_int blue,
			u_int trans, struct fb_info *info)
{
	struct nuc700_fbdrv *fbdrv = info->par;
	u_int val;
		
	if (regno < 256) {
		val = ((red & 0xff00) << 8) |
			((green & 0xff00) >> 0) |
			((blue & 0xff00) >> 8);
			
		fbdrv->palette_buffer[regno] = val;
            
		/* Configure TFT Look-up Table */
		nuc700_lcd_write(REG_LCDCON, (nuc700_lcd_read(REG_LCDCON) | LCD_LCDCON_LUTWREN) & ~LCD_LCDCON_VDLUTEN);		
		/* Configure entries of look-up table */
		nuc700_writelut(regno, val);
		/* Configure TFT Look-up Table */
		nuc700_lcd_write(REG_LCDCON, (nuc700_lcd_read(REG_LCDCON) | LCD_LCDCON_VDLUTEN) & ~LCD_LCDCON_LUTWREN);
    } 

}


static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int nuc700fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;

    if (info->var.grayscale) 
	red = green = blue = (19595 * red + 38470 * green + 7471 * blue) >> 16;

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseuo-palette */
		if (regno < 16) {
			u32 *pal = info->pseudo_palette;

			val  = chan_to_field(red, &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue, &info->var.blue);
			pal[regno] = val;
		}
		break;
	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
	   	nuc700fb_setpalettereg(regno, red, green, blue, transp, info);
	    break;
	default:
		return 1;   /* unknown type */
	}
	return 0;
}

/**
 *      nuc700fb_blank
 *
 */
static int nuc700fb_blank(int blank_mode, struct fb_info *info)
{
	return 0;
}

static struct fb_ops nuc700fb_ops = {
	.owner			= THIS_MODULE,
	.fb_check_var		= nuc700fb_check_var,
	.fb_set_par		= nuc700fb_set_par,
	.fb_blank		= nuc700fb_blank,
	.fb_setcolreg		= nuc700fb_setcolreg,
	.fb_fillrect		= cfb_fillrect,
	.fb_copyarea		= cfb_copyarea,
	.fb_imageblit		= cfb_imageblit,
};

void enable_nuc700_lcd(void)
{
	/* Configure LCD FIFOs controller register and tigger the engine */
	nuc700_lcd_write(REG_FIFOCON, LCD_FIFOCON_FIFOEN_FIFO1); 
	/* LCD Controller Enable */
	nuc700_lcd_write(REG_LCDCON, nuc700_lcd_read(REG_LCDCON) | LCD_LCDCON_LCDCEN);

}

static void nuc700fb_init_specific_lcd(struct fb_info *info, int bursty) 
{
	struct nuc700_fbdrv *fbdrv = info->par;
	struct nuc700fb_display *display = fbdrv->current_display;

	if (strcmp(display->lcdtype, "tft") == 0) {
		#ifdef CONFIG_NUC700_LCD_TFT_CASIO   
		 	nuc700_lcd_write(REG_DISPWINS, 0x00020008); 
			nuc700_lcd_write(REG_DISPWINE, (((display->height + 2) << 16) | (display->width + 8)));

			/* Configure LCD Timing Generation */
			nuc700_lcd_write(REG_LCDTCON1, 0x0070051A);  
			nuc700_lcd_write(REG_LCDTCON2, 0x01E800F2);  
			nuc700_lcd_write(REG_LCDTCON3, 0x00100000);  
			nuc700_lcd_write(REG_LCDTCON4, 0x00060101);  
			nuc700_lcd_write(REG_LCDTCON5, 0x00000006);
		
		#else

			#ifdef CONFIG_NUC700_LCD_TFT_MTV335
				/* Configure Valid Display Window Starting Coordinate */
				nuc700_lcd_write(REG_DISPWINS, (0x11 << 16) | 0x11);  
				 /* Configure Valid Display Window Ending Coordinate */
	    			nuc700_lcd_write(REG_DISPWINE, ((display->height + 0x11) << 16) | (0x11 + display->width * 2));

				nuc700_lcd_write(REG_LCDTCON1, 0x11400401);  
	    			nuc700_lcd_write(REG_LCDTCON2, 0x05EE0101);  
				nuc700_lcd_write(REG_LCDTCON3, 0x00100402); 
				nuc700_lcd_write(REG_LCDTCON4, 0x00000102); 
				nuc700_lcd_write(REG_LCDTCON5, 0x0000000C);

				nuc700_lcd_write(REG_LCDCON, nuc700_lcd_read(REG_LCDCON) | LCD_LCDCON_TVEN); 
			#else
				/* Configure Valid Display Window Starting Coordinate */
	    			nuc700_lcd_write(REG_DISPWINS, 0x00000000);       
	   			 /* Configure Valid Display Window Ending Coordinate */
	    			nuc700_lcd_write(REG_DISPWINE, ((display->height << 16) | display->width));
			#endif
		#endif

		#ifdef CONFIG_NUC700_LCD_TFT_AUO960240
			nuc700_lcd_write(REG_LCDTCON1, 0x00900209);
		    nuc700_lcd_write(REG_LCDTCON2, 0x014000F0);  
		    nuc700_lcd_write(REG_LCDTCON3, 0x00200802);  
		    nuc700_lcd_write(REG_LCDTCON4, 0x000A0101);  
		    nuc700_lcd_write(REG_LCDTCON5, 0x0000000E); 
		#endif

		#ifdef CONFIG_NUC700_LCD_TFT_TOPPOLY240320
			nuc700_lcd_write(REG_LCDTCON1, 0x00904C09);
			nuc700_lcd_write(REG_LCDTCON2, 0x00F10140);  
			nuc700_lcd_write(REG_LCDTCON3, 0x00200802);  
			nuc700_lcd_write(REG_LCDTCON4, 0x000C0101);  
			nuc700_lcd_write(REG_LCDTCON5, 0x0000000E);  
		#endif 
	}else if (strcmp(display->lcdtype, "stn") == 0) {

  	  /* Configure LCD Timing Generation */
    		nuc700_lcd_write(REG_LCDTCON1, 0x00F00401);  
    		nuc700_lcd_write(REG_LCDTCON2, 0x014200F0);  
   		nuc700_lcd_write(REG_LCDTCON3, 0x00000000);  
    		nuc700_lcd_write(REG_LCDTCON4, 0x000A0103);  
		nuc700_lcd_write(REG_LCDTCON5, 0x00000000);  
		nuc700_lcd_write(REG_LCDTCON6, 0x00000000);      

		/* Configure TMED Dithering Pattern */
		nuc700_lcd_write(REG_TMDDITHP1, 0x01010001);        
		nuc700_lcd_write(REG_TMDDITHP2, 0x030380C0);      
		nuc700_lcd_write(REG_TMDDITHP3, 0xE0E0C0E0);      
		nuc700_lcd_write(REG_TMDDITHP4, 0xF0F0E0F0);   
		nuc700_lcd_write(REG_TMDDITHP5, 0xF8F8F8F0);       
		nuc700_lcd_write(REG_TMDDITHP6, 0xFCFCFCF8);      
		nuc700_lcd_write(REG_TMDDITHP7, 0xFFFCFFF8);    
	}

	/* Make sure the TRANSBURSTY can be divided by 8(data burst) * 1(word) */
	if(((display->width / bursty) % 16) == 0) {
		/* Configure LCD FIFO1 transfer parameters */
    		nuc700_lcd_write(REG_FIFO1PRM, (LCD_FIFO1PRM_F1BURSTY_16DATABURST | LCD_FIFO1PRM_F1TRANSZ_4BYTE));    
	} else if(((display->width / bursty) % 8) == 0) {	
    		/* Configure LCD FIFO1 transfer parameters */
    		nuc700_lcd_write(REG_FIFO1PRM, (LCD_FIFO1PRM_F1BURSTY_08DATABURST | LCD_FIFO1PRM_F1TRANSZ_4BYTE));    
	} else {
    		/* Select another transfer type */
    		/* Configure LCD FIFO1 transfer parameters */
    		nuc700_lcd_write(REG_FIFO1PRM, (LCD_FIFO1PRM_F1BURSTY_04DATABURST | LCD_FIFO1PRM_F1TRANSZ_4BYTE));    
	}

	/* Configure LCD Interrupt Enable Register */
	nuc700_lcd_write(REG_LCDINTENB, LCD_LCDINTENB_VSEN); 
    
	/* Configure LCD Interrupt Clear Register */
	nuc700_lcd_write(REG_LCDINTC, LCD_LCDINTC_VSIC);

	nuc700_set_count(display->bpp, display->width, display->height);
	
	enable_nuc700_lcd();

}

/*
 * Initialise LCD-related registers
 */
static int nuc700fb_init_registers(struct fb_info *info)
{
	struct nuc700_fbdrv *fbdrv = info->par;
	struct nuc700fb_display *display = fbdrv->current_display;
	unsigned int val = 0x0;
	int bursty = 1;

	if (strcmp(display->lcdtype, "tft") == 0) {
		val |= LCDTYPE_TFT;
		if (display->tfttype) {
			val |= TFTTYPE_SYNCT;
			if (display->pixelseq == 0x0) {
				val &= ~(0x03 << 18);
				val |= PIXSEQ_R1G1B2R2G3R3;
			} else if (display->pixelseq == 0x01) {
				val &= ~(0x03 << 18);
				val |= PIXSEQ_R1G2B3R4G5B6;
			} else if (display->pixelseq == 0x02) {
				val &= ~(0x03 << 18);
				val |= PIXSEQ_R1G1B1R2G2B2;
			}

			if (display->rgbseq == 0x0) {
				val &= ~(0x03 << 10);
				val |= LCD_LCDCON_RGBSEQ_RGB;
			} else if (display->rgbseq == 0x1) {
				val &= ~(0x03 << 10);
				val |= LCD_LCDCON_RGBSEQ_BGR;
			} else if (display->rgbseq == 0x2) {
				val &= ~(0x03 << 10);
				val |= LCD_LCDCON_RGBSEQ_GBR;
			} else if  (display->rgbseq == 0x3) {
				val &= ~(0x03 << 10);
				val |= LCD_LCDCON_RGBSEQ_RBG;
			} 
		} else {
			val &= ~TFTTYPE_SYNCT;
		}

		if (display->lcdbus == 0x0) {
			val &= ~(0x03 << 8);
			val |= LCD_BUS_24BIT;
		} else if (display->lcdbus == 0x01) {
			val &= ~(0x03 << 8);
			val |= LCD_BUS_18BIT;
		} else if (display->lcdbus == 0x02) {
			val &= ~(0x03 << 8);
			val |= LCD_BUS_8BIT;
		}
	} else if (strcmp(display->lcdtype, "stn") == 0)
		val &= (~LCDTYPE_TFT);

	switch (display->bpp) {
		val &= (~0x07);
	case 32:
	case 24:
		val |= LCD_LCDCON_BPP_24;
		break;
	case 18:
		val |= LCD_LCDCON_BPP_18;
		break;
	case 16:
		val |= LCD_LCDCON_BPP_16;
		break;
	case 12:
		val |= LCD_LCDCON_BPP_12;
		bursty = 1 << 1;
		break;
	case 8:
		val |= LCD_LCDCON_BPP_08;
		val |= LCD_LCDCON_VDLUTEN;
		bursty = 1 << 2;
		break;
	case 4:
		val |= LCD_LCDCON_BPP_04;
		val |= LCD_LCDCON_VDLUTEN;
		bursty = 1 << 3;
		break;
	case 2:
		val |= LCD_LCDCON_BPP_02;
		val |= LCD_LCDCON_VDLUTEN;
		bursty = 1 << 4;
		break;
	case 1:
		val |= LCD_LCDCON_BPP_01;
		val |= LCD_LCDCON_VDLUTEN;
		bursty = 1 << 5;
		break;
	default:
		break;
	}

	nuc700_lcd_write(REG_LCDCON, val);
	/* Configure Dummy Display Color Pattern Register */
	nuc700_lcd_write(REG_DDISPCP, 0x00000000); 
    
	/* Configure Valid Display Window Starting Coordinate */
	nuc700_lcd_write(REG_DISPWINS, 0x00000000);    

	/* Configure Valid Display Window Ending Coordinate */
	nuc700_lcd_write(REG_DISPWINE, 0x00000000);  

	nuc700fb_init_specific_lcd(info, bursty);
	
	return 0;
}


/*
 *    Alloc the SDRAM region of NUC700 for the frame buffer.
 *    The buffer should be a non-cached, non-buffered, memory region
 *    to allow palette and pixel writes without flushing the cache.
 */
static int __init nuc700fb_map_video_memory(struct fb_info *info)
{
	struct nuc700_fbdrv *fbdrv = info->par;
	dma_addr_t map_dma;
	unsigned long map_size = PAGE_ALIGN(info->fix.smem_len);

	dev_dbg(fbdrv->dev, "nuc700fb_map_video_memory(fbdrv=%p) map_size %lu\n",
		fbdrv, map_size);

	info->screen_base = dma_alloc_writecombine(fbdrv->dev, map_size,
							&map_dma, GFP_KERNEL);

	if (!info->screen_base)
		return -ENOMEM;

	memset(info->screen_base, 0x00, map_size);
	info->fix.smem_start = map_dma;

	return 0;
}

static inline void nuc700fb_unmap_video_memory(struct fb_info *info)
{
	struct nuc700_fbdrv *fbdrv = info->par;
	dma_free_writecombine(fbdrv->dev, PAGE_ALIGN(info->fix.smem_len),
			      info->screen_base, info->fix.smem_start);
}

static irqreturn_t nuc700fb_irq(int irq, void *dev_id)
{
	unsigned int interruptStatus;    
 
	interruptStatus = nuc700_lcd_read(REG_LCDINTS);
	if (interruptStatus & LCD_LCDINTS_VSIS) {
		 nuc700_lcd_write(REG_LCDINTC, ~interruptStatus | LCD_LCDINTC_VSIC);
	}
	return IRQ_HANDLED;
}

static char driver_name[] = "nuc700fb";

static int __devinit nuc700fb_probe(struct platform_device *pdev)
{
	struct nuc700fb_platformdata *pdata = pdev->dev.platform_data;
	struct nuc700_fbdrv *fbdrv;
	struct fb_info	   *fbinfo;
	unsigned long smem_len;
	int ret = 0;
	
	if (!pdata) {
		dev_err(&pdev->dev,
			"no platform data for lcd, cannot attach\n");
		return -EINVAL;
	}

	if (pdata->default_display > pdata->num_displays) {
		dev_err(&pdev->dev,
			"default display No. is %d but only %d displays \n",
			pdata->default_display, pdata->num_displays);
		return -EINVAL;
	}

	fbinfo = framebuffer_alloc(sizeof(struct nuc700_fbdrv), &pdev->dev);
	if (!fbinfo)
		return -ENOMEM;

	fbdrv = fbinfo->par;
	fbdrv->dev = &pdev->dev;
	fbdrv->pdata = pdata;
	fbdrv->current_display = fbdrv->pdata->displays + fbdrv->pdata->default_display;

	fbdrv->irqnum = platform_get_irq(pdev, 0);
	if (fbdrv->irqnum < 0) {
		dev_err(&pdev->dev, "no irq for device\n");
		return -ENOENT;
	}

	platform_set_drvdata(pdev, fbinfo);

	fbdrv->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!(request_mem_region(fbdrv->res->start, resource_size(fbdrv->res), pdev->name))) {
		dev_err(&pdev->dev, "failed to alloc memory region\n");
		ret = -ENOENT;
		goto free_fb;
	}

	fbdrv->io = ioremap(fbdrv->res->start, resource_size(fbdrv->res));
	if (!fbdrv->io) {
		dev_err(&pdev->dev, "ioremap() of lcd registers failed\n");
		ret = -ENXIO;
		goto release_region;
	}

	/* fill the fbinfo*/
	strcpy(fbinfo->fix.id, driver_name);
	fbinfo->fix.type		= FB_TYPE_PACKED_PIXELS;
	fbinfo->fix.type_aux		= 0;
	fbinfo->fix.xpanstep		= 0;
	fbinfo->fix.ypanstep		= 0;
	fbinfo->fix.ywrapstep		= 0;
	fbinfo->fix.accel		= FB_ACCEL_NONE;
	fbinfo->var.nonstd		= 0;
	fbinfo->var.activate		= FB_ACTIVATE_NOW;
	fbinfo->var.accel_flags		= 0;
	fbinfo->var.vmode		= FB_VMODE_NONINTERLACED;
	fbinfo->fbops			= &nuc700fb_ops;
	fbinfo->flags			= FBINFO_FLAG_DEFAULT;
	fbinfo->pseudo_palette		= &fbdrv->pseudo_pal;

	ret = request_irq(fbdrv->irqnum, nuc700fb_irq, IRQF_DISABLED,
			  pdev->name, fbinfo);
	if (ret) {
		dev_err(&pdev->dev, "cannot register irq handler %d -err %d\n",
			fbdrv->irqnum, ret);
		ret = -EBUSY;
		goto release_regs;
	}

	fbdrv->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(fbdrv->clk)) {
		printk(KERN_ERR "nuc700-lcd:failed to get lcd clock source\n");
		ret = PTR_ERR(fbdrv->clk);
		goto release_irq;
	}

	clk_enable(fbdrv->clk);
	dev_dbg(&pdev->dev, "got and enabled clock\n");

	/* calutate the video buffer size */
	smem_len = fbdrv->current_display->xres;
	smem_len *= fbdrv->current_display->yres;
	smem_len *= fbdrv->current_display->bpp;
	smem_len >>= 3;
	fbinfo->fix.smem_len = smem_len;

	/* Initialize Video Memory */
	ret = nuc700fb_map_video_memory(fbinfo);
	if (ret) {
		printk(KERN_ERR "Failed to allocate video RAM: %x\n", ret);
		goto release_clock;
	}

	dev_dbg(&pdev->dev, "got video memory\n");

	fbinfo->var.xres = fbdrv->current_display->xres;
	fbinfo->var.yres = fbdrv->current_display->yres;
	fbinfo->var.bits_per_pixel = fbdrv->current_display->bpp;

	nuc700fb_init_registers(fbinfo);

	nuc700fb_check_var(&fbinfo->var, fbinfo);

	ret = register_framebuffer(fbinfo);
	if (ret) {
		printk(KERN_ERR "failed to register framebuffer device: %d\n",
			ret);
		goto free_video_memory;
	}

	printk(KERN_INFO "fb%d: %s frame buffer device\n",
		fbinfo->node, fbinfo->fix.id);

	return 0;

free_video_memory:
	nuc700fb_unmap_video_memory(fbinfo);
release_clock:
	clk_disable(fbdrv->clk);
	clk_put(fbdrv->clk);
release_irq:
	free_irq(fbdrv->irqnum, fbdrv);
release_regs:
	iounmap(fbdrv->io);
release_region:
	release_mem_region(fbdrv->res->start, resource_size(fbdrv->res));
free_fb:
	framebuffer_release(fbinfo);
	return ret;
}

/*
 * shutdown the lcd controller
 */
static void nuc700fb_stop_lcd(struct fb_info *info)
{
	/* Disable FIFO */
	nuc700_lcd_write(REG_FIFOCON, nuc700_lcd_read(REG_FIFOCON) & ~(LCD_FIFOCON_FIFOEN_FIFO1 | LCD_FIFOCON_FIFOEN_FIFO2)); 
	/* Disable LCD Controller */
	nuc700_lcd_write(REG_LCDCON, nuc700_lcd_read(REG_LCDCON) & ~LCD_LCDCON_LCDCEN);
	/* Reset LCD Controller */
	nuc700_lcd_write(REG_LCDCON, LCD_LCDCON_LCDRST);
}

/*
 *  Cleanup
 */
static int nuc700fb_remove(struct platform_device *pdev)
{
	struct fb_info *fbinfo = platform_get_drvdata(pdev);
	struct nuc700_fbdrv *fbdrv = fbinfo->par;

	nuc700fb_stop_lcd(fbinfo);
	msleep(1);

	unregister_framebuffer(fbinfo);
	nuc700fb_unmap_video_memory(fbinfo);
	iounmap(fbdrv->io);
	free_irq(fbdrv->irqnum, fbdrv);
	release_mem_region(fbdrv->res->start, resource_size(fbdrv->res));
	platform_set_drvdata(pdev, NULL);
	framebuffer_release(fbinfo);

	return 0;
}

static struct platform_driver nuc700fb_driver = {
	.probe		= nuc700fb_probe,
	.remove		= nuc700fb_remove,
	.driver		= {
		.name	= "nuc700-lcd",
		.owner	= THIS_MODULE,
	},
};

int __devinit nuc700fb_init(void)
{
	return platform_driver_register(&nuc700fb_driver);
}

static void __exit nuc700fb_cleanup(void)
{
	platform_driver_unregister(&nuc700fb_driver);
}

module_init(nuc700fb_init);
module_exit(nuc700fb_cleanup);

MODULE_AUTHOR("Wan ZongShun <mcuos.com@gmail.com>");
MODULE_DESCRIPTION("Framebuffer driver for the NUC700");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:nuc700-lcd");
