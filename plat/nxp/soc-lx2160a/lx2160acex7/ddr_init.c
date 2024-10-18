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

#if defined(CONFIG_STATIC_DDR) || defined(CONFIG_DDR_NODIMM)
#error not implemented
#endif /* defined(CONFIG_STATIC_DDR) || defined(CONFIG_DDR_NODIMM) */

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;
	const struct ddr_conf *conf = &priv->conf;

	popts->vref_dimm = U(0x24);		/* range 1, 83.4% */
	popts->rtt_override = 0U;
	popts->rtt_park = U(240);
	popts->otf_burst_chop_en = 0;
	popts->burst_length = U(DDR_BL8);
	popts->trwt_override = 1U;
	popts->bstopre = 0U;			/* auto precharge */
	popts->addr_hash = 1;

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
	int spd_addr[] = { 0x51, 0x52, 0x53, 0x54 };
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
