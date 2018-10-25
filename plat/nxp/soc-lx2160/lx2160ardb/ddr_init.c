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
#include <utils.h>
#include <string.h>
#include <ddr.h>

#ifdef CONFIG_DDR_NODIMM
/*
 * Sample code to bypass reading SPD. This is a sample, not recommended
 * for boards with slots. DDR model number: UDIMM MT18ASF1G72AZ-2G6B1.
 */
struct dimm_params ddr_raw_timing = {
	.n_ranks = 2,
	.rank_density = 4294967296u,
	.capacity = 8589934592u,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.device_width = 8,
	.die_density = 0x4,
	.rdimm = 0,
	.mirrored_dimm = 1,
	.n_row_addr = 15,
	.n_col_addr = 10,
	.bank_addr_bits = 0,
	.bank_group_bits = 2,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 750,
	.tckmax_ps = 1600,
	.caslat_x = 0x00FFFC00,
	.taa_ps = 13750,
	.trcd_ps = 13750,
	.trp_ps = 13750,
	.tras_ps = 32000,
	.trc_ps = 457500,
	.twr_ps = 15000,
	.trfc1_ps = 260000,
	.trfc2_ps = 160000,
	.trfc4_ps = 110000,
	.tfaw_ps = 21000,
	.trrds_ps = 3000,
	.trrdl_ps = 4900,
	.tccdl_ps = 5000,
	.refresh_rate_ps = 7800000,
};

int ddr_get_ddr_params(struct dimm_params *pdimm,
			    struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;	/* Modify accordingly */
	memcpy(pdimm, &ddr_raw_timing, sizeof(struct dimm_params));
	memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);

	/* valid DIMM mask, change accordingly, together with dimm_on_ctlr. */
	return 0x5;
}
#endif

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;

	popts->vref_dimm = 0x24;		/* range 1, 83.4% */
	popts->vref_phy = 0x61;
	popts->rtt_override = 0;
	popts->rtt_park = 120;
	popts->otf_burst_chop_en = 0;
	popts->burst_length = DDR_BL8;
	popts->trwt_override = 1;
	popts->trwt = 0x3;
	popts->twrt = 0x3;
	popts->trrt = 0x3;
	popts->twwt = 0x3;
	popts->ap_en = 0;

	return 0;
}

long long _init_ddr(void)
{
	int spd_addr[] = { 0x51, 0x52, 0x53, 0x54 };
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(info));

	/* Set two DDRC. Unused DDRC will be removed automatically. */
	info.num_ctlrs = 2;
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;
	info.ddr[1] = (void *)NXP_DDR2_ADDR;
	info.phy[0] = (void *)NXP_DDR_PHY1_ADDR;
	info.phy[1] = (void *)NXP_DDR_PHY2_ADDR;
	info.clk = get_ddr_freq(&sys, 0);
	if (!info.clk)
		info.clk = get_ddr_freq(&sys, 1);
	info.dimm_on_ctlr = 2;

	dram_size = dram_init(&info);

	if (dram_size < 0)
		ERROR("DDR init failed.\n");

	return dram_size;
}
