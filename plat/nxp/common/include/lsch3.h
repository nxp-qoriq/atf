/*
 * Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author : York Sun <york.sun@nxp.com>
 *	Ruchika Gupta <ruchika.gupta@nxp.com>
 */

#ifndef __LSCH3_H_
#define __LSCH3_H_

/* CCSR */
#define NXP_CCSR_ADDR			0x1000000
#define NXP_CCSR_SIZE			0xF000000

#define NXP_GPIO1_ADDR			0x02300000
#define NXP_GPIO2_ADDR			0x02310000
#define NXP_GPIO3_ADDR			0x02320000
#define NXP_GPIO4_ADDR			0x02330000

/* OCRAM */
#define NXP_OCRAM_ADDR			0x18000000

 // dcfg block register offsets and bitfields
#define DCFG_PORSR1_OFFSET		0x00
#define DCFG_COREDISR_OFFSET		0x94
#define DCFG_SCRATCHRW5_OFFSET		0x210
#define DCFG_SCRATCHRW6_OFFSET		0x214
#define DCFG_SCRATCHRW7_OFFSET		0x218
#define DCFG_SVR_OFFSET			0x0A4
#define DCFG_COREDISABLEDSR_OFFSET	0x990
#define DCFG_BOOTLOCPTRL_OFFSET		0x400
#define DCFG_BOOTLOCPTRH_OFFSET		0x404
#define RCW_SR1_OFFSET          	0x100
#define RCW_SR27_OFFSET         	0x168
#define RCWSR0_OFFSET			0x0100
#define RCWSR5_OFFSET			0x110

/* DCFG regsiters bit masks */
#define RCWSR0_SYS_PLL_RAT_SHIFT	2
#define RCWSR0_SYS_PLL_RAT_MASK		0x1f
#define RCWSR0_MEM_PLL_RAT_SHIFT	10
#define RCWSR0_MEM_PLL_RAT_MASK		0x3f
#define RCWSR0_MEM2_PLL_RAT_SHIFT	18
#define RCWSR0_MEM2_PLL_RAT_MASK	0x3f
#define RCWSR5_SBEN_SHIFT		21
#define RCWSR5_SBEN_MASK		0x1
#define SVR_SEC_MASK			0x100

#define INVALID_SOC_ID			0xFFFFFFFF
#define DCFG_ENDIANNESS_MASK		0xFF000000
#define SOC_ID_SHIFT			0x8
#define NXP_MANUFACTURER_ID		0x87000000

/* PMU Registers */
#define CLUST_TIMER_BASE_ENBL_OFFSET	0x18A0

 /*  reset block register offsets */
#define RST_RSTCR_OFFSET		0x0
#define RST_RSTRQMR1_OFFSE		0x10
#define RST_RSTRQSR1_OFFSET		0x18
#define BRR_OFFSET			0x60

 /* secure register file offsets */
#define CORE_HOLD_OFFSET		0x140

 /* secmon register offsets and bitfields */
#define SECMON_HPCOMR_OFFSET		0x4
#define SECMON_HPCOMR_NPSWAEN		0x80000000

 /* System Counter Offset and Bit Mask */
#define SYS_COUNTER_CNTCR_OFFSET	0x0
#define SYS_COUNTER_CNTCR_EN		0x00000001

#define TIMER_CNTCR_OFFSET		0x0

/* GPIO Register offset */
#define GPDIR_REG_OFFSET                0x0
#define GPDAT_REG_OFFSET                0x8

 /* timer control bitfields */
#define CNTCR_EN_MASK			0x1
#define CNTCR_DIS			0x0

 /* bit masks */
#define RSTCR_RESET_REQ			0x2
#define RSTRQSR1_SWRR			0x800

#define GPIO_SEL_MASK			0x7F
#define GPIO_BIT_MASK			0x1F

 // secure register file offsets and bit masks
#define RSTCNTL_OFFSET        0x180
#define SW_RST_REQ_INIT       0x1

/* TBD York - Check if these can be obtained from ARM CCN driver */
#define CCN_HN_F_SAM_CTL		0x8
#define CCN_HN_F_SNP_DMN_CTL_OFFSET	0x200
#define CCN_HN_F_SNP_DMN_CTL_SET_OFFSE	0x210
#define CCN_HN_F_SNP_DMN_CTL_CLR_OFFSE	0x220
#define CCN_HN_F_SNP_DMN_CTL_MASK	0x80a00

#define CAAM_JR0_OFFSET			0x10000
#define CAAM_JR1_OFFSET			0x20000
#define CAAM_JR2_OFFSET			0x30000
#define CAAM_JR3_OFFSET			0x40000

/* Endianness of IP's */
#define NXP_GUR_LE
#define NXP_DDR_LE
#define NXP_SEC_LE
#define NXP_SFP_LE
#define NXP_SFP_VER_3_4
#define NXP_SNVS_LE
#define NXP_ESDHC_LE

#define CONFIG_PHYS_64BIT

#ifndef __ASSEMBLER__
#include <endian.h>

#define FETCH_SOC_ID(x) ((x & DCFG_ENDIANNESS_MASK) == NXP_MANUFACTURER_ID) ?\
			(le32toh(x) >> SOC_ID_SHIFT) : (INVALID_SOC_ID)

#define CHECK_SEC_DISABLED ((mmio_read_32(NXP_DCFG_ADDR + DCFG_SVR_OFFSET))\
				& SVR_SEC_MASK)

#endif

#endif /* __LSCH3_H_ */
