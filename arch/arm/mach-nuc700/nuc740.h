/*
 * arch/arm/mach-nuc700/nuc740.h
 *
 * Copyright (c) 2010 Nuvoton corporation
 *
 * Header file for NUC900 CPU support
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

struct map_desc;
struct sys_timer;

/* core initialisation functions */

extern void nuc700_init_irq(void);
extern struct sys_timer nuc700_timer;

/* extern file from nuc740.c */

extern void nuc740_board_init(void);
extern void nuc740_init_clocks(void);
extern void nuc740_read_id(void);
extern void nuc740_gpio_init(void);
