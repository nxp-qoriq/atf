/*
 * Copyright 2018,2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Jiafei Pan <jiafei.pan@nxp.com>
 * 	  Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <platform_def.h>
#include <plat_common.h>
#include <arch.h>
#include <arch_helpers.h>
#include <xlat_tables_v2.h>
#include <assert.h>
#include <debug.h>
#include <plat_common.h>
#include <io.h>
#include <mmio.h>
#include <mmu_def.h>
#include <platform.h>

const mmap_region_t *plat_ls_get_mmap(void);

/*
 * Table of memory regions for various BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as arm_setup_page_tables() already
 * takes care of mapping it.
 *
 * The flash needs to be mapped as writable in order to erase the FIP's Table of
 * Contents in case of unrecoverable error (see plat_error_handler()).
 */
#ifdef IMAGE_BL2
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
	{0}
};
#endif

#ifdef IMAGE_BL31
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
#ifdef NXP_DCSR_ADDR
	LS_MAP_DCSR,
#endif
	LS_MAP_OCRAM,
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
	LS_MAP_BL32_SEC_MEM,
	{0}
};
#endif

/* Weak definitions may be overridden in specific NXP SoC */
#pragma weak plat_get_ns_image_entrypoint
#pragma weak plat_ls_get_mmap

#if defined(IMAGE_BL31) || !defined(DDR_LATE_INIT) 
static void mmap_add_ddr_regions_statically(void)
{
	int i = 0;
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	/* MMU map for Non-Secure DRAM Regions */
	VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
	mmap_add_region(info_dram_regions->region[i].addr,
			info_dram_regions->region[i].addr,
			info_dram_regions->region[i].size,
			MT_MEMORY | MT_RW | MT_NS);

	/* MMU map for Secure DDR Region on DRAM-0 */
	if (info_dram_regions->region[i].size >
		(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE)) {
		VERBOSE("Secure DRAM Region %d: %p - %p\n", i,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				+ NXP_SECURE_DRAM_SIZE
				+ NXP_SP_SHRD_DRAM_SIZE
				- 1));
		mmap_add_region((info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE),
				MT_MEMORY | MT_RW | MT_SECURE);
	}

#ifdef IMAGE_BL31
	for (i = 1; i < info_dram_regions->num_dram_regions; i++) {
		if (info_dram_regions->region[i].size == 0)
			break;
		VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
		mmap_add_region(info_dram_regions->region[i].addr,
				info_dram_regions->region[i].addr,
				info_dram_regions->region[i].size,
				MT_MEMORY | MT_RW | MT_NS);
	}
#endif
}
#endif

#ifdef PLAT_XLAT_TABLES_DYNAMIC
void mmap_add_ddr_region_dynamically(void)
{
	int i = 0;
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	/* MMU map for Non-Secure DRAM Regions */
	VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
	mmap_add_dynamic_region(info_dram_regions->region[i].addr,
			info_dram_regions->region[i].addr,
			info_dram_regions->region[i].size,
			MT_MEMORY | MT_RW | MT_NS);

	/* MMU map for Secure DDR Region on DRAM-0 */
	if (info_dram_regions->region[i].size >
		(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE)) {
		VERBOSE("Secure DRAM Region %d: %p - %p\n", i,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				+ NXP_SECURE_DRAM_SIZE
				+ NXP_SP_SHRD_DRAM_SIZE
				- 1));
		mmap_add_dynamic_region((info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE),
				MT_MEMORY | MT_RW | MT_SECURE);
	}

#ifdef IMAGE_BL31
	for (i = 1; i < info_dram_regions->num_dram_regions; i++) {
		if (info_dram_regions->region[i].size == 0)
			break;
		VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
		mmap_add_dynamic_region(info_dram_regions->region[i].addr,
				info_dram_regions->region[i].addr,
				info_dram_regions->region[i].size,
				MT_MEMORY | MT_RW | MT_NS);
	}
#endif
}
#endif

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The extents of the generic memory regions are specified by the function
 * arguments and consist of:
 * - Trusted SRAM seen by the BL image;
 * - Code section;
 * - Read-only data section;
 * - Coherent memory region, if applicable.
 */
