/*
 * Copyright 2021,2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Udit Agarwal <udit.agarwal@nxp.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>
#include <load_img.h>

#include "plat_common.h"
#include <platform_def.h>

#ifdef CONFIG_STATIC_DDR

const struct ddr_cfg_regs static_2600 = {
	.cs[0].bnds = 0x03FF,
	.cs[0].config = 0x80040422,
	.cs[1].bnds = 0x020003FF,
	.cs[1].config = 0x80000422,
	.timing_cfg[0] = 0xFF880018,
	.timing_cfg[1] = 0x2A220444,
	.timing_cfg[2] = 0x007151DC,
	.timing_cfg[3] = 0x125C2100,
	.timing_cfg[4] = 0xD502,
	.timing_cfg[5] = 0x05401400,
	.timing_cfg[7] = 0x28850000,
	.timing_cfg[8] = 0x06337A00,
	.sdram_cfg[0] = 0x65044000,
	.sdram_cfg[1] = 0x00401131,
	.sdram_mode[0] = 0x01010A40,
	.sdram_mode[1] = 0x00200400,
	.sdram_mode[2] = 0x01010A40,
	.sdram_mode[3] = 0x00200400,
	.sdram_mode[4] = 0x00,
	.sdram_mode[5] = 0x00,
	.sdram_mode[6] = 0x00,
	.sdram_mode[7] = 0x00,
	.sdram_mode[8] = 0x0702,
	.sdram_mode[9] = 0x0C140000,
	.sdram_mode[10] = 0x0602,
	.sdram_mode[11] = 0x0C000000,
	.sdram_mode[12] = 0x00,
	.sdram_mode[13] = 0x00,
	.sdram_mode[14] = 0x00,
	.sdram_mode[15] = 0x00,
	.md_cntl = 0x00,
	.interval = 0x279C09E7,
	.data_init = 0xDEADBEEF,
	.init_addr = 0x00,
	.zq_cntl = 0x8A090705,
	.sdram_rcw[0] = 0x00,
	.sdram_rcw[1] = 0x00,
	.sdram_rcw[2] = 0x00,
	.sdram_rcw[3] = 0x00,
	.sdram_rcw[4] = 0x00,
	.sdram_rcw[5] = 0x00,
	.err_disable = 0x00,
	.err_int_en = 0x1D
};

const struct ddr_cfg_regs static_2400 = {
	.cs[0].bnds = 0x03FF,
	.cs[0].config = 0x80040422,
	.cs[1].bnds = 0x020003FF,
	.cs[1].config = 0x80000422,
	.timing_cfg[0] = 0xFF880018,
	.timing_cfg[1] = 0x1710D244,
	.timing_cfg[2] = 0x006151DA,
	.timing_cfg[3] = 0x12592100,
	.timing_cfg[4] = 0xD502,
	.timing_cfg[5] = 0x06401400,
	.timing_cfg[7] = 0x27750000,
	.timing_cfg[8] = 0x06227A00,
	.sdram_cfg[0] = 0x65044000,
	.sdram_cfg[1] = 0x00401131,
	.sdram_mode[0] = 0x01010864,
	.sdram_mode[1] = 0x00180200,
	.sdram_mode[2] = 0x01010864,
	.sdram_mode[3] = 0x00180200,
	.sdram_mode[4] = 0x00,
	.sdram_mode[5] = 0x00,
	.sdram_mode[6] = 0x00,
	.sdram_mode[7] = 0x00,
	.sdram_mode[8] = 0x0702,
	.sdram_mode[9] = 0x08140000,
	.sdram_mode[10] = 0x0602,
	.sdram_mode[11] = 0x08000000,
	.sdram_mode[12] = 0x00,
	.sdram_mode[13] = 0x00,
	.sdram_mode[14] = 0x00,
	.sdram_mode[15] = 0x00,
	.md_cntl = 0x00,
	.interval = 0x24900924,
	.data_init = 0xDEADBEEF,
	.init_addr = 0x00,
	.zq_cntl = 0x8A090705,
	.sdram_rcw[0] = 0x00,
	.sdram_rcw[1] = 0x00,
	.sdram_rcw[2] = 0x00,
	.sdram_rcw[3] = 0x00,
	.sdram_rcw[4] = 0x00,
	.sdram_rcw[5] = 0x00,
	.err_disable = 0x00,
	.err_int_en = 0x1D
};


const struct dimm_params static_dimm = {
	.rdimm = 0,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.n_ranks = 2,
	.device_width = 8,
	.mirrored_dimm = 1,
};

long long board_static_ddr(struct ddr_info *priv)
{
	int valid_spd_mask __unused;
	int ret = 0x0;

	valid_spd_mask = 0x1;
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
	if (priv->num_ctlrs == 2 || priv->num_ctlrs == 1) {
		ret = disable_unused_ddrc(priv, valid_spd_mask,
				NXP_CCN_HN_F_0_ADDR);
		if (ret)
			return ret;
	}
#endif
	memcpy(&priv->ddr_reg, &static_2600, sizeof(static_2600));
	memcpy(&priv->dimm, &static_dimm, sizeof(static_dimm));
	priv->conf.cs_on_dimm[0] = 0x3;
	ddr_board_options(priv);
	compute_ddr_phy(priv);

	return ULL(0x400000000);
}

#elif defined(CONFIG_DDR_NODIMM)
struct dimm_params ddr_raw_timing = {
	.n_ranks = 2,
	.rank_density = 0x0000000200000000u,
	.capacity = 0x0000000400000000u,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.device_width = 8,
	.die_density = 0x5,
	.rdimm = 0,
	.mirrored_dimm = 1,
	.n_row_addr = 16,
	.n_col_addr = 10,
	.bank_addr_bits = 0,
	.bank_group_bits = 2,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 750,
	.tckmax_ps = 1625,
	.caslat_x = 0x01FFF800,
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
	return 0x1;
}
#endif	/* CONFIG_DDR_NODIMM */

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;
	const struct ddr_conf *conf = &priv->conf;

	popts->vref_dimm = 0x14;		/* range 1, 83.4% */
	popts->rtt_override = 1;
	popts->rtt_override_value = 0x5;        /* RTT being used as 60 ohm */
	popts->rtt_park = 240;
	popts->otf_burst_chop_en = 0;
	popts->burst_length = DDR_BL8;
	popts->trwt_override = 1;
	popts->bstopre = 0;			/* auto precharge */
	popts->addr_hash = 1;
