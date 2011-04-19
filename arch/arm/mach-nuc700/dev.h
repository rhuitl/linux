/*
 * arch/arm/mach-nuc700/dev.h
 *
 * Copyright (c) 2011 Nuvoton technology corporation
 * All rights reserved.
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

#define DEFINE_DEV(_name, _ctrlbit)			\
struct clk clk_##_name = {				\
		.enable	= nuc900_clk_enable,		\
		.cken	= (1 << _ctrlbit),		\
	}
