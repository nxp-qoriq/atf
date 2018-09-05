/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#ifndef _PLAT_TZC400_H_
#define _PLAT_TZC400_H_

#include <tzc400.h>

/* Structure to configure TZC Regions' boundaries and attributes. */
struct tzc400_reg {
	uint8_t reg_filter_en;
	unsigned long long start_addr;
	unsigned long long end_addr;
	tzc_region_attributes_t sec_attr;
	unsigned int nsaid_permissions;
};

#define TZC_REGION_NS_NONE	0x00000000

/* NXP Platforms do not support NS Access ID (NSAID) based non-secure access.
 * Supports only non secure through generic NS ACCESS ID
 */
#define TZC_NS_ACCESS_ID	0xFFFFFFFF

/* 3 regions:
 *  Region 0(default),
 *  Region 1 (DRAM0, Secure Memory),
 *  Region 2 (DRAM0, Shared memory)
 */
#define MAX_NUM_TZC_REGION	NUM_DRAM_REGIONS + 3

/*
 * Typical Memory map of DRAM0
 *    |-----------NXP_NS_DRAM_ADDR ( = NXP_DRAM0_ADDR)----------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |								|
 *    |------- (NXP_NS_DRAM_ADDR + NXP_NS_DRAM_SIZE - 1) -------|
 *    |-----------------NXP_SECURE_DRAM_ADDR--------------------|
 *    |								|
 *    |								|
 *    |								|
 *    |			SECURE REGION (= 64MB)			|
 *    |								|
 *    |								|
 *    |								|
 *    |--- (NXP_SECURE_DRAM_ADDR + NXP_SECURE_DRAM_SIZE - 1)----|
 *    |-----------------NXP_SP_SHRD_DRAM_ADDR-------------------|
 *    |								|
 *    |	       Secure EL1 Payload SHARED REGION (= 2MB)         |
 *    |								|
 *    |-----------(NXP_DRAM0_ADDR + NXP_DRAM0_SIZE - 1)---------|
 *
 *
 *
 * Typical Memory map of DRAM1
 *    |---------------------NXP_DRAM1_ADDR----------------------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |---(NXP_DRAM1_ADDR + Dynamically calculated Size - 1) ---|
 *
 *
 * Typical Memory map of DRAM2
 *    |---------------------NXP_DRAM2_ADDR----------------------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |---(NXP_DRAM2_ADDR + Dynamically calculated Size - 1) ---|
 */
static struct tzc400_reg tzc400_reg_list[] = {
	{	/* Region 0 */
		1,			/* Region Enable Flag*/
		0x00000000,		/* Start Address */
		0xFFFFFFFF,             /* End Address */
		TZC_REGION_S_NONE,      /* .secure attr */
		TZC_REGION_NS_NONE      /* Non-Secure Access ID */
	},
	{	/* Region 1 */
		1,			/* Region Enable Flag*/
		0x00000000,		/* Start Address */
		0x00000000,             /* End Address */
		TZC_REGION_S_RDWR,      /* .secure attr */
		TZC_REGION_NS_NONE      /* Non-Secure Access ID */
	},
	{	/* Region 2 */
		1,			/* Region Enable Flag*/
		0x00000000,		/* Start Address */
		0x00000000,             /* End Address */
		TZC_REGION_S_RDWR,      /* .secure attr */
		TZC_NS_ACCESS_ID        /* Non-Secure Access ID */
	},
	{				/* Region 3 */
		1,			/* Region Enable Flag*/
		0x00000000,		/* Start Address */
		0x00000000,             /* End Address */
		TZC_REGION_S_RDWR,      /* .secure attr */
		TZC_NS_ACCESS_ID        /* Non-Secure Access ID */
	},
#ifdef NXP_DRAM1_ADDR
	{				/* Region 4 */
		1,			/* Region Enable Flag*/
		0x00000000,		/* Start Address */
		0x00000000,             /* End Address */
		TZC_REGION_S_NONE,      /* .secure attr */
		TZC_NS_ACCESS_ID        /* Non-Secure Access ID */
	},
#endif
#ifdef NXP_DRAM2_ADDR
	{				/* Region 5 */
		1,			/* Region Enable Flag*/
		0x00000000,		/* Start Address */
		0x00000000,             /* End Address */
		TZC_REGION_S_NONE,      /* .secure attr */
		TZC_NS_ACCESS_ID        /* Non-Secure Access ID */
	},
#endif
	{}
};

#endif /* _PLAT_TZC400_H_ */
