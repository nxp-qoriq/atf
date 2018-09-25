/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 *	Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#ifndef _SOC_H
#define	_SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <lsch3.h>

/* ARM Required MACRO's */
/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU		0x0
/* Number of cores in platform */
#define PLATFORM_CORE_COUNT		16
#define NUMBER_OF_CLUSTERS		8
#define CORES_PER_CLUSTER		2
 /* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS    1

#define PLATFORM_CACHE_LINE_SHIFT	6	/* 64 byte */

#define NXP_OCRAM_SIZE			0x40000

#define NXP_FLEXSPI_FLASH_ADDR		0x20000000

#define NUM_DRAM_REGIONS		3
#define	NXP_DRAM0_ADDR			0x80000000
#define NXP_DRAM0_MAX_SIZE		0x80000000	/*  2 GB  */

#define NXP_DRAM1_ADDR			0x2080000000
#define NXP_DRAM1_MAX_SIZE		0x1F80000000	/* 126 G */

#define NXP_DRAM2_ADDR			0x6000000000
#define NXP_DRAM2_MAX_SIZE		0x2000000000	/* 128G */

/*DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

/* CCSR space memory Map */
#define NXP_DDR_ADDR			0x01080000
#define NXP_DDR2_ADDR			0x01090000
#define NXP_TZC_ADDR			0x01100000
#define NXP_TZC2_ADDR			0x01110000
#define NXP_TZC3_ADDR			0x01120000
#define NXP_TZC4_ADDR			0x01130000
#define NXP_DDR_PHY1_ADDR		0x01400000
#define NXP_DDR_PHY2_ADDR		0x01600000
#define NXP_DCFG_ADDR			0x01E00000
#define NXP_PMU_ADDR			0x01E30000
#define NXP_RESET_ADDR			0x01E60000
#define NXP_SFP_ADDR			0x01E80000
#define NXP_SEC_REGFILE_ADDR		0x01E88000
#define NXP_RST_ADDR			0x01E88000
#define NXP_SNVS_ADDR			0x01E90000
#define NXP_SCFG_ADDR			0x01FC0000
#define NXP_I2C_ADDR			0x02000000
#define NXP_FLEXSPI_ADDR		0x020C0000
#define NXP_ESDHC_ADDR			0x02140000
#define NXP_ESDHC2_ADDR			0x02150000
#define NXP_UART_ADDR			0x021C0000
#define NXP_UART1_ADDR			0x021D0000
#define NXP_OCRAM_TZPC_ADDR		0x02200000
#define NXP_GPIO1_ADDR			0x02300000
#define NXP_GPIO2_ADDR			0x02310000
#define NXP_GPIO3_ADDR			0x02320000
#define NXP_GPIO4_ADDR			0x02330000
#define NXP_WDOG1_NS_ADDR		0x02390000
#define NXP_WDOG2_NS_ADDR		0x023A0000
#define NXP_WDOG1_TZ_ADDR		0x023B0000
#define NXP_WDOG2_TZ_ADDR		0x023C0000
#define NXP_TIMER_ADDR			0x023E0000
#define NXP_TIMER_STATUS_ADDR		0x023F0000
#define NXP_CCN_ADDR			0x04000000
#define NXP_CCN_HNI_ADDR		0x04080000
#define NXP_CCN_HN_F_0_ADDR		0x04200000
#define NXP_CCN_HN_F_1_ADDR		0x04210000
#define NXP_SMMU_ADDR			0x05000000
#define NXP_GICD_ADDR			0x06000000
#define NXP_GICR_ADDR			0x06200000
#define NXP_CAAM_ADDR			0x08000000

#define SA_AUX_CTRL_REG_OFFSET		0x500
#define NUM_HNI_NODE			2
#define CCN_HNI_MEMORY_MAP_SIZE		0x10000

#define POS_TERMINATE_BARRIERS		0x10
#define SERIALIZE_DEV_nGnRnE_WRITES	0x200
#define ENABLE_ERR_SIGNAL_TO_MN		0x4

#define TZPC_BLOCK_SIZE		0x1000

/* PORSR1 */
#define PORSR1_RCW_MASK		0x07800000
#define PORSR1_RCW_SHIFT	23

#define SDHC1_VAL		0x8
#define SDHC2_VAL		0x9
#define I2C1_VAL		0xa
#define FLEXSPI_NAND2K_VAL	0xc
#define FLEXSPI_NAND4K_VAL	0xd
#define FLEXSPI_NOR		0xf

/* bits */
/* SVR Definition */
#define SVR_WO_E		0xFFFFFE
#define SVR_LX2160A		0x873600
//#define SVR_LX2160AE		0x879200

/*
 * Required LS standard platform porting definitions
 * for CCN-508
 */
#define PLAT_CLUSTER_TO_CCN_ID_MAP 0,11,12,15,16,27,28,31

/* NXP1046 version info for GIC configuration */

/* Errata */
#define NXP_ERRATUM_A011396

#define NUM_OF_DDRC				2
#define NXP_DDR_INTLV_256B

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	2
#define NXP_UART_CLK_DIVIDER		4

 /* pwr mgmt features supported in the soc-specific code:
  *   value == 0x0, the soc code does not support this feature
  *   value != 0x0, the soc code supports this feature
  */
#define SOC_CORE_RELEASE      0x1
#define SOC_CORE_RESTART      0x0
#define SOC_CORE_OFF          0x0
#define SOC_CORE_STANDBY      0x0
#define SOC_CORE_PWR_DWN      0x0
#define SOC_CLUSTER_STANDBY   0x0
#define SOC_CLUSTER_PWR_DWN   0x0  
#define SOC_SYSTEM_STANDBY    0x0
#define SOC_SYSTEM_PWR_DWN    0x0 
#define SOC_SYSTEM_OFF        0x0 
#define SOC_SYSTEM_RESET      0x1 

/* Platform security policy related defines */

#endif // _SOC_H
