/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 *        Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#ifndef _SOC_H
#define	_SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <lsch3.h>
#include <plat_gicv3.h>

/* ARM Required MACRO's */
/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU		0x0

#define SVR_LS1048A		0x870320
#define SVR_LS1084A		0x870302
#define SVR_LS1088A		0x870300
#define SVR_LS1044A		0x870322
#define SVR_WO_E		0xFFFFFE
/* Number of cores in platform */
#define PLATFORM_CORE_COUNT		 8	
#define NUMBER_OF_CLUSTERS		 2
#define CORES_PER_CLUSTER	 	 4

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

#define NXP_EPU_ADDR			0x700060000
#define NXP_POWMGTDCR			0x700123C20
/* CCSR space memory Map */
#define NXP_DDR_ADDR			0x01080000
/* TZC is not there in LS2088 */
#define NXP_TZC_ADDR			0x01100000
#define NXP_DCFG_ADDR			0x01E00000
#define NXP_PMU_ADDR			0x01E30000
#define NXP_RESET_ADDR			0x01E60000
#define NXP_SFP_ADDR			0x01E80000 //--check
#define NXP_SEC_REGFILE_ADDR		0x01E88000
#define NXP_RST_ADDR			0x01E88000
#define NXP_SNVS_ADDR			0x01E90000
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
#define NXP_CCI_ADDR			0x04090000

#define NXP_SMMU_ADDR			0x05000000
#define NXP_GICD_ADDR			0x06000000
#define NXP_GICR_ADDR			0x06100000
#define NXP_GICR_SGI_ADDR		0x06110000
#define NXP_CAAM_ADDR			0x08000000

 /* epu register offsets and values */
#define EPU_EPGCR_OFFSET              0x0
#define EPU_EPIMCR10_OFFSET           0x128
#define EPU_EPCTR10_OFFSET            0xa28
#define EPU_EPCCR10_OFFSET            0x828
#define EPU_EPCCR10_VAL               0xf2800000
#define EPU_EPIMCR10_VAL              0xba000000
#define EPU_EPCTR10_VAL               0x0
#define EPU_EPGCR_VAL                 (1 << 31)

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
#define PMU_IPPDEXPCR0_OFFSET         0x4040
#define PMU_IPPDEXPCR1_OFFSET         0x4044
#define PMU_IPPDEXPCR2_OFFSET         0x4048
#define PMU_IPPDEXPCR3_OFFSET         0x404C
#define PMU_IPPDEXPCR4_OFFSET         0x4050
#define PMU_IPPDEXPCR5_OFFSET         0x4054
#define PMU_IPSTPCR0_OFFSET           0x4120
#define PMU_IPSTPCR1_OFFSET           0x4124
#define PMU_IPSTPCR2_OFFSET           0x4128
#define PMU_IPSTPCR3_OFFSET           0x412C
#define PMU_IPSTPCR4_OFFSET           0x4130
#define PMU_IPSTPCR5_OFFSET           0x4134
#define PMU_IPSTPCR6_OFFSET           0x4138
#define PMU_IPSTPACK0_OFFSET          0x4140
#define PMU_IPSTPACK1_OFFSET          0x4144
#define PMU_IPSTPACK2_OFFSET          0x4148
#define PMU_IPSTPACK3_OFFSET          0x414C
#define PMU_IPSTPACK4_OFFSET          0x4150
#define PMU_IPSTPACK5_OFFSET          0x4154
#define PMU_IPSTPACK6_OFFSET          0x4158
#define PMU_POWMGTCSR_VAL             (1 << 20)
#define PMU_IDLE_CLUSTER_MASK         0x0
#define PMU_FLUSH_CLUSTER_MASK        0x0
#define PMU_IDLE_CORE_MASK            0xfe

#define IPPDEXPCR0_MASK               0xFFFFFFFF
#define IPPDEXPCR1_MASK               0xFFFFFFFF
#define IPPDEXPCR2_MASK               0xFFFFFFFF
#define IPPDEXPCR3_MASK               0xFFFFFFFF
#define IPPDEXPCR4_MASK               0xFFFFFFFF
#define IPPDEXPCR5_MASK               0xFFFFFFFF

#define IPPDEXPCR_FLX_TMR        0x00004000    // DEVDISR5_FLX_TMR
#define DEVDISR5_FLX_TMR         0x00004000

#define IPSTPCR0_VALUE            0x0041310C
#define IPSTPCR1_VALUE            0x000003FF
#define IPSTPCR2_VALUE            0x00013006
#if 0// Dont' stop UART
#define IPSTPCR3_VALUE            0x0000033E
#else
#define IPSTPCR3_VALUE            0x0000033A
#endif
#define IPSTPCR4_VALUE            0x00103300
#define IPSTPCR5_VALUE            0x00000001
#define IPSTPCR6_VALUE            0x00000000












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

/*
 * Required LS standard platform porting definitions
 * for CCN-504 - Read from RN-F node ID register
 */
#define PLAT_CLUSTER_TO_CCN_ID_MAP 1,9,11,19

/* Errata */
#define NXP_ERRATUM_A008511
#define NXP_ERRATUM_A009803
#define NXP_ERRATUM_A009942
#define NXP_ERRATUM_A010165

#define NUM_OF_DDRC				1

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
// TBD - Check Platform Clk
#define NXP_PLATFORM_CLK_DIVIDER	1
#define NXP_UART_CLK_DIVIDER		2

 /* dcfg register offsets and values */
#define DCFG_DEVDISR1_OFFSET	0x70
#define DCFG_DEVDISR2_OFFSET	0x74
#define DCFG_DEVDISR3_OFFSET	0x78
#define DCFG_DEVDISR4_OFFSET	0x7c
#define DCFG_DEVDISR5_OFFSET	0x80
#define DCFG_DEVDISR6_OFFSET	0x84

#define DCFG_DEVDISR1_SEC   	(1 << 22)
#define DCFG_DEVDISR3_QBMAIN	(1 << 12)
#define DCFG_DEVDISR4_SPI_QSPI	(1 << 4 | 1 << 5)
#define DCFG_DEVDISR5_MEM   	(1 << 0)

#define DEVDISR1_VALUE		0x0041310c
#define DEVDISR2_VALUE		0x000003ff
#define DEVDISR3_VALUE		0x00013006
#define DEVDISR4_VALUE		0x0000033e
#define DEVDISR5_VALUE		0x00103300
#define DEVDISR6_VALUE		0x00000001

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
	3,
	4,
};
#endif

#endif // _SOC_H
