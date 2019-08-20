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

/* ARM Required MACRO's */
/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU		0x0

/* Number of cores in platform */
#define PLATFORM_CORE_COUNT		 8	
#define NUMBER_OF_CLUSTERS		 4
#define CORES_PER_CLUSTER	 	 2

 /* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS    1

#define PLATFORM_CACHE_LINE_SHIFT	6	/* 64 byte */

#define NXP_OCRAM_SIZE			0x20000

#define NXP_QSPI_FLASH_ADDR		0x20000000
#define NXP_NOR_FLASH_ADDR		0x30000000

#define NUM_DRAM_REGIONS		2
#define	NXP_DRAM0_ADDR			0x80000000
#define NXP_DRAM0_MAX_SIZE		0x80000000	/*  2 GB  */

#define NXP_DRAM1_ADDR			0x8080000000
#define NXP_DRAM1_MAX_SIZE		0x7F80000000	/* 510 G */

/*DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

#define NXP_DCSR_ADDR			0x700000000
#define NXP_DCSR_SIZE			0x40000000

/* CCSR space memory Map */
#define NXP_DDR_ADDR			0x01080000
#define NXP_DDR2_ADDR			0x01090000
/* TZC is not there in LS2088 */
#define NXP_TZC_ADDR			0x01100000
#define NXP_TZC2_ADDR			0x01110000
#define NXP_DCFG_ADDR			0x01E00000
#define NXP_PMU_ADDR			0x01E30000
#define NXP_RESET_ADDR			0x01E60000
#define NXP_SFP_ADDR			0x01E80000 //--check
#define NXP_SEC_REGFILE_ADDR		0x01E88000 //--check
#define NXP_RST_ADDR			0x01E88000
#define NXP_SNVS_ADDR			0x01E90000
#define NXP_SCFG_ADDR			0x01FC0000
#define NXP_I2C_ADDR			0x02000000
#define NXP_QSPI_ADDR			0x020C0000
#define NXP_ESDHC_ADDR			0x02140000
#define NXP_UART_ADDR			0x021C0500
#define NXP_UART1_ADDR			0x021C0600
#define NXP_OCRAM_TZPC_ADDR		0x02200000
#define NXP_IFC_ADDR			0x02240000
#define NXP_GPIO1_ADDR			0x02300000
#define NXP_GPIO2_ADDR			0x02310000
#define NXP_GPIO3_ADDR			0x02320000
#define NXP_GPIO4_ADDR			0x02330000
#define NXP_WDOG1_TZ_ADDR		0x023C0000
#define NXP_TIMER_ADDR			0x023E0000
#define NXP_TIMER_STATUS_ADDR		0x023F0000
#define NXP_CCN_ADDR			0x04000000
#define NXP_CCN_HNI_ADDR		0x04080000
#define NXP_CCN_HN_F_0_ADDR		0x04200000
#define NXP_CCN_HN_F_1_ADDR		0x04210000
#define NXP_SMMU_ADDR			0x05000000
#define NXP_GICD_ADDR			0x06000000
#define NXP_GICR_ADDR			0x06100000
#define NXP_GICR_SGI_ADDR		0x06110000
#define NXP_CAAM_ADDR			0x08000000
#define NXP_DDR3_ADDR			0x08210000
#define NXP_EPU_ADDR			0x700060000

 /* epu register offsets and values */
#define EPU_EPGCR_OFFSET              0x0
#define EPU_EPIMCR10_OFFSET           0x128
#define EPU_EPCTR10_OFFSET            0xa28
#define EPU_EPCCR10_OFFSET            0x828
#define EPU_EPCCR10_VAL               0xb2800000
#define EPU_EPIMCR10_VAL              0xba000000
#define EPU_EPCTR10_VAL               0x0
#define EPU_EPGCR_VAL                 (1 << 31)

#define PMU_IDLE_CLUSTER_MASK         0x2
#define PMU_FLUSH_CLUSTER_MASK        0x2
#define PMU_IDLE_CORE_MASK            0xfe

 /* pmu register offsets and values */
