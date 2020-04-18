/*
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __LSCH3_H_
#define __LSCH3_H_

#include <dcfg_lsch3.h>

/* CCSR */
#define NXP_CCSR_ADDR			0x1000000
#define NXP_CCSR_SIZE			0xF000000

#define NXP_GPIO1_ADDR			0x02300000
#define NXP_GPIO2_ADDR			0x02310000
#define NXP_GPIO3_ADDR			0x02320000
#define NXP_GPIO4_ADDR			0x02330000

/* OCRAM */
#define NXP_OCRAM_ADDR			0x18000000

#define NXP_WDT1_BASE			0x0C000000

#define INVALID_SOC_ID			0xFFFFFFFF
#define DCFG_ENDIANNESS_MASK		0xFF000000
#define SOC_ID_SHIFT			0x8
#define NXP_MANUFACTURER_ID		0x87000000

/* PMU Registers */
#define CORE_TIMEBASE_ENBL_OFFSET	0x8A0
#define CLUST_TIMER_BASE_ENBL_OFFSET	0x18A0

 /*  reset block register offsets */
#define RST_RSTCR_OFFSET		0x0
#define RST_RSTRQMR1_OFFSET		0x10
#define RST_RSTRQSR1_OFFSET		0x18
#define BRR_OFFSET			0x60

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

 /* secure register file offsets */
#define CORE_HOLD_OFFSET		0x140

 /* secmon register offsets and bitfields */
#define SECMON_HPCOMR_OFFSET		0x4
#define SECMON_HPCOMR_NPSWAEN		0x80000000

 /* System Counter Offset and Bit Mask */
#define SYS_COUNTER_CNTCR_OFFSET	0x0
#define SYS_COUNTER_CNTCR_EN		0x00000001

#define TIMER_CNTCR_OFFSET		0x0

 /* timer control bitfields */
#define CNTCR_EN_MASK			0x1
#define CNTCR_DIS			0x0

 /* bit masks */
#define RSTCR_RESET_REQ			0x2
#define RSTRQSR1_SWRR			0x800

#define GPIO_SEL_MASK			0x7F
#define GPIO_BIT_MASK			0x1F

/* secure register file offsets and bit masks */
#define RSTCNTL_OFFSET        0x180
#define SW_RST_REQ_INIT       0x1

/* MMU 500 */
#define SMMU_SCR0		(NXP_SMMU_ADDR + 0x0)
#define SMMU_SCR1		(NXP_SMMU_ADDR + 0x4)
#define SMMU_SCR2		(NXP_SMMU_ADDR + 0x8)
#define SMMU_SACR		(NXP_SMMU_ADDR + 0x10)
#define SMMU_IDR0		(NXP_SMMU_ADDR + 0x20)
#define SMMU_IDR1		(NXP_SMMU_ADDR + 0x24)

#define SMMU_NSCR0		(NXP_SMMU_ADDR + 0x400)
#define SMMU_NSCR2		(NXP_SMMU_ADDR + 0x408)
#define SMMU_NSACR		(NXP_SMMU_ADDR + 0x410)

#define SCR0_CLIENTPD_MASK	0x00000001
#define SCR0_USFCFG_MASK	0x00000400

#define CHS_QSPI_MCR			0x020C0000
#define CHS_QSPI_64LE			0xC

/* CCI defines */
#define CCI_TERMINATE_BARRIER_TX	0x8

#define CONFIG_PHYS_64BIT

#define NXP_QSPI_LE

#ifndef __ASSEMBLER__
#include <endian.h>

#define FETCH_SOC_ID(x) (((x) & DCFG_ENDIANNESS_MASK) == NXP_MANUFACTURER_ID) ?\
			(le32toh(x) >> SOC_ID_SHIFT) : (INVALID_SOC_ID)

#endif

#endif /* __LSCH3_H_ */
