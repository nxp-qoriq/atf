/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <platform_def.h>
#include <pl011.h>
#include <console.h>
#include <plat_common.h>
#include <utils.h>
#include <debug.h>

/*
 * Perform arm specific early platform setup. At this moment we only initialize
 * the console and the memory layout.
 */
void plat_console_init(void)
{
	static console_pl011_t console;
	struct sysinfo sys;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);

	console_pl011_register(NXP_CONSOLE_ADDR,
			       (sys.freq_platform/NXP_UART_CLK_DIVIDER),
			       NXP_CONSOLE_BAUDRATE, &console);
}
