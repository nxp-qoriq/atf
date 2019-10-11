/*
 * Copyright 2018-2019 NXP
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
#include <plat_gicv3.h>

#define SVR_LS1017A		0x870B24
#define SVR_LS1018A		0x870B20
#define SVR_LS1027A		0x870B04
#define SVR_LS1028A		0x870B00
#define SVR_WO_E		0xFFFFFE

/* ARM Required MACRO's */
/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU		0x0

/* Number of cores in platform */
#define PLATFORM_CORE_COUNT		 2
#define NUMBER_OF_CLUSTERS		 1
#define CORES_PER_CLUSTER	 	 2

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

#define NXP_DCSR_ADDR			0x700000000
#define NXP_DCSR_SIZE			0x40000000

/* CCSR space memory Map */
#define NXP_DDR_ADDR			0x01080000
#define NXP_TZC_ADDR			0x01100000
#define NXP_DCFG_ADDR			0x01E00000
#define NXP_PMU_ADDR			0x01E30000
#define NXP_EPU_ADDR			0x700060000
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
#define NXP_UART_ADDR			0x021C0500
#define NXP_UART1_ADDR			0x021C0600
#define NXP_OCRAM_TZPC_ADDR		0x02200000
#define NXP_GPIO1_ADDR			0x02300000
#define NXP_GPIO2_ADDR			0x02310000
#define NXP_GPIO3_ADDR			0x02320000
#define NXP_WDOG1_TZ_ADDR		0x023C0000
#define NXP_TIMER_ADDR			0x023E0000
#define NXP_TIMER_STATUS_ADDR		0x023F0000
#define NXP_CCI_ADDR			0x04090000
#define NXP_SMMU_ADDR			0x05000000
#define NXP_GICD_ADDR			0x06000000
#define NXP_GICR_ADDR			0x06040000
#define NXP_GICR_SGI_ADDR		0x06050000
#define NXP_CAAM_ADDR			0x08000000

#define TZPC_BLOCK_SIZE		0x1000
 /* epu register offsets and values */
#define EPU_EPGCR_OFFSET              0x0
#define EPU_EPIMCR10_OFFSET           0x128
#define EPU_EPCTR10_OFFSET            0xa28
#define EPU_EPCCR10_OFFSET            0x828
#define EPU_EPCCR10_VAL               0xb2800000
#define EPU_EPIMCR10_VAL              0xba000000
#define EPU_EPCTR10_VAL               0x0
#define EPU_EPGCR_VAL                 (1 << 31)

#define PMU_IDLE_CLUSTER_MASK         0x0
#define PMU_FLUSH_CLUSTER_MASK        0x0
#define PMU_IDLE_CORE_MASK            0x2

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

/*
 * Required LS standard platform porting definitions
 * for CCI-400
 */
#define NXP_CCI_CLUSTER0_SL_IFACE_IX	4

/* Errata */
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

 /* dcfg register offsets and values */
#define DCFG_DEVDISR1_OFFSET          0x70
#define DCFG_DEVDISR2_OFFSET          0x74
#define DCFG_DEVDISR4_OFFSET          0x7c
#define DCFG_DEVDISR1_SEC             (1 << 22)
#define DCFG_DEVDISR4_SPI_QSPI        (1 << 4 | 1 << 5)
#define DCFG_DEVDISR2_ENETC           (1 << 31)

 /* reset register bit */
#define RSTRQMR_RPTOE_MASK		(1 << 19)

#define  MPIDR_AFFINITY0_MASK   0x00FF
#define  MPIDR_AFFINITY1_MASK   0xFF00
#define  CPUECTLR_DISABLE_TWALK_PREFETCH 0x4000000000
#define  CPUECTLR_INS_PREFETCH_MASK      0x1800000000
#define  CPUECTLR_DAT_PREFETCH_MASK      0x0300000000
#define  OSDLR_EL1_DLK_LOCK              0x1
#define  CNTP_CTL_EL0_EN                 0x1
#define  CNTP_CTL_EL0_IMASK              0x2

#define TPMWAKEMR0_ADDR		0x700123c50

 /* pwr mgmt features supported in the soc-specific code:
  *   value == 0x0, the soc code does not support this feature
  *   value != 0x0, the soc code supports this feature
  */
#define SOC_CORE_RELEASE      0x1
#define SOC_CORE_RESTART      0x1
#define SOC_CORE_OFF          0x1
#define SOC_CORE_STANDBY      0x1
#define SOC_CORE_PWR_DWN      0x1
#define SOC_CLUSTER_STANDBY   0x1
#define SOC_CLUSTER_PWR_DWN   0x1
#define SOC_SYSTEM_STANDBY    0x1
#define SOC_SYSTEM_PWR_DWN    0x1
#define SOC_SYSTEM_OFF        0x1
#define SOC_SYSTEM_RESET      0x1

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
