/*
 * Copyright (c) 2015, 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP Semiconductors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 *	  Ruchika Gupta <ruchika.gupta@nxp.com>
 *        Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#ifndef _SOC_H
#define	_SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <lsch2.h>

/* ARM Required MACRO's */
/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU		0x0
/* Number of cores in platform */
#define PLATFORM_CORE_COUNT		1
#define NUMBER_OF_CLUSTERS		1
#define CORES_PER_CLUSTER		1
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

/*
 * Required LS standard platform porting definitions
 * for CCI-400
 */
#define NXP_CCI_CLUSTER0_SL_IFACE_IX	4

/* Errata */

#define NUM_OF_DDRC	1

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	1
#define NXP_UART_CLK_DIVIDER		4

 // pwr mgmt features supported in the soc-specific code:
 //   value == 0x0  the soc code does not support this feature
 //   value != 0x0  the soc code supports this feature
#define SOC_CORE_RELEASE       0x0
#define SOC_CORE_RESTART       0x0
#define SOC_CORE_OFF           0x0
#define SOC_CORE_STANDBY       0x0
#define SOC_CORE_PWR_DWN       0x0  
#define SOC_CLUSTER_STANDBY    0x0
#define SOC_CLUSTER_PWR_DWN    0x0  
#define SOC_SYSTEM_STANDBY     0x0
#define SOC_SYSTEM_PWR_DWN     0x0 
#define SOC_SYSTEM_OFF         0x1 
#define SOC_SYSTEM_RESET       0x1 

#endif // _SOC_H
