/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Jiafei Pan <Jiafei.Pan@nxp.com>
 *
 */

#ifndef __PLAT_MACROS_S__
#define __PLAT_MACROS_S__

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant GIC and CCI registers
	 * whenever an unhandled exception is taken in
	 * BL31.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	.endm

#endif /* __PLAT_MACROS_S__ */
