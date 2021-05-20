/*
 * Copyright (c) 2015, 2016 Freescale Semiconductor, Inc.
 * Copyright 2019-2020 NXP Semiconductors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 *	Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#ifndef _SOC_H
#define	_SOC_H

/* Chassis specific defines */
#include <lsch2.h>
#include <plat_gicv2.h>

/* ARM Required MACRO's */
/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU		0x0

/* Number of cores in platform */
#define PLATFORM_CORE_COUNT		4
#define NUMBER_OF_CLUSTERS		1
#define CORES_PER_CLUSTER		4

 /* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS    1

/* DDR Regions Info */
#define NUM_DRAM_REGIONS	3
#define	NXP_DRAM0_ADDR		0x80000000
#define NXP_DRAM0_MAX_SIZE	0x80000000	/*  2 GB  */
#define	NXP_DRAM1_ADDR		0x880000000
#define NXP_DRAM1_MAX_SIZE	0x780000000	/* 30 GB  */
#define	NXP_DRAM2_ADDR		0x8800000000
#define NXP_DRAM2_MAX_SIZE	0x7800000000	/* 480 GB */
/*DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE		PLAT_DEF_DRAM0_SIZE

/* SVR Definition */
#define SVR_WO_E		0xFFFFFE
#define SVR_LA1246 		0x879204
#define SVR_LA1246AE		0x879200
/*
 * Required LS standard platform porting definitions
 * for CCI-400
 */
#define NXP_CCI_CLUSTER0_SL_IFACE_IX	4

/* NXP1046 version info for GIC configuration */
#define REV1_0			0x10
#define REV1_1			0x11
#define GIC_ADDR_BIT		31

/* Errata */
#define NXP_ERRATUM_A008511
#define NXP_ERRATUM_A009803
#define NXP_ERRATUM_A009942
#define NXP_ERRATUM_A010165

#define NUM_OF_DDRC	1

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	1
#define NXP_UART_CLK_DIVIDER		2

 /* set this switch to 1 if you need to keep the debug block
  * clocked during system power-down
  */
#define DEBUG_ACTIVE  0

 /* pwr mgmt features supported in the soc-specific code:
  *   value == 0x0  the soc code does not support this feature
  *   value != 0x0  the soc code supports this feature
  */
#define SOC_CORE_RELEASE       0x1
#define SOC_CORE_RESTART       0x1
#define SOC_CORE_OFF           0x1
#define SOC_CORE_STANDBY       0x1
#define SOC_CORE_PWR_DWN       0x1  
#define SOC_CLUSTER_STANDBY    0x1
#define SOC_CLUSTER_PWR_DWN    0x1  
#define SOC_SYSTEM_STANDBY     0x1
#define SOC_SYSTEM_PWR_DWN     0x1
#define SOC_SYSTEM_OFF         0x1 
#define SOC_SYSTEM_RESET       0x1 

 /* PSCI-specific defines */
#define SYSTEM_PWR_DOMAINS 1
#define PLAT_NUM_PWR_DOMAINS   (PLATFORM_CORE_COUNT + \
				NUMBER_OF_CLUSTERS  + \
				SYSTEM_PWR_DOMAINS)

 /* Power state coordination occurs at the system level */
#define PLAT_PD_COORD_LVL MPIDR_AFFLVL2
#define PLAT_MAX_PWR_LVL  PLAT_PD_COORD_LVL

 /* Local power state for power domains in Run state */
#define LS_LOCAL_STATE_RUN  PSCI_LOCAL_STATE_RUN

 /* define retention state */
#define PLAT_MAX_RET_STATE  (PSCI_LOCAL_STATE_RUN + 1)
#define LS_LOCAL_STATE_RET  PLAT_MAX_RET_STATE

 /* define power-down state */
#define PLAT_MAX_OFF_STATE  (PLAT_MAX_RET_STATE + 1)
#define LS_LOCAL_STATE_OFF  PLAT_MAX_OFF_STATE

 /* Some data must be aligned on the biggest cache line size in the platform.
  * This is known only to the platform as it might have a combination of
  * integrated and external caches.
  */
#define CACHE_WRITEBACK_GRANULE	 (1 << 6)

 /* One cache line needed for bakery locks on ARM platforms */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE (1 * CACHE_WRITEBACK_GRANULE)

#ifndef __ASSEMBLER__
/* CCI slave interfaces */
static const int cci_map[] = {
	NXP_CCI_CLUSTER0_SL_IFACE_IX,
};
#endif

#endif // _SOC_H
