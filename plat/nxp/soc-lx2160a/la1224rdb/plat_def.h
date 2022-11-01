/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_DEF_H
#define PLAT_DEF_H

#include <arch.h>
#include <cortex_a72.h>
/* Required without TBBR.
 * To include the defines for DDR PHY
 * Images.
 */
#include <tbbr_img_def.h>

#include <policy.h>
#include <soc.h>

#if defined(IMAGE_BL31)
#define LS_SYS_TIMCTL_BASE		0x2890000
#define PLAT_LS_NSTIMER_FRAME_ID	0
#define LS_CONFIG_CNTACR		1
#endif
/* Special value used to verify platform parameters from BL2 to BL31 */

#define LS_BL31_PLAT_PARAM_VAL      0x0f1e2d3c4b5a6978ULL
#define PLATFORM_LINKER_ARCH            aarch64
#define PLATFORM_LINKER_FORMAT      "elf64-littleaarch64"

#define NXP_SYSCLK_FREQ		100000000
#define NXP_DDRCLK_FREQ		100000000

/* UART related definition */
#define NXP_CONSOLE_ADDR	NXP_UART_ADDR
#define NXP_CONSOLE_BAUDRATE	115200

#define NXP_SPD_EEPROM0     0x51

#define PLAT_DEF_DRAM0_SIZE 0x80000000  /*  2G */

/* Size of cacheable stacks */
#if defined(IMAGE_BL2)
#if defined(TRUSTED_BOARD_BOOT)
#define PLATFORM_STACK_SIZE	0x2000
#else
#define PLATFORM_STACK_SIZE	0x1000
#endif
#elif defined(IMAGE_BL31)
#define PLATFORM_STACK_SIZE	0x1000
#endif

/* SD block buffer */
#define NXP_SD_BLOCK_BUF_ADDR	(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE \
				- NXP_SD_BLOCK_BUF_SIZE)

#ifdef SD_BOOT
#define BL2_LIMIT		(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE \
				- NXP_SD_BLOCK_BUF_SIZE)
#else
#define BL2_LIMIT		(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE)
#endif
#define BL2_TEXT_LIMIT      (BL2_LIMIT)

/* 2 MB reserved in secure memory for DDR */
#define BL31_BASE       NXP_SECURE_DRAM_ADDR
#define BL31_SIZE       (0x200000)
#define BL31_LIMIT      (BL31_BASE + BL31_SIZE)

/* Put BL32 in secure memory */
#define BL32_BASE       (NXP_SECURE_DRAM_ADDR + BL31_SIZE)
#define BL32_LIMIT      (NXP_SECURE_DRAM_ADDR + \
				NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE)

/* BL33 memory region */
/* Hardcoded based on current address in u-boot */
#define BL33_BASE       0x82000000
#define BL33_LIMIT      (NXP_NS_DRAM_ADDR + NXP_NS_DRAM_SIZE)

/* SD block buffer */
#define NXP_SD_BLOCK_BUF_SIZE   (0xC000)
#define NXP_SD_BLOCK_BUF_ADDR   (NXP_OCRAM_ADDR + NXP_OCRAM_SIZE \
				- NXP_SD_BLOCK_BUF_SIZE)


/* IO defines as needed by IO driver framework */
#define MAX_IO_DEVICES		4
#define MAX_IO_BLOCK_DEVICES	1
#define MAX_IO_HANDLES		4

#define PHY_GEN2_FW_IMAGE_BUFFER	(NXP_OCRAM_ADDR + CSF_HDR_SZ)

/*
 * FIP image defines - Offset at which FIP Image would be present
 * Image would include Bl31 , Bl33 and Bl32 (optional)
 */
#ifdef POLICY_FUSE_PROVISION
#define MAX_FIP_DEVICES		3
#define FUSE_BUF        ULL(0x81000000)
#define FUSE_SZ         0x80000
#endif

#ifndef MAX_FIP_DEVICES
#define MAX_FIP_DEVICES		2
#endif

#define PLAT_FIP_OFFSET     0x100000
#define PLAT_FIP_MAX_SIZE   0x400000

/* Check if this size can be determined from array size */
#if defined(IMAGE_BL2)
#define MAX_MMAP_REGIONS    8
#define MAX_XLAT_TABLES     6
#elif defined(IMAGE_BL31)
#define MAX_MMAP_REGIONS    9
#define MAX_XLAT_TABLES     9
#elif defined(IMAGE_BL32)
#define MAX_MMAP_REGIONS    8
#define MAX_XLAT_TABLES     9
#endif
/*
 * ID of the secure physical generic timer interrupt used by the BL32.
 */
#define BL32_IRQ_SEC_PHY_TIMER	29

#define BL31_WDOG_SEC		89

#define BL31_NS_WDOG_WS1	108

/*
 * Define properties of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_LS_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(BL32_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

/* SGI 15 and Secure watchdog interrupts assigned to Group 0 */
#define NXP_IRQ_SEC_SGI_7		15

#define PLAT_LS_G0_IRQ_PROPS(grp)	\
	INTR_PROP_DESC(BL31_WDOG_SEC, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(BL31_NS_WDOG_WS1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(NXP_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)
#endif
