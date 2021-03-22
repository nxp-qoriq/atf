/*
 * Copyright 2018-2019 NXP
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
#include <ddr.h>
#include <utils.h>
#include <utils_def.h>
#include <errata.h>

#ifdef CONFIG_STATIC_DDR
const struct ddr_cfg_regs static_2100 = {
	.cs[0].config = 0x80040322,
	.cs[0].bnds = 0x1FF,
	.cs[1].config = 0x80000322,
	.cs[1].bnds = 0x1FF,
	.sdram_cfg[0] = 0xE5004000,
	.sdram_cfg[1] = 0x401151,
	.timing_cfg[0] = 0xD1770018,
	.timing_cfg[1] = 0xF2FC9245,
	.timing_cfg[2] = 0x594197,
	.timing_cfg[3] = 0x2101100,
	.timing_cfg[4] = 0x220002,
	.timing_cfg[5] = 0x5401400,
	.timing_cfg[7] = 0x26600000,
	.timing_cfg[8] = 0x5446A00,
	.dq_map[0] = 0x32C57554,
	.dq_map[1] = 0xD4BB0BD4,
	.dq_map[2] = 0x2EC2F554,
	.dq_map[3] = 0xD95D4001,
	.sdram_mode[0] = 0x3010631,
	.sdram_mode[1] = 0x100200,
	.sdram_mode[9] = 0x8400000,
	.sdram_mode[8] = 0x500,
	.sdram_mode[2] = 0x10631,
	.sdram_mode[3] = 0x100200,
	.sdram_mode[10] = 0x400,
	.sdram_mode[11] = 0x8400000,
	.sdram_mode[4] = 0x10631,
	.sdram_mode[5] = 0x100200,
	.sdram_mode[12] = 0x400,
	.sdram_mode[13] = 0x8400000,
	.sdram_mode[6] = 0x10631,
	.sdram_mode[7] = 0x100200,
	.sdram_mode[14] = 0x400,
	.sdram_mode[15] = 0x8400000,
	.interval = 0x1FFE07FF,
	.zq_cntl = 0x8A090705,
	.clk_cntl = 0x2000000,
	.cdr[0] = 0x80040000,
	.cdr[1] = 0xC1,
	.wrlvl_cntl[0] = 0x86750609,
	.wrlvl_cntl[1] = 0xA0B0C0D,
	.wrlvl_cntl[2] = 0xF10110E,
};

const struct ddr_cfg_regs static_1800 = {
	.cs[0].config = 0x80040322,
	.cs[0].bnds = 0x1FF,
	.cs[1].config = 0x80000322,
	.cs[1].bnds = 0x1FF,
	.sdram_cfg[0] = 0xE5004000,
	.sdram_cfg[1] = 0x401151,
	.timing_cfg[0] = 0x91660018,
	.timing_cfg[1] = 0xDDD82045,
	.timing_cfg[2] = 0x512153,
	.timing_cfg[3] = 0x10E1100,
	.timing_cfg[4] = 0x220002,
	.timing_cfg[5] = 0x4401400,
	.timing_cfg[7] = 0x14400000,
	.timing_cfg[8] = 0x3335900,
	.dq_map[0] = 0x32C57554,
	.dq_map[1] = 0xD4BB0BD4,
	.dq_map[2] = 0x2EC2F554,
	.dq_map[3] = 0xD95D4001,
	.sdram_mode[0] = 0x3010421,
	.sdram_mode[1] = 0x80200,
	.sdram_mode[9] = 0x4400000,
	.sdram_mode[8] = 0x500,
	.sdram_mode[2] = 0x10421,
	.sdram_mode[3] = 0x80200,
	.sdram_mode[10] = 0x400,
	.sdram_mode[11] = 0x4400000,
	.sdram_mode[4] = 0x10421,
	.sdram_mode[5] = 0x80200,
	.sdram_mode[12] = 0x400,
	.sdram_mode[13] = 0x4400000,
	.sdram_mode[6] = 0x10421,
	.sdram_mode[7] = 0x80200,
	.sdram_mode[14] = 0x400,
	.sdram_mode[15] = 0x4400000,
	.interval = 0x1B6C06DB,
	.zq_cntl = 0x8A090705,
	.clk_cntl = 0x2000000,
	.cdr[0] = 0x80040000,
	.cdr[1] = 0xC1,
	.wrlvl_cntl[0] = 0x86750607,
	.wrlvl_cntl[1] = 0x8090A0B,
	.wrlvl_cntl[2] = 0xD0E0F0C,
};

const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = 0x80040322,
	.cs[0].bnds = 0x1FF,
	.cs[1].config = 0x80000322,
	.cs[1].bnds = 0x1FF,
	.sdram_cfg[0] = 0xE5004000,
	.sdram_cfg[1] = 0x401151,
	.sdram_cfg[2] = 0x0,
	.timing_cfg[0] = 0x91550018,
	.timing_cfg[1] = 0xBAB48E44,
	.timing_cfg[2] = 0x490111,
	.timing_cfg[3] = 0x10C1000,
	.timing_cfg[4] = 0x220002,
	.timing_cfg[5] = 0x3401400,
	.timing_cfg[6] = 0x0,
	.timing_cfg[7] = 0x13300000,
	.timing_cfg[8] = 0x1224800,
	.timing_cfg[9] = 0x0,
	.dq_map[0] = 0x32C57554,
	.dq_map[1] = 0xD4BB0BD4,
	.dq_map[2] = 0x2EC2F554,
	.dq_map[3] = 0xD95D4001,
	.sdram_mode[0] = 0x3010211,
	.sdram_mode[1] = 0x0,
	.sdram_mode[9] = 0x400000,
	.sdram_mode[8] = 0x500,
	.sdram_mode[2] = 0x10211,
	.sdram_mode[3] = 0x0,
	.sdram_mode[10] = 0x400,
	.sdram_mode[11] = 0x400000,
	.sdram_mode[4] = 0x10211,
	.sdram_mode[5] = 0x0,
	.sdram_mode[12] = 0x400,
	.sdram_mode[13] = 0x400000,
	.sdram_mode[6] = 0x10211,
	.sdram_mode[7] = 0x0,
	.sdram_mode[14] = 0x400,
	.sdram_mode[15] = 0x400000,
	.interval = 0x18600618,
	.zq_cntl = 0x8A090705,
	.ddr_sr_cntr = 0x0,
	.clk_cntl = 0x2000000,
	.cdr[0] = 0x80040000,
	.cdr[1] = 0xC1,
	.wrlvl_cntl[0] = 0x86750607,
	.wrlvl_cntl[1] = 0x8090A0B,
	.wrlvl_cntl[2] = 0xD0E0F0C,
};

struct static_table {
	unsigned long rate;
	const struct ddr_cfg_regs *regs;
};

const struct static_table table[] = {
	{1600, &static_1600},
	{1800, &static_1800},
	{2100, &static_2100},
};

long long board_static_ddr(struct ddr_info *priv)
{
	const unsigned long clk = priv->clk / 1000000;
	long long size = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(table); i++) {
		if (table[i].rate >= clk)
			break;
	}
	if (i < ARRAY_SIZE(table)) {
		VERBOSE("Found static setting for rate %ld\n", table[i].rate);
		memcpy(&priv->ddr_reg, table[i].regs,
		       sizeof(struct ddr_cfg_regs));
		size = 0x200000000UL;
	} else {
		ERROR("Not static settings for rate %ld\n", clk);
	}

	return size;
}
#else
static const struct rc_timing rce[] = {
	{1600, 8, 7},
	{1867, 8, 7},
	{2134, 8, 9},
	{}
};

static const struct board_timing udimm[] = {
	{0x04, rce, 0x01020304, 0x06070805},
	{0x0f, rce, 0x01020304, 0x06070805},
};

int ddr_board_options(struct ddr_info *priv)
{
	int ret;
	struct memctl_opt *popts = &priv->opt;

	if (popts->rdimm) {
		debug("RDIMM parameters not set.\n");
		return -EINVAL;
	}

	ret = cal_board_params(priv, udimm, ARRAY_SIZE(udimm));
	if (ret)
		return ret;

	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0x0;	/* 32 clocks */
	popts->cpo_sample = 0x61;
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN	|
			  DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm)	|
			  DDR_CDR2_VREF_TRAIN_EN		|
			  DDR_CDR2_VREF_RANGE_2;
	popts->bstopre = 0;

	return 0;
}
#endif

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