/*
 * #if DDRC_NUM_DIMM != 2
 * #error This board has two DIMM slots per controller.
 * #endif
 */
	/* Set ODT impedance on PHY side */
	switch (conf->cs_on_dimm[1]) {
	case 0xc:	/* Two slots dual rank */
	case 0x4:	/* Two slots single rank, not valid for interleaving */
		popts->trwt = 0x0F;
		popts->twrt = 0x07;
		popts->trrt = 0x07;
		popts->twwt = 0x07;
		popts->vref_phy = 0x5E;	/* 83.6% */
		popts->odt = 80;
		popts->phy_tx_impedance = 36;
		break;
	case 0:		/* Ont slot used */
	default:
		popts->trwt = 0x0F;
		popts->twrt = 0x07;
		popts->trrt = 0x07;
		popts->twwt = 0x07;
		popts->vref_phy = 0x5E;	/* 75% */
		popts->odt = 80;
		popts->phy_tx_impedance = 36;
		break;
	}

	return 0;
}
#ifdef NXP_WARM_BOOT
long long init_ddr(uint32_t wrm_bt_flg)
#else
long long init_ddr(void)
#endif
{
	int spd_addr[] = { 0x51};
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys)) {
		ERROR("System clocks are not set\n");
		assert(0);
	}
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(info));

	/* Set two DDRC. Unused DDRC will be removed automatically. */
	info.num_ctlrs = NUM_OF_DDRC;
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;
	info.ddr[1] = (void *)NXP_DDR2_ADDR;
	info.phy[0] = (void *)NXP_DDR_PHY1_ADDR;
	info.phy[1] = (void *)NXP_DDR_PHY2_ADDR;
	info.clk = get_ddr_freq(&sys, 0);
	info.img_loadr = load_img;
	info.phy_gen2_fw_img_buf = PHY_GEN2_FW_IMAGE_BUFFER;
	if (!info.clk)
		info.clk = get_ddr_freq(&sys, 1);
	info.dimm_on_ctlr = DDRC_NUM_DIMM;

	info.warm_boot_flag = DDR_WRM_BOOT_NT_SUPPORTED;
#ifdef NXP_WARM_BOOT
	if (wrm_bt_flg) {
		info.warm_boot_flag = DDR_WARM_BOOT;
	} else if (wrm_bt_flg == 0x0) {
		info.warm_boot_flag = DDR_COLD_BOOT;
	}
#endif

	dram_size = dram_init(&info
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
			, NXP_CCN_HN_F_0_ADDR
#endif
			);

	if (dram_size < 0)
		ERROR("DDR init failed.\n");

	return dram_size;
}
