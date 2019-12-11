/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Jiafei Pan <Jiafei.Pan@nxp.com>
 */

#ifndef __PLAT_LS_H__
#define __PLAT_LS_H__

#include <sys/types.h>
#include <stdbool.h>
#include <cpu_data.h>
#include <platform_def.h>

/* Structure to define SoC personality */
struct soc_type {
	char name[10];
	uint32_t version;
	uint32_t num_clusters;
	uint32_t cores_per_cluster;
};

#define SOC_ENTRY(n, v, ncl, nc) \
        { .name = #n, .version = SVR_##v, .num_clusters = (ncl), .cores_per_cluster = (nc)}

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

uint32_t read_reg_porsr1(void);
uint32_t read_saved_porsr1(void);
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
void plat_ls_gic_pcpu_init(void);

void ls_bl2_plat_arch_setup(void);
void ls_bl2_el3_plat_arch_setup(void);
void plat_ls_security_setup(void);
uint32_t ls_get_spsr_for_bl32_entry(void);
uint32_t ls_get_spsr_for_bl33_entry(void);
int plat_core_pos_by_mpidr(u_register_t mpidr);
unsigned int plat_my_core_mask(void);
unsigned int plat_core_mask(u_register_t mpidr);
unsigned int plat_my_core_pos(void);
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

/* Security utility functions */
int tzc380_setup(void);

/* Timer utility functions */
uint64_t get_timer_val(uint64_t start);
void delay_timer_init(void);

/* DDR Related functions */
#if DDR_INIT
long long _init_ddr(void);
void i2c_init(void);
#endif
int load_img(unsigned int image_id, uintptr_t *image_base,
		      uint32_t *image_size);

/* GIC utility functions */
void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base);

int ls_sd_emmc_init(uintptr_t *block_dev_spec);

/* SoC specific functions */
void soc_early_init(void);
void soc_init(void);
void soc_mem_access(void);

bool check_boot_mode_secure(uint32_t *mode);

/*
 * Function to initalize platform's console
 * and register with console framework
 */
void plat_console_init(void);

struct sysinfo {
	unsigned long freq_platform;
	unsigned long freq_ddr_pll0;
	unsigned long freq_ddr_pll1;
};

void get_clocks(struct sysinfo *sys);
void _set_platform_security(void);

void wdt_init(void);
void wdt_reset(void);

#endif /* __PLAT_LS_H__ */
