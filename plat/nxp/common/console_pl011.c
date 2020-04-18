/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <common/debug.h>
#include <dcfg.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <lib/utils.h>
#include <plat_common.h>
#include <platform_def.h>

/*
 * Perform arm specific early platform setup. At this moment we only initialize
 * the console and the memory layout.
 */
void plat_console_init(void)
{
	static console_t nxp_boot_console;
	struct sysinfo sys;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys, NXP_DCFG_ADDR, NXP_SYSCLK_FREQ,
			NXP_DDRCLK_FREQ, NXP_PLATFORM_CLK_DIVIDER)) {
		ERROR("System clocks are not set\n");
		assert(0);
	}

	console_pl011_register(NXP_CONSOLE_ADDR,
			       (sys.freq_platform/NXP_UART_CLK_DIVIDER),
			       NXP_CONSOLE_BAUDRATE, &nxp_boot_console);
}
