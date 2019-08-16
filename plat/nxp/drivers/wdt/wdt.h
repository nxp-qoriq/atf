/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Alison Wang <alison.wang@nxp.com>
 *
 */

#ifndef __WDT_H__
#define __WDT_H__

#define WDT_TIMEOUT	128000	/* ms */

#define	WCR_WDZST	0x01
#define	WCR_WDBG	0x02
#define	WCR_WDE		0x04
#define	WCR_WDT		0x08
#define	WCR_SRS		0x10
#define	WCR_WDA		0x20
#define	SET_WCR_WT(x)	(x << 8)
#define	WCR_WT_MSK	SET_WCR_WT(0xFF)

struct watchdog_regs {
	uint32_t wcr;   /* Control */
	uint32_t wsr;   /* Service */
	uint32_t wrsr;  /* Reset Status */
};

#endif /* __WDT_H__ */
