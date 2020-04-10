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
#define PLATFORM_CORE_COUNT		16
#define NUMBER_OF_CLUSTERS		 8
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
#define NXP_DDR2_ADDR			0x01090000
#define NXP_TZC_ADDR			0x01100000
#define NXP_TZC2_ADDR			0x01110000
#define NXP_TZC3_ADDR			0x01120000
#define NXP_TZC4_ADDR			0x01130000
#define NXP_DDR_PHY1_ADDR		0x01400000
#define NXP_DDR_PHY2_ADDR		0x01600000
#define NXP_DCFG_ADDR			0x01E00000
#define NXP_PMU_CCSR_ADDR		0x01E30000
#define NXP_PMU_DCSR_ADDR		0x700123000
#define NXP_PMU_ADDR                    NXP_PMU_CCSR_ADDR
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
#define NXP_GICR_SGI_ADDR		0x06210000
#define NXP_CAAM_ADDR			0x08000000

#define SA_AUX_CTRL_REG_OFFSET		0x500
#define NUM_HNI_NODE			2
#define CCN_HNI_MEMORY_MAP_SIZE		0x10000

#define PCIeRC_RN_I_NODE_ID_OFFSET	0x8
#define PoS_CONTROL_REG_OFFSET		0x0
#define POS_EARLY_WR_COMP_EN		0x20
#define HNI_POS_EN			0x01
#define POS_TERMINATE_BARRIERS		0x10
#define SERIALIZE_DEV_nGnRnE_WRITES	0x200
#define ENABLE_ERR_SIGNAL_TO_MN		0x4
#define ENABLE_RESERVE_BIT53		0x400
#define ENABLE_WUO			0x10

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
#define SVR_LX2160A		0x873601
#define SVR_LX2120A		0x873621
#define SVR_LX2080A		0x873603

/*
 * Required LS standard platform porting definitions
 * for CCN-508
 */
#define PLAT_CLUSTER_TO_CCN_ID_MAP 11,15,27,31,12,28,16,0
#define PLAT_6CLUSTER_TO_CCN_ID_MAP 11,15,27,31,12,28

/* NXP1046 version info for GIC configuration */

/* Errata */
#define NXP_ERRATUM_A011396
#define DDR_PLL_FIX

#define NXP_DDR_PHY_GEN2
#define NUM_OF_DDRC				2
#define NXP_DDR_INTLV_256B

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	2
#define NXP_UART_CLK_DIVIDER		4

/* TBD: Need to re-visited */
#define PMU_IDLE_CLUSTER_MASK         0x2
#define PMU_FLUSH_CLUSTER_MASK        0x2
#define PMU_IDLE_CORE_MASK            0xfe

 /* pmu register offsets and bitmaps */
#define PMU_POWMGTDCR0_OFFSET     0xC20
#define PMU_POWMGTCSR_OFFSET      0x4000
#define PMU_CLAINACTSETR_OFFSET   0x1100
#define PMU_CLAINACTCLRR_OFFSET   0x1104
#define PMU_CLSINACTSETR_OFFSET   0x1108
#define PMU_CLSINACTCLRR_OFFSET   0x110C
#define PMU_CLL2FLUSHSETR_OFFSET  0x1110
#define PMU_CLL2FLUSHCLRR_OFFSET  0x1114
#define PMU_IPPDEXPCR0_OFFSET     0x4040
#define PMU_IPPDEXPCR1_OFFSET     0x4044
#define PMU_IPPDEXPCR2_OFFSET     0x4048
#define PMU_IPPDEXPCR3_OFFSET     0x404C
#define PMU_IPPDEXPCR4_OFFSET     0x4050
#define PMU_IPPDEXPCR5_OFFSET     0x4054
#define PMU_IPPDEXPCR6_OFFSET     0x4058
#define PMU_IPSTPCR0_OFFSET       0x4120
#define PMU_IPSTPCR1_OFFSET       0x4124
#define PMU_IPSTPCR2_OFFSET       0x4128
#define PMU_IPSTPCR3_OFFSET       0x412C
#define PMU_IPSTPCR4_OFFSET       0x4130
#define PMU_IPSTPCR5_OFFSET       0x4134
#define PMU_IPSTPCR6_OFFSET       0x4138
#define PMU_IPSTPACKSR0_OFFSET    0x4140
#define PMU_IPSTPACKSR1_OFFSET    0x4144
#define PMU_IPSTPACKSR2_OFFSET    0x4148
#define PMU_IPSTPACKSR3_OFFSET    0x414C
#define PMU_IPSTPACKSR4_OFFSET    0x4150
#define PMU_IPSTPACKSR5_OFFSET    0x4154
#define PMU_IPSTPACKSR6_OFFSET    0x4158

#define CLAINACT_DISABLE_ACP  0xFF
#define CLSINACT_DISABLE_SKY  0xFF
#define POWMGTDCR_STP_OV_EN       0x1
#define POWMGTCSR_LPM20_REQ       0x00100000
#define DEVDISR1_MASK             0x024F3504 
#define DEVDISR2_MASK             0x0003FFFF
#define DEVDISR3_MASK             0x0000303F
#define DEVDISR4_MASK             0x0000FFFF
 /* this mask excludes the ddr and ocram memory clocks */
#define DEVDISR5_MASK             0x00F07603
#define DEVDISR5_MASK_ALL_MEM     0x00001003
#define DEVDISR5_MASK_DDR         0x00000003
#define DEVDISR5_MASK_OCRAM       0x00001000
#define DEVDISR5_MASK_DBG         0x00000400
#define DEVDISR6_MASK             0x00000001

 /* dcfg register offsets and values */
#define DCFG_DEVDISR1_OFFSET          0x70
#define DCFG_DEVDISR2_OFFSET          0x74
#define DCFG_DEVDISR3_OFFSET          0x78
#define DCFG_DEVDISR4_OFFSET          0x7C
#define DCFG_DEVDISR5_OFFSET          0x80
#define DCFG_DEVDISR6_OFFSET          0x84
#define DCFG_DEVDISR1_SEC             (1 << 22)
#define DCFG_DEVDISR3_QBMAIN          (1 << 12)
#define DCFG_DEVDISR4_SPI_QSPI        (1 << 4 | 1 << 5)

 /* CCN-508 defines */
#define CCN_HN_F_0_BASE			0x04200000
#define CCN_HN_F_1_BASE			0x04210000
#define CCN_HN_F_SAM_NODEID_MASK	0x7f
#define CCN_HN_F_SNP_DMN_CTL_OFFSET	0x200
#define CCN_HN_F_SNP_DMN_CTL_SET_OFFSET	0x210
#define CCN_HN_F_SNP_DMN_CTL_CLR_OFFSET	0x220
#define CCN_HN_F_SNP_DMN_CTL_MASK	0x80a00
#define CCN_HNF_NODE_COUNT              8
#define CCN_HNF_OFFSET                  0x10000

 /* ddr controller offsets and defines */
#define DDR_CFG_2_OFFSET                0x114
#define CFG_2_FORCE_REFRESH             0x80000000

 /* reset register bit */
#define RSTRQMR_RPTOE_MASK		(1 << 19)

#define  MPIDR_AFFINITY0_MASK   0x00FF
#define  MPIDR_AFFINITY1_MASK   0xFF00
#define  CPUECTLR_DISABLE_TWALK_PREFETCH 0x4000000000
#define  CPUECTLR_INS_PREFETCH_MASK      0x1800000000
#define  CPUECTLR_DAT_PREFETCH_MASK      0x0300000000
#define  CPUECTLR_RET_8CLK               0x2
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
