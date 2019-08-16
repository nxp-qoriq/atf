/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Alison Wang <alison.wang@nxp.com>
 *
 */

#ifndef __SP805_WDT_H__
#define __SP805_WDT_H__

#define WDT_TIMEOUT		0xFFFFFFFC

#define INT_ENABLE		BIT(0)
#define RESET_ENABLE		BIT(1)
#define DISABLE			0
#define UNLOCK			0x1ACCE551
#define LOCK			0x00000001
#define INT_MASK		BIT(0)

struct watchdog_regs {
	uint32_t wdtload;
	uint32_t wdtvalue;
	uint32_t wdtcontrol;
	uint32_t wdtintclr;
	uint32_t wdtris;
	uint32_t wdtmis;
	uint32_t temp[762];
	uint32_t wdtlock;
};

#endif /* __SP805_WDT_H__ */
