/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_COMMON_H
#define PLAT_COMMON_H

#include <stdbool.h>

#include <lib/el3_runtime/cpu_data.h>
#include <platform_def.h>

/*******************************************************************************
 * This structure represents the superset of information that can be passed to
 * BL31 e.g. while passing control to it from BL2. The BL32 parameters will be
 * populated only if BL2 detects its presence. A pointer to a structure of this
 * type should be passed in X0 to BL31's cold boot entrypoint.
 *
 * Use of this structure and the X0 parameter is not mandatory: the BL31
 * platform code can use other mechanisms to provide the necessary information
 * about BL32 and BL33 to the common and SPD code.
 *
 * BL31 image information is mandatory if this structure is used. If either of
 * the optional BL32 and BL33 image information is not provided, this is
 * indicated by the respective image_info pointers being zero.
 ******************************************************************************/
typedef struct bl31_params {
	param_header_t h;
	image_info_t *bl31_image_info;
	entry_point_info_t *bl32_ep_info;
	image_info_t *bl32_image_info;
	entry_point_info_t *bl33_ep_info;
	image_info_t *bl33_image_info;
} bl31_params_t;

/* Structure to define SoC personality */
struct soc_type {
	char name[10];
	uint32_t version;
	uint32_t num_clusters;
	uint32_t cores_per_cluster;
};

#define SOC_ENTRY(n, v, ncl, nc) {	\
		.name = #n,		\
		.version = SVR_##v,	\
		.num_clusters = (ncl),	\
		.cores_per_cluster = (nc)}

enum boot_device {
	BOOT_DEVICE_IFC_NOR,
	BOOT_DEVICE_IFC_NAND,
	BOOT_DEVICE_QSPI,
	BOOT_DEVICE_EMMC,
	BOOT_DEVICE_SDHC2_EMMC,
	BOOT_DEVICE_FLEXSPI_NOR,
	BOOT_DEVICE_FLEXSPI_NAND,
	BOOT_DEVICE_NONE
};

enum boot_device get_boot_dev(void);

typedef struct {
	uint64_t addr;
	uint64_t size;
} region_info_t;

typedef struct {
	uint64_t num_dram_regions;
	uint64_t total_dram_size;
	region_info_t region[NUM_DRAM_REGIONS];
} dram_regions_info_t;


dram_regions_info_t *get_dram_regions_info(void);
void mmap_add_ddr_region_dynamically(void);

void set_base_freq_CNTFID0(void);

/* BL3 utility functions */
void ls_bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2);
/* Interconnect CCI/CCN functions */
void plat_ls_interconnect_enter_coherency(unsigned int num_clusters);
void enable_timer_base_to_cluster(void);
void enable_core_tb(void);

/* IO storage utility functions */
int plat_io_setup(void);
int open_backend(const uintptr_t spec);
int plat_io_block_setup(size_t fip_offset, uintptr_t block_dev_spec);
int plat_io_memmap_setup(size_t fip_offset);

/* GIC common API's */
void plat_ls_gic_driver_init(void);
void plat_ls_gic_init(void);
void plat_ls_gic_cpuif_enable(void);
void plat_ls_gic_cpuif_disable(void);
void plat_ls_gic_redistif_on(void);
void plat_ls_gic_redistif_off(void);
void plat_gic_pcpu_init(void);

void ls_bl2_plat_arch_setup(void);
void ls_bl2_el3_plat_arch_setup(void);
void plat_ls_security_setup(void);
uint32_t ls_get_spsr_for_bl32_entry(void);
uint32_t ls_get_spsr_for_bl33_entry(void);
unsigned int plat_my_core_mask(void);
unsigned int plat_core_mask(u_register_t mpidr);
unsigned int plat_core_pos(u_register_t mpidr);
void soc_init_start(void);
void soc_init_finish(void);
void soc_init_percpu(void);
void _soc_set_start_addr(u_register_t addr);
void _init_global_data(void);
void _initialize_psci(void);
uint32_t _getCoreState(u_register_t core_mask);
void _setCoreState(u_register_t core_mask, u_register_t core_state);

#ifdef LS_SYS_TIMCTL_BASE
void ls_configure_sys_timer(void);
#endif
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr);
void ls_setup_page_tables(uintptr_t total_base,
			size_t total_size,
			uintptr_t code_start,
			uintptr_t code_limit,
			uintptr_t rodata_start,
			uintptr_t rodata_limit
#if USE_COHERENT_MEM
			, uintptr_t coh_start,
			uintptr_t coh_limit
#endif
);

/* PSCI utility functions */
int ls_check_mpidr(u_register_t mpidr);

/* DDR Related functions */
#if DDR_INIT
#ifdef NXP_WARM_BOOT
long long _init_ddr(uint32_t wrm_bt_flg);
#else
long long _init_ddr(void);
#endif
#endif

/* GIC utility functions */
void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base);

int ls_sd_emmc_init(uintptr_t *block_dev_spec);

/* SoC specific functions */
void soc_early_init(void);
void soc_init(void);
void soc_mem_access(void);

/*
 * Function to initialize platform's console
 * and register with console framework
 */
void plat_console_init(void);

void _set_platform_security(void);

void wdt_init(void);
void wdt_reset(void);

/* Board specific weak functions */
void board_enable_povdd(void);
void board_disable_povdd(void);

#endif /* PLAT_COMMON_H */
