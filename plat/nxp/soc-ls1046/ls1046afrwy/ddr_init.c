/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
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
#include <ddr.h>
#include <utils.h>
#include <utils_def.h>
#include <errata.h>

const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = 0x80010412,
	.cs[0].bnds = 0x7F,
	.sdram_cfg[0] = 0xE50C0008,
	.sdram_cfg[1] = 0x00401010,
	.sdram_cfg[2] = 0x1,
	.timing_cfg[0] = 0xFA550018,
	.timing_cfg[1] = 0xBAB40C52,
	.timing_cfg[2] = 0x0048C11C,
	.timing_cfg[3] = 0x01111000,
	.timing_cfg[4] = 0x00000002,
	.timing_cfg[5] = 0x03401400,
	.timing_cfg[6] = 0x0,
	.timing_cfg[7] = 0x23300000,
	.timing_cfg[8] = 0x02116600,
	.timing_cfg[9] = 0x0,
	.dq_map[0] = 0x0,
	.dq_map[1] = 0x0,
	.dq_map[2] = 0x0,
	.dq_map[3] = 0x0,
	.sdram_mode[0] = 0x01010210,
	.sdram_mode[1] = 0x0,
	.sdram_mode[8] = 0x00000500,
	.sdram_mode[9] = 0x04000000,
	.interval = 0x18600618,
	.zq_cntl = 0x8A090705,
	.ddr_sr_cntr = 0x0,
	.clk_cntl = 0x2000000,
	.cdr[0] = 0x80040000,
	.cdr[1] = 0xC1,
	.wrlvl_cntl[0] = 0x86550607,
	.wrlvl_cntl[1] = 0x07070708,
	.wrlvl_cntl[2] = 0x0808088,
};

long long board_static_ddr(struct ddr_info *priv)
{
        memcpy(&priv->ddr_reg, &static_1600, sizeof(static_1600));

	return 0x80000000ULL;
}

long long _init_ddr(void)
{
	int spd_addr[] = { NXP_SPD_EEPROM0 };
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(struct ddr_info));
	info.num_ctlrs = 1;
	info.dimm_on_ctlr = 1;
	info.clk = get_ddr_freq(&sys, 0);
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);

	if (dram_size < 0)
		ERROR("DDR init failed.\n");

	erratum_a008850_post();

	return dram_size;
}
