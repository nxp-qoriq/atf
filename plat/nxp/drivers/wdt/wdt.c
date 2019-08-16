/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Alison Wang <alison.wang@nxp.com>
 *
 */

#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <errno.h>
#include <io.h>
#include <delay_timer.h>
#include <mmio.h>
#include "wdt.h"

void wdt_reset(void)
{
	struct watchdog_regs *wdt = (struct watchdog_regs *)NXP_WDT1_BASE;

	mmio_write_16((uintptr_t)&wdt->wsr, 0x5555);
	mmio_write_16((uintptr_t)&wdt->wsr, 0xaaaa);
}

void wdt_init(void)
{
	struct watchdog_regs *wdt = (struct watchdog_regs *)NXP_WDT1_BASE;
	uint16_t timeout;

	timeout = (WDT_TIMEOUT / 500) - 1;
	mmio_write_16((uintptr_t)&wdt->wcr, (WCR_WDA | WCR_SRS | WCR_WDE)
		      << 8 | timeout);
}
