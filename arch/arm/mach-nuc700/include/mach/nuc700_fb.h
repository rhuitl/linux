/* linux/arch/arm/mach-nuc700/inlcude/mach/nuc700_fb.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * author: Wan Zongshun <mcuos.com@gmail.com>
 */

#ifndef __ASM_ARM_NUC700_FB_H
#define __ASM_ARM_NUC700_FB_H

/* LCD Display Description */
struct nuc700fb_display {
	/* LCD Screen Size */
	unsigned short width;
	unsigned short height;

	/* LCD Screen Info */
	unsigned short xres;
	unsigned short yres;
	unsigned short bpp;

	char *lcdtype;
	unsigned char tfttype; 
	char lcdbus;
	char rgbseq; 
	char pixelseq;
};

struct nuc700fb_platformdata {
	struct nuc700fb_display *displays;
	unsigned num_displays;
	unsigned default_display;
};

#endif /* __ASM_ARM_NUC700_FB_H */
