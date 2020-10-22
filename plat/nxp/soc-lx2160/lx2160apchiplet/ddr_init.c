/*
 * Copyright 2020 NXP
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

#ifdef CONFIG_STATIC_DDR
const struct ddr_cfg_regs static_2900 = {
	.cs[0].config = 0x80010422,
	.cs[0].bnds = 0x01FF,
	.sdram_cfg[0] = 0xe5248000,/*enable 2T*/
	.sdram_cfg[1] = 0x00401030,
	.timing_cfg[0] = 0xFF990016,
	.timing_cfg[1] = 0x676A5884,
	.timing_cfg[2] = 0x0005702C,
	.timing_cfg[3] = 0x13682100,
	.timing_cfg[4] = 0xD502,
	.timing_cfg[5] = 0x17401400,
	.timing_cfg[7] = 0x39960000,
	.timing_cfg[8] = 0x0944AB00,
	.sdram_mode[0] = 0x01010C50,
	.sdram_mode[9] = 0x10000000,
	.sdram_mode[8] = 0x0703,
	.sdram_mode[1] = 0x00280000,
	.sdram_mode[10] = 0x00,
	.sdram_mode[11] = 0x00,
	.sdram_mode[4] = 0x00,
	.sdram_mode[12] = 0x00,
	.sdram_mode[13] = 0x00,
	.sdram_mode[6] = 0x00,
	.sdram_mode[14] = 0x00,
	.sdram_mode[15] = 0x00,
	.interval = 0x2C2E0B0B,
	.data_init = 0xDEADBEEF,
	.zq_cntl = 0x8A090705,
	.wrlvl_cntl[0] = 0x00,
	.wrlvl_cntl[1] = 0x00,
	.wrlvl_cntl[2] = 0x00,
	.dq_map[0] = 0x2f50f420,
	.dq_map[1] = 0xd8bd96d8,
	.dq_map[2] = 0x12d0ed34,
	.dq_map[3] = 0xb98a8000,
	.debug[28] = 0x01080F70
};

const struct dimm_params static_dimm = {
	.rdimm = 0,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.n_ranks = 1,
	.device_width = 16,
	.mirrored_dimm = 0,
};

/* Sample code using two UDIMM MT18ASF1G72AZ-2G6B1, on each DDR controller */
long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_2900, sizeof(static_2900));
	memcpy(&priv->dimm, &static_dimm, sizeof(static_dimm));
	priv->conf.cs_on_dimm[0] = 0x1;
	ddr_board_options(priv);
	compute_ddr_phy(priv);

	return ULL(0x200000000);
}

#elif defined(CONFIG_DDR_NODIMM)
/* DDR model number: MT40A1G16KNR-062E:E */
struct dimm_params ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 8589934592u,
	.capacity = 8589934592u,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.device_width = 16,
	.die_density = 0x5,
	.rdimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 16,
	.n_col_addr = 10,
	.bank_group_bits = 2,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 625,
	.tckmax_ps = 1500,
	.caslat_x = 0x00FFFA00,
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
	.trrds_ps = 2500,
	.trrdl_ps = 4900,
	.tccdl_ps = 5000,
	.refresh_rate_ps = 7800000,
	.dq_mapping[0] = 0x0b,
	.dq_mapping[1] = 0x35,
	.dq_mapping[2] = 0x03,
	.dq_mapping[3] = 0x34,
	.dq_mapping[4] = 0x08,
	.dq_mapping[5] = 0x36,
	.dq_mapping[6] = 0x0b,
	.dq_mapping[7] = 0x36,
	.dq_mapping[10] = 0x16,
	.dq_mapping[11] = 0x36,
	.dq_mapping[12] = 0x04,
	.dq_mapping[13] = 0x2d,
	.dq_mapping[14] = 0x03,
	.dq_mapping[15] = 0x2d,
	.dq_mapping[16] = 0x0d,
	.dq_mapping[17] = 0x2e,
	.dq_mapping[8] = 0x18,
	.dq_mapping[9] = 0x2a,
	.dq_mapping_ors = 0,
};

int ddr_get_ddr_params(struct dimm_params *pdimm,
			    struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;	/* Modify accordingly */
	memcpy(pdimm, &ddr_raw_timing, sizeof(struct dimm_params));
	memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);

	/* valid DIMM mask, change accordingly, together with dimm_on_ctlr. */
	return 0x1;
}
#endif	/* CONFIG_DDR_NODIMM */

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;

	popts->vref_dimm = 0x0F;
	popts->rtt_override = 0;
	popts->rtt_park = 240;
	popts->otf_burst_chop_en = 0;
	popts->burst_length = DDR_BL8;
	popts->trwt_override = 1;
	popts->bstopre = 0;			/* auto precharge */
	popts->addr_hash = 1;

	popts->trwt = 0xF;
	popts->twrt = 0x7;
	popts->trrt = 0x7;
	popts->twwt = 0x7;
	popts->vref_phy = 0x59;
	popts->odt = 80;
	popts->phy_tx_impedance = 36;

	popts->twot_en = 1;	/* enable 2T timing */

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
