/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 */

#include <platform_def.h>
#include <arch.h>
#include <cassert.h>
#include <plat_common.h>
#include <io.h>
#include <mmio.h>
#include <errata.h>
#include <ccn.h>
#include <plat_tzc400.h>
#include <debug.h>
#include <xlat_tables_v2.h>
#if POLICY_OTA
#include <ls_ota.h>
#endif

static unsigned char _power_domain_tree_desc[NUMBER_OF_CLUSTERS + 2];
static struct soc_type soc_list[] =  {
	SOC_ENTRY(LS2080A, LS2080A, 4, 2),
	SOC_ENTRY(LS2088A, LS2088A, 4, 2),
	SOC_ENTRY(LS2084A, LS2084A, 4, 2),
	SOC_ENTRY(LS2081A, LS2081A, 4, 2),
	SOC_ENTRY(LS2048A, LS2048A, 2, 2),
	SOC_ENTRY(LS2044A, LS2044A, 2, 2),
	SOC_ENTRY(LS2041A, LS2041A, 2, 2),
	SOC_ENTRY(LS2040A, LS2040A, 2, 2),
};

static const unsigned char master_to_2rn_id_map[] = {
	PLAT_2CLUSTER_TO_CCN_ID_MAP
};

static const unsigned char master_to_rn_id_map[] = {
	PLAT_CLUSTER_TO_CCN_ID_MAP
};

static const ccn_desc_t plat_two_cluster_ccn_desc = {
	.periphbase = NXP_CCN_ADDR,
	.num_masters = ARRAY_SIZE(master_to_2rn_id_map),
	.master_to_rn_id_map = master_to_2rn_id_map
};

static const ccn_desc_t plat_ccn_desc = {
	.periphbase = NXP_CCN_ADDR,
	.num_masters = ARRAY_SIZE(master_to_rn_id_map),
	.master_to_rn_id_map = master_to_rn_id_map
};

/*******************************************************************************
 * This function dynamically constructs the topology according to
 *  SoC Flavor and returns it.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	uint32_t *ccsr_svr = (uint32_t *)(NXP_DCFG_ADDR + DCFG_SVR_OFFSET);
	uint32_t num_clusters = NUMBER_OF_CLUSTERS;
	uint32_t cores_per_cluster = CORES_PER_CLUSTER;
	uint32_t svr = mmio_read_32((uintptr_t)ccsr_svr);
	unsigned int i, ver;

	ver = (svr >> 8) & SVR_WO_E;

	for (i = 0; i < ARRAY_SIZE(soc_list); i++) {
		if ((soc_list[i].version & SVR_WO_E) == ver) {
			num_clusters = soc_list[i].num_clusters;
			cores_per_cluster = soc_list[i].cores_per_cluster;
			break;
		}
	}

	/*
	 * The highest level is the system level. The next level is constituted
	 * by clusters and then cores in clusters.
	 */
	_power_domain_tree_desc[0] = 1;
	_power_domain_tree_desc[1] = num_clusters;

	for (i = 0; i < _power_domain_tree_desc[1]; i++)
		_power_domain_tree_desc[i + 2] = cores_per_cluster;


	return _power_domain_tree_desc;
}

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
		assert_invalid_ls2088a_cluster_count);

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	return CORES_PER_CLUSTER;
}

/*******************************************************************************
 * This function returns the number of clusters in the SoC
 ******************************************************************************/
static unsigned int get_num_cluster()
{
	uint32_t *ccsr_svr = (uint32_t *)(NXP_DCFG_ADDR + DCFG_SVR_OFFSET);
	uint32_t num_clusters = NUMBER_OF_CLUSTERS;
	uint32_t svr = mmio_read_32((uintptr_t)ccsr_svr);
	unsigned int i, ver;

	ver = (svr >> 8) & SVR_WO_E;

	for (i = 0; i < ARRAY_SIZE(soc_list); i++) {
		if ((soc_list[i].version & SVR_WO_E) == ver) {
			num_clusters = soc_list[i].num_clusters;
			break;
		}
	}

	return num_clusters;
}

/*******************************************************************************
 * This function returns the total number of cores in the SoC
 ******************************************************************************/
