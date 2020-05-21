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
static soc_info_t soc_info = {0};
static devdisr5_info_t devdisr5_info = {0};

uint32_t read_saved_porsr1(void)
{
	return porsr1;
}

/* Read the PORSR1 register */
uint32_t read_reg_porsr1(uintptr_t nxp_dcfg_addr)
{
	unsigned int *porsr1_addr = NULL;

	porsr1_addr = (void *)(nxp_dcfg_addr + DCFG_PORSR1_OFFSET);
	porsr1 = gur_in32(porsr1_addr);

	return porsr1;
}

const soc_info_t *get_soc_info(uintptr_t nxp_dcfg_addr)
{
	uint32_t reg;

	if (soc_info.is_populated == true)
		return (const soc_info_t *) &soc_info;

	reg = gur_in32(nxp_dcfg_addr + DCFG_SVR_OFFSET);

	soc_info.mfr_id = (reg & SVR_MFR_ID_MASK) >> SVR_MFR_ID_SHIFT;
#if defined(CONFIG_CHASSIS_3_2)
	soc_info.family = (reg & SVR_FAMILY_MASK) >> SVR_FAMILY_SHIFT;
	soc_info.dev_id = (reg & SVR_DEV_ID_MASK) >> SVR_DEV_ID_SHIFT;
#endif
	/* zero means SEC enabled. */
	soc_info.sec_enabled = ((reg & SVR_SEC_MASK) >> SVR_SEC_SHIFT) ? 0 : 1;

	soc_info.personality = (reg & SVR_PERSONALITY_MASK)
				>> SVR_PERSONALITY_SHIFT;
	soc_info.maj_ver = (reg & SVR_MAJ_VER_MASK) >> SVR_MAJ_VER_SHIFT;
	soc_info.min_ver = reg & SVR_MIN_VER_MASK;

	soc_info.is_populated = true;
	return (const soc_info_t *) &soc_info;
}

const devdisr5_info_t *get_devdisr5_info(uintptr_t nxp_dcfg_addr)
{
	uint32_t reg;

	if (devdisr5_info.is_populated == true)
		return (const devdisr5_info_t *) &devdisr5_info;

	reg = gur_in32(nxp_dcfg_addr + DCFG_DEVDISR5_OFFSET);

#if defined(CONFIG_CHASSIS_3_2)
	devdisr5_info.ddrc1_present = (reg & DISR5_DDRC1_MASK) ? 0 : 1;
	devdisr5_info.ddrc2_present = (reg & DISR5_DDRC2_MASK) ? 0 : 1;
	devdisr5_info.ocram_present = (reg & DISR5_OCRAM_MASK) ? 0 : 1;
#endif
	devdisr5_info.is_populated = true;

	return (const devdisr5_info_t *) &devdisr5_info;
}

int get_clocks(struct sysinfo *sys,
		uintptr_t nxp_dcfg_addr,
		unsigned long nxp_sysclk_freq,
		unsigned long nxp_ddrclk_freq,
		unsigned int nxp_plat_clk_divider)
{
	unsigned int *rcwsr0 = NULL;
	const unsigned long sysclk = nxp_sysclk_freq;
	const unsigned long ddrclk = nxp_ddrclk_freq;

	rcwsr0 = (void *)(nxp_dcfg_addr + RCWSR0_OFFSET);
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
	uint32_t *rcwsr = NULL;
	*mode = 0;

	if (sfp_check_its(nxp_sfp_addr) == 1) {
		/* ITS =1 , Production mode */
		*mode = 1;
		return true;
	}

	rcwsr = (void *)(nxp_dcfg_addr + RCWSR_SB_EN_OFFSET);

	val = (gur_in32(rcwsr) >> RCWSR_SBEN_SHIFT) &
				RCWSR_SBEN_MASK;

	if (val == RCWSR_SBEN_MASK) {
		*mode = 0;
		return true;
	}

	return false;
}
#endif