#define PMU_PCPW20SR_OFFSET           0x830
#define PMU_CLAINACTSETR_OFFSET       0x1100
#define PMU_CLAINACTCLRR_OFFSET       0x1104
#define PMU_CLSINACTSETR_OFFSET       0x1108
#define PMU_CLSINACTCLRR_OFFSET       0x110C
#define PMU_CLL2FLUSHSETR_OFFSET      0x1110
#define PMU_CLSL2FLUSHCLRR_OFFSET     0x1114
#define PMU_CLL2FLUSHSR_OFFSET        0x1118
#define PMU_POWMGTCSR_OFFSET          0x4000
#define PMU_POWMGTCSR_VAL             (1 << 20)
#define PMU_IDLE_CORE_MASK            0xfe

#define PCIeRC_RN_I_NODE_ID_OFFSET	0x8
#define PoS_CONTROL_REG_OFFSET		0x0
#define POS_EARLY_WR_COMP_EN		0x20
#define HNI_POS_EN			0x01
#define SA_AUX_CTRL_REG_OFFSET		0x500
#define NUM_HNI_NODE			1
#define CCN_HNI_MEMORY_MAP_SIZE		0x10000
#define PORT_S0_CTRL_REG_RNI		0x010
#define PORT_S1_CTRL_REG_RNI		0x110
#define PORT_S2_CTRL_REG_RNI		0x210
#define ENABLE_WUO			0x10
#define ENABLE_FORCE_RD_QUO		0x20
#define QOS_SETTING			0x00FF000C

#define POS_TERMINATE_BARRIERS		0x10
#define SERIALIZE_DEV_nGnRnE_WRITES	0x200
#define ENABLE_ERR_SIGNAL_TO_MN		0x4
#define ENABLE_RESERVE_BIT53		0x400

#define TZPC_BLOCK_SIZE		0x1000

/* PORSR1 */
#define PORSR1_RCW_MASK		0xFF800000
#define PORSR1_RCW_SHIFT	23

/* CFG_RCW_SRC[6:0] */
#define RCW_SRC_TYPE_MASK		0x70

/* RCW SRC NOR */
#define	NOR_16B_VAL			0x20

/* RCW SRC Serial Flash
 * 1. SERAIL NOR (QSPI)
 * 2. OTHERS (SD/MMC, SPI, I2C1
 */
#define RCW_SRC_SERIAL_MASK	0x7F
#define QSPI_VAL		0x62
#define SDHC_VAL		0x40
#define EMMC_VAL		0x41

#define SVR_WO_E              0xFFFFFE
#define SVR_LS2080A             0x870110
#define SVR_LS2040A             0x870130
#define SVR_LS2088A             0x870900
#define SVR_LS2048A             0x870920
#define SVR_LS2084A             0x870910
#define SVR_LS2044A             0x870930
#define SVR_LS2081A             0x870918
#define SVR_LS2041A             0x870914

/*
 * Required LS standard platform porting definitions
 * for CCN-504 - Read from RN-F node ID register
 */
#define PLAT_CLUSTER_TO_CCN_ID_MAP 1,9,11,19
#define PLAT_2CLUSTER_TO_CCN_ID_MAP 1,9

/* Errata */
#define NXP_ERRATUM_A008511
#define NXP_ERRATUM_A009663
#define NXP_ERRATUM_A009803
#define NXP_ERRATUM_A009942
#define NXP_ERRATUM_A010165

#define NUM_OF_DDRC	2
#define NXP_DDR_INTLV_256B

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	2
#define NXP_UART_CLK_DIVIDER		2

 /* dcfg register offsets and values */
#define DCFG_DEVDISR1_OFFSET          0x70
#define DCFG_DEVDISR3_OFFSET          0x78
#define DCFG_DEVDISR4_OFFSET          0x7c
#define DCFG_DEVDISR1_SEC             (1 << 22)
#define DCFG_DEVDISR3_QBMAIN          (1 << 12)
#define DCFG_DEVDISR4_SPI_QSPI        (1 << 4 | 1 << 5)

#define CCI_HN_F_0_BASE			0x04200000
#define CCI_HN_F_1_BASE			0x04210000
#define CCN_HN_F_SAM_NODEID_MASK	0x7f
#define CCN_HN_F_SAM_NODEID_DDR0	0x4
#define CCN_HN_F_SAM_NODEID_DDR1  	0xe
#define CCN_HN_F_SNP_DMN_CTL_OFFSET	0x200
#define CCN_HN_F_SNP_DMN_CTL_SET_OFFSET	0x210
#define CCN_HN_F_SNP_DMN_CTL_CLR_OFFSET	0x220
#define CCN_HN_F_SNP_DMN_CTL_MASK	0x80a00

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

#endif // _SOC_H
