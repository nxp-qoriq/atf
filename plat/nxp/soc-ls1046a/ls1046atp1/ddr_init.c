/*
 * Copyright 2020,2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <ddr.h>
#include <utils.h>
#include <utils_def.h>
#include <errata.h>
#include <common/debug.h>

static const struct rc_timing rce[] = {
/*	{2134, 8, 7},	// Modified by Leo for TP1 LAB1*/
	{2134, 10, 5},
	{}
};

static const struct board_timing udimm[] = {
/*	{0x1f, rce, 0x01010101, 0x01010101},	//Modified by Leo for TP1 LAB1*/
	{0x1f, rce, 0x01010204, 0x04050503},
};

/* DDR model number: MT40A512M8HX-093E */
struct dimm_params ddr_raw_timing = {
	.n_ranks = 1,
/*	.rank_density = 4294967296u,
	.capacity = 4294967296u,	// ORG setting, Modified by Leo for TP1 LAB1*/
	.rank_density = 8589934592u,
	.capacity = 8589934592u,
	.primary_sdram_width = 64,
	.ec_sdram_width = 16,
	.rdimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 17,
	.n_col_addr = 10,
	.bank_group_bits = 1,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 800,
	.tckmax_ps = 2000,
	.caslat_x = 0x0001FFE00,
	.taa_ps = 13500,
	.trcd_ps = 13500,
	.trp_ps = 13500,
	.tras_ps = 33000,
	.trc_ps = 46500,
	.twr_ps = 15000,
	.trfc1_ps = 350000,
	.trfc2_ps = 260000,
	.trfc4_ps = 160000,
	.tfaw_ps = 30000,
	.trrds_ps = 5300,
	.trrdl_ps = 6400,
	.tccdl_ps = 5355,
	.refresh_rate_ps = 7800000,
	.rc = 0x1f,
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

#if 0
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0x0;	/* 32 clocks */
	popts->cpo_sample = 0x61;
	popts->ddr_cdr1 = 0x80080000;
	popts->ddr_cdr2 = 0x80;
#else

	popts->bstopre = 0;
	popts->half_strength_drive_en = 1;
	popts->cpo_sample = 0x46;
//	popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
//		DDR_CDR1_ODT(DDR_CDR_ODT_50ohm);
//		Removed by Leo for test
	popts->ddr_cdr1 = 0x80080000;
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_50ohm) |
		DDR_CDR2_VREF_TRAIN_EN;
	popts->output_driver_impedance = 1;
#endif
	return 0;
}

long long init_ddr(void)
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
