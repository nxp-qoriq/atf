/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
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
#error not implemented
#elif defined(CONFIG_DDR_NODIMM)
#if CONFIG_DDR_NODIMM == 1
/*
 * PCB Revision 1.0: 9 x K4A8G085WC-BCWE TODO
 */
#error Not implemented.
#endif /* CONFIG_DDR_NODIMM == 1 */

#if CONFIG_DDR_NODIMM == 2 || CONFIG_DDR_NODIMM == 3
/*
 * PCB Revision 1.1/1.2: 2 x 5 x MT40A1G16TB-062E IT:F
 */
#define CONFIG_DDR_NODIMM_CH2

#if CONFIG_DDR_NODIMM == 3
/* PCB Revision 1.2 supports ECC */
#define CONFIG_DDR_NODIMM_ECC
#endif /* CONFIG_DDR_NODIMM == 3 */

static const struct dimm_params static_dimm = {
	.mpart = "Fixed DDR Config " __XSTRING(CONFIG_DDR_NODIMM),
	.n_ranks = 1,
	.die_density = 0x6, // encoded per spd byte 4, 0b110 = 16Gbit
	.rank_density = 0x200000000, // 16Gbit
	.capacity = 0x200000000, // 16Gbit
	.primary_sdram_width = 64,
#ifdef CONFIG_DDR_NODIMM_ECC
	.ec_sdram_width = 8, // 8 bit ecc extension
#else
	.ec_sdram_width = 0, // no ecc extension
#endif
	.rdimm = 0,
	.package_3ds = 0,
	.device_width = 16, // 16 bit per sdram
	.rc = 0,

	.n_row_addr = 17,
	.n_col_addr = 10,
#ifdef CONFIG_DDR_NODIMM_ECC
	.edc_config = 2, // enable ecc
#else
	.edc_config = 0, // disable ecc
#endif
	.bank_addr_bits = 0, // 4 banks
	.bank_group_bits = 1, // 2 bank groups
	.burst_lengths_bitmask = 0xc, // enable 4 & 8-bit burst (DDR4 spec)

	.mirrored_dimm = 0,

	// timings based on MT40A4G4 / MT40A2G8 / MT40A1G16 datasheet (DDR4-3200 22-22-22)
	.mtb_ps = 125, // MTB per SPD spec
	.ftb_10th_ps = 10, // default value, unused by nxp ddr driver
	.taa_ps = 13750, // min. 13.75ns
	.tfaw_ps = 30000, // min: max(30ns or 28CK) (this 8Gbit sdram has 2KB pages)

	.tckmin_x_ps = 625, // 2400 (CK=1600)
	.tckmax_ps = 1250, // 1600 (CK=800)

	.caslat_x = 0b00000001011111111111110000000000, // CL = [10-22,24] (1 << CL)

	.trcd_ps = 13750, // 13.75ns
	.trp_ps = 13750, // 13.75ns
	.tras_ps = 32000, // 32ns

	.trfc1_ps = 350000, // 350ns,
	.trfc2_ps = 260000, // 260ns
	.trfc4_ps = 160000, // 160ns
	.trrds_ps = 5300, // min: max(4CK or 5.3ns)
	.trrdl_ps = 6400, // min: max(4CK or 6.4ns)
	.tccdl_ps = 5000, // min: max(4CK or 5ns)
	.trfc_slr_ps = 0,

	.trc_ps = 45750, // tras + trp 45.75ns
	.twr_ps = 15000, // 15ns

	.refresh_rate_ps = 7800000, // 1x mode 7.8us for standard temperature range (TODO: pick correct range based on temperature?!)
	// .extended_op_srt = 0,

	// .rcw = {}, // only for registered dimm
	.dq_mapping = {
		0x00,
		0x20,
		0x00,
		0x20,
		0x00,
		0x20,
		0x00,
		0x20,
		0x00,
		0x20,
		0x00,
		0x20,
		0x00,
		0x20,
		0x00,
		0x20,
		0x00,
		0x20,
	},
	.dq_mapping_ors = 1,
};
#endif /* CONFIG_DDR_NODIMM == 2 || CONFIG_DDR_NODIMM == 3 */

int ddr_get_ddr_params(struct dimm_params *pdimm,
		       struct ddr_conf *conf)
{
	// channel 1
	conf->dimm_in_use[0] = 1;
	memcpy(&pdimm[0], &static_dimm, sizeof(struct dimm_params));

#if defined(CONFIG_DDR_NODIMM_CH2)
	// channel 2
	conf->dimm_in_use[1] = 1; // enable (module on) channel 2
	memcpy(&pdimm[1], &static_dimm, sizeof(struct dimm_params));

	/* 2 modules */
	return 0x3;
#else
	/* 1 module */
	return 0x1;
#endif /* defined(CONFIG_DDR_NODIMM_CH2) */
}
#endif /* defined(CONFIG_DDR_NODIMM) */

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;
	const struct ddr_conf *conf = &priv->conf;

	popts->vref_dimm = U(0x24);		/* range 1, 83.4% */
	popts->rtt_override = 0U;
	popts->rtt_park = U(240);
	popts->otf_burst_chop_en = 0;
	popts->burst_length = U(DDR_BL8);
	popts->trwt_override = 0U;
	popts->bstopre = 0U;			/* auto precharge */
	popts->addr_hash = 1;
	popts->caslat_override = 0; // TODO: why is this set by default?!
	popts->caslat_override_value = 0; // TODO: why is this set by default?!
	popts->auto_self_refresh_en = 1;
	popts->output_driver_impedance = 0; // 34 Ohm
	popts->twot_en = 0;
	popts->threet_en = 0;
	popts->addt_lat_override = 0; // TODO: why is this set by default?!
	popts->addt_lat_override_value = 0; // TODO: why is this set by default?!
	popts->phy_atx_impedance = 30;
	popts->skip2d = 0;

	/* Set ODT impedance on PHY side */
	switch (conf->cs_on_dimm[1]) {
	case 0xc:	/* Two slots dual rank */
	case 0x4:	/* Two slots single rank, not valid for interleaving */
		popts->trwt = U(0xf);
		popts->twrt = U(0x7);
		popts->trrt = U(0x7);
		popts->twwt = U(0x7);
		popts->vref_phy = U(0x6B);	/* 83.6% */
		popts->odt = U(60);
		popts->phy_tx_impedance = U(28);
		break;
	case 0:		/* One slot used */
	default:
		popts->trwt = U(0x3);
		popts->twrt = U(0x3);
		popts->trrt = U(0x3);
		popts->twwt = U(0x3);
		popts->vref_phy = U(0x60);	/* 75% */
		popts->odt = U(48);
		popts->phy_tx_impedance = U(28);
		break;
	}

	return 0;
}

long long init_ddr(void)
{
	int spd_addr[] = { 0x51, 0x00, 0x53, 0x00 };
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys) != 0) {
		ERROR("System clocks are not set\n");
		panic();
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
	if (info.clk == 0) {
		info.clk = get_ddr_freq(&sys, 1);
	}
	info.dimm_on_ctlr = DDRC_NUM_DIMM;

	info.warm_boot_flag = DDR_WRM_BOOT_NT_SUPPORTED;

	dram_size = dram_init(&info
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
		    , NXP_CCN_HN_F_0_ADDR
#endif
		    );


	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

	return dram_size;
}
