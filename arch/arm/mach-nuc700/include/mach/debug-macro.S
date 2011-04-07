/*
 * Copyright (C) 2008 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <mach/hardware.h>

	.macro	addruart, rp, rv
	ldr	\rp, =UART_BA
	ldr	\rv, =UART_BA
	.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
