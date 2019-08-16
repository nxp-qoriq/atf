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
#include "sp805_wdt.h"

void wdt_reset(void)
{
	struct watchdog_regs *wdt = (struct watchdog_regs *)NXP_WDT1_BASE;

	mmio_write_32((uintptr_t)&wdt->wdtlock, UNLOCK);
	mmio_write_32((uintptr_t)&wdt->wdtintclr, INT_MASK);
	mmio_write_32((uintptr_t)&wdt->wdtlock, LOCK);
	mmio_read_32((uintptr_t)&wdt->wdtlock);
}

void wdt_init(void)
{
	struct watchdog_regs *wdt = (struct watchdog_regs *)NXP_WDT1_BASE;

	mmio_write_32((uintptr_t)&wdt->wdtlock, UNLOCK);
	mmio_write_32((uintptr_t)&wdt->wdtload, WDT_TIMEOUT);
	mmio_write_32((uintptr_t)&wdt->wdtintclr, INT_MASK);
	mmio_write_32((uintptr_t)&wdt->wdtcontrol, INT_ENABLE | RESET_ENABLE);
	mmio_write_32((uintptr_t)&wdt->wdtlock, LOCK);
	mmio_read_32((uintptr_t)&wdt->wdtlock);
}
