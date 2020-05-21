/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DCFG_H
#define DCFG_H

#if defined(CONFIG_CHASSIS_3_2)
#include <dcfg_lsch3.h>
#endif

#ifdef NXP_GUR_BE
#define gur_in32(a)		bswap32(mmio_read_32((uintptr_t)(a)))
#define gur_out32(a, v)		mmio_write_32((uintptr_t)(a), bswap32(v))
#elif defined(NXP_DDR_LE)
#define gur_in32(a)		mmio_read_32((uintptr_t)(a))
#define gur_out32(a, v)		mmio_write_32((uintptr_t)(a), v)
#else
#error Please define CCSR GUR register endianness
#endif

#define CHECK_SEC_DISABLED(x) ((gur_in32((x) + DCFG_SVR_OFFSET))\
				& SVR_SEC_MASK)
typedef struct {
	bool is_populated;
	uint8_t mfr_id;
#if defined(CONFIG_CHASSIS_3_2)
	uint8_t family;
	uint8_t dev_id;
#endif
	uint8_t personality;
	uint8_t sec_enabled;
	uint8_t maj_ver;
	uint8_t min_ver;
} soc_info_t;

typedef struct {
	bool is_populated;
#if defined(CONFIG_CHASSIS_3_2)
	uint8_t ocram_present;
	uint8_t ddrc1_present;
	uint8_t ddrc2_present;
#endif
} devdisr5_info_t;


struct sysinfo {
	unsigned long freq_platform;
	unsigned long freq_ddr_pll0;
	unsigned long freq_ddr_pll1;
};

int get_clocks(struct sysinfo *sys,
		uintptr_t nxp_dcfg_addr,
		unsigned long nxp_sysclk_freq,
		unsigned long nxp_ddrclk_freq,
		unsigned int nxp_plat_clk_divider);

/* Read the PORSR1 register */
uint32_t read_reg_porsr1(uintptr_t nxp_dcfg_addr);
uint32_t read_saved_porsr1(void);

/*******************************************************************************
 * Returns true if secur eboot is enabled on board
 * mode = 0  (development mode - sb_en = 1)
 * mode = 1 (production mode - ITS = 1)
 ******************************************************************************/
bool check_boot_mode_secure(uint32_t *mode,
			    uintptr_t nxp_dcfg_addr,
			    uintptr_t nxp_sfp_addr);

const soc_info_t *get_soc_info(uintptr_t nxp_dcfg_addr);
const devdisr5_info_t *get_devdisr5_info(uintptr_t nxp_dcfg_addr);

#endif /*	DCFG_H	*/
