/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 *        Rod Dorris <rod.dorris@nxp.com>
 */

#include <platform_def.h>
#include <arch.h>
#include <cassert.h>
#include <plat_common.h>
#include <io.h>
#include <mmio.h>
#include <csu.h>
#include <cci.h>
#include <debug.h>
#include <endian.h>
#if POLICY_OTA
#include <ls_ota.h>
#endif

const unsigned char _power_domain_tree_desc[] = {1,1,1};

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
		assert_invalid_ls1012_cluster_count);

/******************************************************************************
 * This function returns the SoC topology
 *****************************************************************************/

const unsigned char *plat_get_power_domain_tree_desc(void)
{

	return _power_domain_tree_desc;
}

/******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 *****************************************************************************/
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	return CORES_PER_CLUSTER;
}

/*******************************************************************************
 * This function returns the number of clusters in the SoC
 ******************************************************************************/
static unsigned int get_num_cluster()
{
	return NUMBER_OF_CLUSTERS;
}


/******************************************************************************
 * This function implements soc specific erratas
 * This is called before DDR is initialized or MMU is enabled
 *****************************************************************************/
void soc_early_init(void)
{
    
	set_base_freq_CNTFID0();

	/* Enable snooping on SEC read and write transactions */
	scfg_setbits32((void *)(NXP_SCFG_ADDR + SCFG_SNPCNFGCR_OFFSET),
			SCFG_SNPCNFGCR_SECRDSNP | SCFG_SNPCNFGCR_SECWRSNP);

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_ls_interconnect_enter_coherency(get_num_cluster());
}

/******************************************************************************
 * For LS1043a rev1.0, GIC base address align with 4k.
 * For LS1043a rev1.1, if DCFG_GIC400_ALIGN[GIC_ADDR_BIT]
 * is set, GIC base address align with 4K, or else align
 * with 64k.
 *****************************************************************************/
void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base)
{
	*gicc_base = NXP_GICC_4K_ADDR;
	*gicd_base = NXP_GICD_4K_ADDR;
}

/*******************************************************************************
 * This function initializes the soc from the BL31 module
 ******************************************************************************/
void soc_init(void)
{
	 /* low-level init of the soc */
	soc_init_start();
	soc_init_percpu();
	_init_global_data();
	_initialize_psci();

	/*
	 * Initialize the interconnect during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this but we can't
         * assume so. No harm in executing this code twice.
	 */
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));

	/* Init CSU to enable non-secure access to peripherals */
	enable_layerscape_ns_access();
}

/******************************************************************************
 * Function to return the SoC SYS CLK
 *****************************************************************************/
unsigned int get_sys_clk(void)
{
	return NXP_SYSCLK_FREQ;
}

/******************************************************************************
 * Function to set the base counter frequency at the
 * the first entry of the Frequency Mode Table,
 * at CNTFID0 (0x20 offset).
 *
 * Set the value of the pirmary core register cntfrq_el0.
 *****************************************************************************/
void set_base_freq_CNTFID0(void)
{
	/*
	 * The system counter always works with 25 MHz frequency clock.
	 *
	 */
	unsigned int counter_base_frequency = 25000000;

	/* Setting the frequency in the Frequency modes table.
	 *
	 * Note: The value for ls1046ardb board at this offset
	 *       is not RW as stated. This offset have the
	 *       fixed value of 100000400 Hz.
	 *
	 * The below code line has no effect.
	 * Keeping it for other platforms where it has effect.
	 */
	mmio_write_32(NXP_TIMER_ADDR + CNTFID_OFF, counter_base_frequency);

	write_cntfrq_el0(counter_base_frequency);
}

/*
 * Function returns the base counter frequency
 * after reading the first entry at CNTFID0 (0x20 offset).
 *
 * Function is used by:
 *   1. ARM common code for PSCI management.
 *   2. ARM Generic Timer init.
 *
 */
unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with 25000000Hz clock.
	 */
	counter_base_frequency = 25000000;

	return counter_base_frequency;
}

/*****************************************************************************
 * This function sets up access permissions on memory regions
 ****************************************************************************/
void soc_mem_access(void)
{
	/* Nothing to do for ls1012 as there is no TZASC block */
}

/*****************************************************************************
 * This function returns the boot device based on RCW_SRC
 * LS1012 supports only QSPI as boot source
 ****************************************************************************/
enum boot_device get_boot_dev(void)
{
	enum boot_device src = BOOT_DEVICE_QSPI;

#if POLICY_OTA
	if (ota_status_check() == 0)
		src = BOOT_DEVICE_EMMC;
#else
	INFO("BOOT SRC is QSPI\n");
#endif

	return src;
}
