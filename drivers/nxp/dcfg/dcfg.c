/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>
#include "dcfg.h"
#include <lib/mmio.h>
#if defined (NXP_SFP_ENABLED)
#include <sfp.h>
#endif

static uint32_t porsr1;

uint32_t read_saved_porsr1(void)
{
	return porsr1;
}

/* Read the PORSR1 register */
uint32_t read_reg_porsr1(uintptr_t nxp_dcfg_addr)
{
	unsigned int *porsr1_addr = (void *)(nxp_dcfg_addr
						+ DCFG_PORSR1_OFFSET);

	porsr1 = gur_in32(porsr1_addr);

	return porsr1;
}

int get_clocks(struct sysinfo *sys,
		uintptr_t nxp_dcfg_addr,
		unsigned long nxp_sysclk_freq,
		unsigned long nxp_ddrclk_freq,
		unsigned int nxp_plat_clk_divider)
{
	unsigned int *rcwsr0 = (void *)(nxp_dcfg_addr + RCWSR0_OFFSET);
	const unsigned long sysclk = nxp_sysclk_freq;
	const unsigned long ddrclk = nxp_ddrclk_freq;

	sys->freq_platform = sysclk;
	sys->freq_ddr_pll0 = ddrclk;
	sys->freq_ddr_pll1 = ddrclk;

	sys->freq_platform *= (gur_in32(rcwsr0) >>
				RCWSR0_SYS_PLL_RAT_SHIFT) &
				RCWSR0_SYS_PLL_RAT_MASK;

	sys->freq_platform /= nxp_plat_clk_divider;

	sys->freq_ddr_pll0 *= (gur_in32(rcwsr0) >>
				RCWSR0_MEM_PLL_RAT_SHIFT) &
				RCWSR0_MEM_PLL_RAT_MASK;
	sys->freq_ddr_pll1 *= (gur_in32(rcwsr0) >>
				RCWSR0_MEM2_PLL_RAT_SHIFT) &
				RCWSR0_MEM2_PLL_RAT_MASK;
	if (sys->freq_platform == 0)
		return 1;
	else
		return 0;
}

#if defined (NXP_SFP_ENABLED)
/*******************************************************************************
 * Returns true if secur eboot is enabled on board
 * mode = 0  (development mode - sb_en = 1)
 * mode = 1 (production mode - ITS = 1)
 ******************************************************************************/
bool check_boot_mode_secure(uint32_t *mode,
			    uintptr_t nxp_dcfg_addr,
			    uintptr_t nxp_sfp_addr)
{
	uint32_t val = 0;
	uint32_t *rcwsr = (void *)(nxp_dcfg_addr + RCWSR_SB_EN_OFFSET);
	*mode = 0;

	if (sfp_check_its(nxp_sfp_addr) == 1) {
		/* ITS =1 , Production mode */
		*mode = 1;
		return true;
	}

	val = (gur_in32(rcwsr) >> RCWSR_SBEN_SHIFT) &
				RCWSR_SBEN_MASK;

	if (val == RCWSR_SBEN_MASK) {
		*mode = 0;
		return true;
	}

	return false;
}
#endif