void ls_setup_page_tables(uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit
#if USE_COHERENT_MEM
			   ,
			   uintptr_t coh_start,
			   uintptr_t coh_limit
#endif
			   )
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Memory seen by this BL image: %p - %p\n",
		(void *) total_base, (void *) (total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *) code_start, (void *) code_limit);
	mmap_add_region(code_start, code_start,
			code_limit - code_start,
			MT_CODE | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *) rodata_start, (void *) rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start,
			MT_RO_DATA | MT_SECURE);

#if USE_COHERENT_MEM
	/* Re-map the coherent memory region */
	VERBOSE("Coherent region: %p - %p\n",
		(void *) coh_start, (void *) coh_limit);
	mmap_add_region(coh_start, coh_start,
			coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_ls_get_mmap());


#if defined(IMAGE_BL31) || !defined(DDR_LATE_INIT) 
	mmap_add_ddr_regions_statically();
#endif

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t ls_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
#ifndef AARCH32
uint32_t ls_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = EL_IMPLEMENTED(2) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#else
/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
uint32_t ls_get_spsr_for_bl33_entry(void)
{
	unsigned int hyp_status, mode, spsr;

	hyp_status = GET_VIRT_EXT(read_id_pfr1());

	mode = (hyp_status) ? MODE32_hyp : MODE32_svc;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_MODE32(mode, plat_get_ns_image_entrypoint() & 0x1,
			SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#endif /* AARCH32 */

/*******************************************************************************
 * TBD: Configures access to the system counter timer module.
 ******************************************************************************/
#ifdef LS_SYS_TIMCTL_BASE
void ls_configure_sys_timer(void)
{
	unsigned int reg_val;

#if LS_CONFIG_CNTACR
	/* TBD: System Timer implementation
	 * Need to move LS_CONFIG_CNTACR from ls_common.mk
	 * platform.mk
	 */
	reg_val = (1 << CNTACR_RPCT_SHIFT) | (1 << CNTACR_RVCT_SHIFT);
	reg_val |= (1 << CNTACR_RFRQ_SHIFT) | (1 << CNTACR_RVOFF_SHIFT);
	reg_val |= (1 << CNTACR_RWVT_SHIFT) | (1 << CNTACR_RWPT_SHIFT);
	mmio_write_32(LS_SYS_TIMCTL_BASE +
		      CNTACR_BASE(PLAT_LS_NSTIMER_FRAME_ID), reg_val);
	mmio_write_32(LS_SYS_TIMCTL_BASE, plat_get_syscnt_freq2());
#endif /* LS_CONFIG_CNTACR */

	reg_val = (1 << CNTNSAR_NS_SHIFT(PLAT_LS_NSTIMER_FRAME_ID));
	mmio_write_32(LS_SYS_TIMCTL_BASE + CNTNSAR, reg_val);
}
#endif /* LS_SYS_TIMCTL_BASE */

/*******************************************************************************
 * Returns NXP platform specific memory map regions.
 ******************************************************************************/
const mmap_region_t *plat_ls_get_mmap(void)
{
	return plat_ls_mmap;
}

void get_clocks(struct sysinfo *sys)
{
	unsigned int *rcwsr0 = (void *)(NXP_DCFG_ADDR + RCWSR0_OFFSET);
	const unsigned long sysclk = NXP_SYSCLK_FREQ;
	const unsigned long ddrclk = NXP_DDRCLK_FREQ;

	sys->freq_platform = sysclk;
	sys->freq_ddr_pll0 = ddrclk;
	sys->freq_ddr_pll1 = ddrclk;

	sys->freq_platform *= (gur_in32(rcwsr0) >>
				RCWSR0_SYS_PLL_RAT_SHIFT) &
				RCWSR0_SYS_PLL_RAT_MASK;

	sys->freq_platform /= NXP_PLATFORM_CLK_DIVIDER;

	sys->freq_ddr_pll0 *= (gur_in32(rcwsr0) >>
				RCWSR0_MEM_PLL_RAT_SHIFT) &
				RCWSR0_MEM_PLL_RAT_MASK;
	sys->freq_ddr_pll1 *= (gur_in32(rcwsr0) >>
				RCWSR0_MEM2_PLL_RAT_SHIFT) &
				RCWSR0_MEM2_PLL_RAT_MASK;

}

/* Read the PORSR1 register */
uint32_t read_reg_porsr1(void)
{
	unsigned int *porsr1 = (void *)(NXP_DCFG_ADDR + DCFG_PORSR1_OFFSET);
	uint32_t val;

	val = gur_in32(porsr1);

	return val;
}