unsigned int get_tot_num_cores()
{
	uint32_t *ccsr_svr = (uint32_t *)(NXP_DCFG_ADDR + DCFG_SVR_OFFSET);
	uint32_t num_clusters = NUMBER_OF_CLUSTERS;
	uint32_t cores_per_cluster = CORES_PER_CLUSTER;
	uint32_t svr = mmio_read_32((uintptr_t)ccsr_svr);
	unsigned int i, ver;

	ver = (svr >> 8) & SVR_WO_E;

	for (i = 0; i < ARRAY_SIZE(soc_list); i++) {
		if ((soc_list[i].version & SVR_WO_E) == ver) {
			num_clusters = soc_list[i].num_clusters;
			cores_per_cluster = soc_list[i].cores_per_cluster;
			break;
		}
	}

	return (num_clusters * cores_per_cluster);
}

/*******************************************************************************
 * This function returns the PMU IDLE Cluster mask.
 ******************************************************************************/
unsigned int get_pmu_idle_cluster_mask()
{
	return ((1 << get_num_cluster()) - 2);
}

/*******************************************************************************
 * This function returns the PMU Flush Cluster mask.
 ******************************************************************************/
unsigned int get_pmu_flush_cluster_mask()
{
	return ((1 << get_num_cluster()) - 2);
}

/*******************************************************************************
 * This function returns the PMU IDLE Core mask.
 ******************************************************************************/
unsigned int get_pmu_idle_core_mask()
{
	return ((1 << get_tot_num_cores()) - 2) ;
}

static void soc_interconnect_config(void)
{
	unsigned long long val = 0x0;
	uint32_t rni_map[] = {0, 2, 6, 12, 16, 20};
	uint32_t idx = 0;
	uint32_t num_clusters = get_num_cluster();

	if (num_clusters == 2)
		ccn_init(&plat_two_cluster_ccn_desc);
	else
		ccn_init(&plat_ccn_desc);

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_ls_interconnect_enter_coherency(num_clusters);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, PCIeRC_RN_I_NODE_ID_OFFSET);
	val |= (1 << 12);
	ccn_write_node_reg(NODE_TYPE_HNI, 10, PCIeRC_RN_I_NODE_ID_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET);
	val &= ~(ENABLE_RESERVE_BIT53);
	val |= SERIALIZE_DEV_nGnRnE_WRITES;
	ccn_write_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, PoS_CONTROL_REG_OFFSET);
	val &= ~(HNI_POS_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 10, PoS_CONTROL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET);
	val &= ~(POS_EARLY_WR_COMP_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET, val);

#if POLICY_PERF_WRIOP
	val = ccn_read_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_WUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET, val);
#else
	val = ccn_read_node_reg(NODE_TYPE_RNI, 12, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_WUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 12, SA_AUX_CTRL_REG_OFFSET, val);
#endif

	val = ccn_read_node_reg(NODE_TYPE_RNI, 6, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_FORCE_RD_QUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 6, SA_AUX_CTRL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_FORCE_RD_QUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET, val);

	/* RN-I map in MN Memory map is in-correctly set.
	 * Bit corresponding to Node Id 0 is not set.
	 * Hence loop below is started from 1.
	 */
	val = mmio_read_64(NXP_CCN_ADDR + (128 * 0x10000)
						+ PORT_S0_CTRL_REG_RNI);
	val |= QOS_SETTING;
	mmio_write_64(NXP_CCN_ADDR + (128 * 0x10000) + PORT_S0_CTRL_REG_RNI,
			val);

	val = mmio_read_64(NXP_CCN_ADDR + (128 * 0x10000)
						+ PORT_S1_CTRL_REG_RNI);
	val |= QOS_SETTING;
	mmio_write_64(NXP_CCN_ADDR + (128 * 0x10000) + PORT_S1_CTRL_REG_RNI,
			val);

	val = mmio_read_64(NXP_CCN_ADDR + (128 * 0x10000)
						+ PORT_S2_CTRL_REG_RNI);
	val |= QOS_SETTING;
	mmio_write_64(NXP_CCN_ADDR + (128 * 0x10000) + PORT_S2_CTRL_REG_RNI,
			val);

	for (idx = 1; idx < (sizeof(rni_map)/sizeof(uint32_t)); idx++) {
		val = ccn_read_node_reg(NODE_TYPE_RNI, rni_map[idx],
							PORT_S0_CTRL_REG_RNI);
		val |= QOS_SETTING;
		ccn_write_node_reg(NODE_TYPE_RNI, rni_map[idx],
						PORT_S0_CTRL_REG_RNI, val);

		val = ccn_read_node_reg(NODE_TYPE_RNI, rni_map[idx],
							PORT_S1_CTRL_REG_RNI);
		val |= QOS_SETTING;
		ccn_write_node_reg(NODE_TYPE_RNI, rni_map[idx],
						PORT_S1_CTRL_REG_RNI, val);

		val = ccn_read_node_reg(NODE_TYPE_RNI, rni_map[idx],
							PORT_S2_CTRL_REG_RNI);
		val |= QOS_SETTING;
		ccn_write_node_reg(NODE_TYPE_RNI, rni_map[idx],
						PORT_S2_CTRL_REG_RNI, val);
	}
}

