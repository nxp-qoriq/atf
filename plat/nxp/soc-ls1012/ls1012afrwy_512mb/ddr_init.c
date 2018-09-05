/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author York Sun <york.sun@nxp.com>
 */

#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <fsl_mmdc.h>

long long _init_ddr(void)
{
	static const struct fsl_mmdc_info mparam = {
		.mdctl = 0x04180000,
		.mdpdc = 0x00030035,
		.mdotc = 0x12554000,
		.mdcfg0 = 0xbabf7954,
		.mdcfg1 = 0xdb328f64,
		.mdcfg2 = 0x01ff00db,
		.mdmisc = 0x00001680,
		.mdref = 0x0f3c8000,
		.mdrwd = 0x00002000,
		.mdor = 0x00bf1023,
		.mdasp = 0x0000003f,
		.mpodtctrl = 0x0000022a,
		.mpzqhwctrl = 0xa1390003,
	};

	mmdc_init(&mparam);
	NOTICE("DDR Init Done\n");

	/* Need to see there is any other way to
	 * deduce the total DRAM size rather
	 * hard-coding it.
	 */
	return NXP_DRAM0_SIZE;
}
