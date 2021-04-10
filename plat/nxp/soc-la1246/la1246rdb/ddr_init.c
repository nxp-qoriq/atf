/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Authors:	Parth Girishkumar Bera <parthgirishkumar.bera@nxp.com>
 *		Pankit Garg <pankit.garg@nxp.com>
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

static const struct rc_timing rce[] = {
	{1600, 8, 10},
	{1867, 8, 10},
	{2134, 8, 10},
	{}
};

static const struct board_timing udimm[] = {
	{0x03, rce, 0x0, 0x0},
};

struct dimm_params ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 8589934592u,
	.capacity = 8589934592u,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.rdimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 16,
	.n_col_addr = 10,
	.bank_group_bits = 2,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 750,
	.tckmax_ps = 1900,
	.caslat_x = 0x000FFFE00,
	.taa_ps = 13750,
	.trcd_ps = 13750,
	.trp_ps = 13750,
	.tras_ps = 32000,
	.trc_ps = 45750,
	.twr_ps = 15000,
	.trfc1_ps = 350000,
	.trfc2_ps = 260000,
	.trfc4_ps = 160000,
	.tfaw_ps = 21000,
	.trrds_ps = 3000,
	.trrdl_ps = 5156,
	.tccdl_ps = 5000,
	.refresh_rate_ps = 7800000,
	.dq_mapping[0] = 0x0c,
	.dq_mapping[1] = 0x2b,
	.dq_mapping[2] = 0x2d,
	.dq_mapping[3] = 0x04,
	.dq_mapping[4] = 0x16,
	.dq_mapping[5] = 0x35,
	.dq_mapping[6] = 0x23,
	.dq_mapping[7] = 0x0d,
	.dq_mapping[8] = 0x36,
	.dq_mapping[9] = 0x0c,
	.dq_mapping[10] = 0x2c,
	.dq_mapping[11] = 0x0b,
	.dq_mapping[12] = 0x03,
	.dq_mapping[13] = 0x24,
	.dq_mapping[14] = 0x35,
	.dq_mapping[15] = 0x0c,
	.dq_mapping[16] = 0x03,
	.dq_mapping[17] = 0x2d,
	.dq_mapping_ors = 0,
	.rc = 0x03,
};

int ddr_get_ddr_params(struct dimm_params *pdimm,
		       struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;
	memcpy(pdimm, &ddr_raw_timing, sizeof(struct dimm_params));
	memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);

	return 1;
}

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
	popts->ddr_cdr1 = 0x80080000;
	popts->ddr_cdr2 = 0x80;
	return 0;
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