/******************************************************************************
 *****************************************************************************/
static void bypass_smmu(void)
{
	uint32_t val;

	val = ((mmio_read_32(SMMU_SCR0) | SCR0_CLIENTPD_MASK)) &
						~(SCR0_USFCFG_MASK);
	mmio_write_32(SMMU_SCR0, val);

	val = (mmio_read_32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) &
						~(SCR0_USFCFG_MASK);
	mmio_write_32(SMMU_NSCR0, val);
}
/*******************************************************************************
 * This function implements soc specific erratas
 * This is called before DDR is initialized or MMU is enabled
 ******************************************************************************/
void soc_early_init(void)
{
	uint32_t mode;

	enable_timer_base_to_cluster();
	enable_core_tb();
	soc_interconnect_config();

	enum  boot_device dev = get_boot_dev();
	/* Mark the buffer for SD in OCRAM as non secure.
	 * The buffer is assumed to be at end of OCRAM for
	 * the logic below to calculate TZPC programming
	 */
	if (dev == BOOT_DEVICE_EMMC) {
		/* Calculate the region in OCRAM which is secure
		 * The buffer for SD needs to be marked non-secure
		 * to allow SD to do DMA operations on it
		 */
		uint32_t secure_region = (NXP_OCRAM_SIZE - NXP_SD_BLOCK_BUF_SIZE);
		uint32_t mask = secure_region/TZPC_BLOCK_SIZE;
		mmio_write_32(NXP_OCRAM_TZPC_ADDR, mask);

		/* Add the entry for buffer in MMU Table */
		mmap_add_region(NXP_SD_BLOCK_BUF_ADDR, NXP_SD_BLOCK_BUF_ADDR,
				NXP_SD_BLOCK_BUF_SIZE, MT_DEVICE | MT_RW | MT_NS);
	}

	/* For secure boot disable SMMU.
	 * Later when platform security policy comes in picture,
	 * this might get modified based on the policy
	 */
	if (check_boot_mode_secure(&mode) == true) {
		bypass_smmu();
	}

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

#ifndef TEST_BL31
	if (ccn_get_part0_id(NXP_CCN_ADDR) != CCN_504_PART0_ID) {
		ERROR("Unrecognized CCN variant detected. Only CCN-504"
				" is supported");
		panic();
	}
#endif

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	uint32_t num_clusters = get_num_cluster();

	if (num_clusters == 2)
		ccn_init(&plat_two_cluster_ccn_desc);
	else
		ccn_init(&plat_ccn_desc);

	plat_ls_interconnect_enter_coherency(num_clusters);

	/* Set platform security policies */
	_set_platform_security();

	 /* make sure any parallel init tasks are finished */
	soc_init_finish();

}

/*******************************************************************************
 * Function to return the SoC SYS CLK
 ******************************************************************************/
unsigned int get_sys_clk(void)
{
	return NXP_SYSCLK_FREQ;
}

/******************************************************************************
 * Function returns the base counter frequency
 * after reading the first entry at CNTFID0 (0x20 offset).
 *
 * Function is used by:
 *   1. ARM common code for PSCI management.
 *   2. ARM Generic Timer init.
 *
 *****************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with SYS_REF_CLK/4 frequency clock.
	 *
	 *
	 */
	counter_base_frequency = mmio_read_32(NXP_TIMER_ADDR + CNTFID_OFF);

	return counter_base_frequency;
}

/*******************************************************************************
 * Configure memory access permissions
 *   - Region 0 with no access;
 *   - Region 1 to 4 with ?????
 ******************************************************************************/
