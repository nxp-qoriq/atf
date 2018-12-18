/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <assert.h>
#include <bl_common.h>
#include <desc_image_load.h>
#include <uart_16550.h>
#include <console.h>
#include <plat_common.h>
#include <tbbr_img_def.h>
#include <mmu_def.h>
#include <sfp.h>

#define BL2_END                    (unsigned long)(&__BL2_END__)

#pragma weak bl2_el3_early_platform_setup
#pragma weak bl2_el3_plat_arch_setup
#pragma weak bl2_el3_plat_prepare_exit
#pragma weak soc_early_init

void soc_early_init(void)
{
}

static uint32_t porsr1;
static dram_regions_info_t dram_regions_info  = {0};

uint32_t read_saved_porsr1(void)
{
	return porsr1;
}

/*******************************************************************************
 * Return the pointer to the 'dram_regions_info structure of the DRAM.
 * This structure is populated after _init_ddr().
 ******************************************************************************/
dram_regions_info_t *get_dram_regions_info(void)
{
	return &dram_regions_info;
}

static void populate_dram_regions_info(void)
{
	long long dram_remain_size = dram_regions_info.total_dram_size;
	uint8_t reg_id = 0;

	dram_regions_info.region[reg_id].addr = NXP_DRAM0_ADDR;
	dram_regions_info.region[reg_id].size =
			dram_remain_size > NXP_DRAM0_MAX_SIZE ?
				NXP_DRAM0_MAX_SIZE : dram_remain_size;

	if (dram_regions_info.region[reg_id].size != NXP_DRAM0_SIZE)
		NOTICE("Incorrect DRAM0 size is defined in platfor_def.h\n");

	dram_remain_size -= dram_regions_info.region[reg_id].size;
	dram_regions_info.region[reg_id].size -= (NXP_SECURE_DRAM_SIZE
						+ NXP_SP_SHRD_DRAM_SIZE);

	assert(dram_regions_info.region[reg_id].size > 0);

	/* Reducing total dram size by 66MB */
	dram_regions_info.total_dram_size -= (NXP_SECURE_DRAM_SIZE
						+ NXP_SP_SHRD_DRAM_SIZE);

#if defined(NXP_DRAM1_ADDR) && defined(NXP_DRAM1_MAX_SIZE)
	if (dram_remain_size > 0) {
		reg_id++;
		dram_regions_info.region[reg_id].addr = NXP_DRAM1_ADDR;
		dram_regions_info.region[reg_id].size =
				dram_remain_size > NXP_DRAM1_MAX_SIZE ?
					NXP_DRAM1_MAX_SIZE : dram_remain_size;
		dram_remain_size -= dram_regions_info.region[reg_id].size;
	}
#endif
#if defined(NXP_DRAM1_ADDR) && defined(NXP_DRAM1_MAX_SIZE)
	if (dram_remain_size > 0) {
		reg_id++;
		dram_regions_info.region[reg_id].addr = NXP_DRAM1_ADDR;
		dram_regions_info.region[reg_id].size =
				dram_remain_size > NXP_DRAM1_MAX_SIZE ?
					NXP_DRAM1_MAX_SIZE : dram_remain_size;
		dram_remain_size -= dram_regions_info.region[reg_id].size;
	}
#endif
	reg_id++;
	dram_regions_info.num_dram_regions = reg_id;
}

void bl2_el3_early_platform_setup(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init();
#endif

	/* Read the PORSR1 value and store it in global variable
	 * PORSR1 value contains the RCW SRC.
	 * In later functions there is a possibility that this
	 * value is over-ridden by some errata eg A009660
	 */
	porsr1 = read_reg_porsr1();

	/*
	 * SoC specific early init
	 * Any errata handling or SoC specific early initialization can
	 * be done here
	 * Set Counter Base Frequency in CNTFID0 and in cntfrq_el0.
	 * Initialize the interconnect. 
	 * Enable coherency for primary CPU cluster
	 */
	soc_early_init();


	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	delay_timer_init();

	/* Initialise the IO layer and register platform IO devices */
	plat_io_setup();

#ifndef DDR_LATE_INIT
	/* Initialize DDR */
	i2c_init();
	dram_regions_info.total_dram_size = _init_ddr();
	if (dram_regions_info.total_dram_size < NXP_DRAM0_SIZE) {
		NOTICE("ERROR: DRAM0 Size is not correctly configured.");
		assert(0);
	}
	if (dram_regions_info.total_dram_size > 0)
		populate_dram_regions_info();
#endif
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void ls_bl2_el3_plat_arch_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	ls_setup_page_tables(
#if SEPARATE_RW_AND_NOLOAD
			      BL2_START,
			      BL2_LIMIT - BL2_START,
#else
			      BL2_BASE,
			      BL2_END - BL2_BASE,
#endif
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
			      );

#ifdef AARCH32
	enable_mmu_secure(0);
#else
	enable_mmu_el3(0);
#endif
}

void bl2_el3_plat_arch_setup(void)
{
	ls_bl2_el3_plat_arch_setup();
}

/*
 * Perform ARM standard platform setup.
 */
void ls_bl2_platform_setup(void)
{
}

void bl2_platform_setup(void)
{
	ls_bl2_platform_setup();
}

int ls_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);

	switch (image_id) {
	case BL31_IMAGE_ID:
		bl_mem_params->ep_info.args.arg3 =
					(u_register_t) &dram_regions_info;

		/* Pass the value of PORSR1 regoster in Argument 4 */
		bl_mem_params->ep_info.args.arg4 =
					(u_register_t) porsr1;
		flush_dcache_range((uintptr_t)&dram_regions_info,
				sizeof(dram_regions_info));
		break;
#ifdef AARCH64
	case BL32_IMAGE_ID:
		bl_mem_params->ep_info.spsr = ls_get_spsr_for_bl32_entry();
		break;
#endif
	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = ls_get_spsr_for_bl33_entry();
		break;
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return ls_bl2_handle_post_image_load(image_id);
}

void bl2_el3_plat_prepare_exit(void)
{
}

void bl2_plat_preload_setup(void)
{
/* DDR is initialized late after MMU has been enabled */
#ifdef DDR_LATE_INIT
	/* Initialize DDR */
	i2c_init();
	dram_regions_info.total_dram_size = _init_ddr();
	if (dram_regions_info.total_dram_size < NXP_DRAM0_SIZE) {
		NOTICE("ERROR: DRAM0 Size is not correctly configured.");
		assert(0);
	}
	if (dram_regions_info.total_dram_size > 0)
		populate_dram_regions_info();

	mmap_add_ddr_region_dynamically();
#endif

    /* setup the memory region access permissions */
	soc_mem_access();

#ifdef POLICY_FUSE_PROVISION
	uint32_t size;
	uintptr_t image_buf;
	int ret = -1;
	if (!sfp_check_oem_wp()) {
		size = FUSE_SZ;
		image_buf = (uintptr_t)FUSE_BUF;
		ret = load_img(FUSE_PROV_IMAGE_ID, &image_buf, &size);
		if (ret) {
			ERROR("Failed to load FUSE PRIV image\n");
			assert(ret == 0);
		}
		provision_fuses(image_buf);
	}
#endif
}
