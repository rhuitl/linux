/*
 * arch/arm/mach-nuc700/nuc710.h
 *
 * Copyright (c) 2011 Nuvoton corporation
 *
 * Header file for NUC700 CPU support
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "cpu.h"

struct map_desc;
struct sys_timer;

/* core initialisation functions */

extern void nuc700_init_irq(void);
extern struct sys_timer nuc700_timer;

/* extern file from nuc710.c */

extern void nuc710_board_init(void);
extern void nuc710_init_clocks(void);
extern void nuc710_read_id(void);
extern void nuc710_gpio_init(void);