static void mem_access_setup(uintptr_t base, uint32_t total_regions)
{
	uint32_t index = 0;

	INFO("Configuring TrustZone Controller\n");

	tzc400_init(base);

	/* Disable filters. */
	tzc400_disable_filters();

	/* Region 0 set to no access by default */
	tzc400_configure_region0(TZC_REGION_S_NONE, 0);

	for (index = 1; index < total_regions; index++) {
		tzc400_configure_region(
			tzc400_reg_list[index].reg_filter_en,
			index,
			tzc400_reg_list[index].start_addr,
			tzc400_reg_list[index].end_addr,
			tzc400_reg_list[index].sec_attr,
			tzc400_reg_list[index].nsaid_permissions);
	}

	/*
	 * Raise an exception if a NS device tries to access secure memory
	 * TODO: Add interrupt handling support.
	 */
	tzc400_set_action(TZC_ACTION_ERR);

	/* Enable filters. */
	tzc400_enable_filters();
}

/*****************************************************************************
 * This function sets up access permissions on memory regions
 ****************************************************************************/
void soc_mem_access(void)
{
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	int i = 0;
	/* index 0 is reserved for region-0 */
	int index = 1;

	for (i = 0; i < info_dram_regions->num_dram_regions; i++) {
		if (info_dram_regions->region[i].size == 0)
			break;

		if (i == 0) {
			/* TZC Region 1 on DRAM0 for Secure Memory*/
			tzc400_reg_list[index].start_addr =
					info_dram_regions->region[i].addr
					+ info_dram_regions->region[i].size;
			tzc400_reg_list[index].end_addr =
					info_dram_regions->region[i].addr
					+ info_dram_regions->region[i].size
					+ NXP_SECURE_DRAM_SIZE
					- 1;
			index++;

			/* TZC Region 2 on DRAM0 for Shared Memory*/
			tzc400_reg_list[index].start_addr =
					info_dram_regions->region[i].addr
					+ info_dram_regions->region[i].size
					+ NXP_SECURE_DRAM_SIZE;
			tzc400_reg_list[index].end_addr =
					info_dram_regions->region[i].addr
					+ info_dram_regions->region[i].size
					+ NXP_SECURE_DRAM_SIZE
					+ NXP_SP_SHRD_DRAM_SIZE
					- 1;
			index++;

			/* TZC Region 3 on DRAM0 for Non-Secure Memory*/
			tzc400_reg_list[index].start_addr =
					info_dram_regions->region[i].addr;
			tzc400_reg_list[index].end_addr =
					info_dram_regions->region[i].addr
					+ info_dram_regions->region[i].size
					- 1;
			index++;
		} else {
			/* TZC Region 3+i on DRAM(> 0) for Non-Secure Memory*/
			tzc400_reg_list[index].start_addr =
					info_dram_regions->region[i].addr;
			tzc400_reg_list[index].end_addr =
					info_dram_regions->region[i].addr
					+ info_dram_regions->region[i].size
					- 1;
			index++;
		}
	}

	mem_access_setup(NXP_TZC_ADDR, index);
	mem_access_setup(NXP_TZC2_ADDR, index);
}

/*****************************************************************************
 * This function returns the boot device based on RCW_SRC
 ****************************************************************************/
enum boot_device get_boot_dev(void)
{
	enum boot_device src = BOOT_DEVICE_NONE;
	uint32_t porsr1;
	uint32_t rcw_src, val;

	porsr1 = read_reg_porsr1();

	rcw_src = (porsr1 & PORSR1_RCW_MASK) >> PORSR1_RCW_SHIFT;
	/*
	 * In case of some errata this value might get zeroized
	 * Use the saved porsr1 value in that case
	 */
	if (!rcw_src) {
		porsr1 = read_saved_porsr1();
		rcw_src = (porsr1 & PORSR1_RCW_MASK) >> PORSR1_RCW_SHIFT;
	}

	/* RCW SRC NOR */
	val = rcw_src & RCW_SRC_TYPE_MASK;
	if (val == NOR_16B_VAL) {
		src = BOOT_DEVICE_IFC_NOR;
		INFO("RCW BOOT SRC is IFC NOR\n");
	} else {
		val = rcw_src & RCW_SRC_SERIAL_MASK;
		switch (val) {
		case QSPI_VAL:
			src = BOOT_DEVICE_QSPI;
			INFO("RCW BOOT SRC is QSPI\n");
			break;
		case SDHC_VAL:
			src = BOOT_DEVICE_EMMC;
			INFO("RCW BOOT SRC is SD/EMMC\n");
			break;
		case EMMC_VAL:
			src = BOOT_DEVICE_EMMC;
			INFO("RCW BOOT SRC is SD/EMMC\n");
			break;
		default:
			src = BOOT_DEVICE_NONE;
		}
	}

#if POLICY_OTA
	if (ota_status_check() == 0)
		src = BOOT_DEVICE_EMMC;
#endif

	return src;
}
