/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 */


#ifndef _BL31_DATA_H
#define	_BL31_DATA_H

#define OCRAM_BASE_ADDR      NXP_OCRAM_ADDR
#define OCRAM_SIZE_IN_BYTES  NXP_OCRAM_SIZE

/*-----------------------------------------------------------------------------
 *
 * bl31 runtime 128KB OCRAM layout
 *
 * 0x1801_FFFF --------------------------------------------------------> top of OCRAM
 *             smc global data 0x80 bytes
 * 
 *                 core_mask   .4byte
 *                 end_flag    .4byte
 *                 begin_flag  .4byte
 *             task4_base -------------------------> TASK4_BASE
 *                 core_mask   .4byte
 *                 end_flag    .4byte
 *                 begin_flag  .4byte
 *             task3_base -------------------------> TASK3_BASE
 *                 core_mask   .4byte
 *                 end_flag    .4byte
 *                 begin_flag  .4byte
 *             task2_base -------------------------> TASK2_BASE
 *                 core_mask   .4byte
 *                 end_flag    .4byte
 *                 begin_flag  .4byte
 *             task1_base -------------------------> TASK1_BASE
 * 
 *             reserved        .8byte
 *             pref_dis_mask   .8byte  // prefetch disable mask
 *             boot_svcs_flag  .8byte
 *             bootlocptr      .8byte
 * 0x1801_FF80 ------------------------------------> SMC_GLBL_BASE
 *             bootcore psci data 0xC0 bytes
 * 
 *             hcr_el2         .8byte
 *             scr_el3         .8byte
 *             AUX_07          .8byte - soc-specific storage
 *             AUX_06          .8byte - soc-specific storage
 *             AUX_05          .8byte - soc-specific storage
 *             AUX_04          .8byte - soc-specific storage
 *             AUX_03          .8byte - soc-specific storage
 *             AUX_02          .8byte - soc-specific storage
 *             AUX_01          .8byte - soc-specific storage
 *             cpuectlr        .8byte
 *             sctlr_elx       .8byte
 *             abort_flag      .8byte
 *             gicc_ctlr       .8byte
 *             link_addr       .8byte
 *             start_addr      .8byte
 *             context_id      .8byte
 *             spsr_el3        .8byte
 *             core_state      .8byte
 * 0x1801_FEC0 ------------------------------------> BC_PSCI_BASE, SECONDARY_TOP
 *             secondary-core psci data 0xC0 bytes per core
 * 0x1801_FE00 ---------------------------------- core 1
 *             secondary-core psci data 0xC0 bytes per core
 * 0x1801_FD40 ---------------------------------- core 2
 *             secondary-core psci data 0xC0 bytes per core
 * .
 * .
 * .
 * .
 * to access secondary core data area
 * coreN_data_base = SECONDARY_TOP - (N * SEC_REGION_SIZE)
 *
 * 0x1800_0000 --------------------------------------------------------> bottom of OCRAM
 *
 *----------------------------------------------------------------------------*/

#define SECURE_DATA_BASE     NXP_OCRAM_ADDR
#define SECURE_DATA_SIZE     NXP_OCRAM_SIZE
#define SECURE_DATA_TOP      (SECURE_DATA_BASE + SECURE_DATA_SIZE)
#define SMC_REGION_SIZE      0x80
#define SMC_GLBL_BASE        (SECURE_DATA_TOP - SMC_REGION_SIZE)
#define BC_PSCI_DATA_SIZE    0xC0
#define BC_PSCI_BASE         (SMC_GLBL_BASE - BC_PSCI_DATA_SIZE)
#define SECONDARY_TOP        BC_PSCI_BASE

#define SEC_PSCI_DATA_SIZE   0xC0
#define SEC_REGION_SIZE      SEC_PSCI_DATA_SIZE

/* SMC global data */
#define BOOTLOC_OFFSET       0x0
#define BOOT_SVCS_OSET       0x8
#define PREFETCH_DIS_OFFSET  0x10  // offset to prefetch disable mask
#define LAST_SMC_GLBL_OFFSET 0x18  // must reference last smc global entry

#define SMC_TASK_OFFSET      0xC
#define TSK_START_OFFSET     0x0
#define TSK_DONE_OFFSET      0x4
#define TSK_CORE_OFFSET      0x8
#define SMC_TASK1_BASE       (SMC_GLBL_BASE + 32)
#define SMC_TASK2_BASE       (SMC_TASK1_BASE + SMC_TASK_OFFSET)
#define SMC_TASK3_BASE       (SMC_TASK2_BASE + SMC_TASK_OFFSET)
#define SMC_TASK4_BASE       (SMC_TASK3_BASE + SMC_TASK_OFFSET)
//-----------------------------------------------------------------------------

 /* psci data area offsets */
#define CORE_STATE_DATA    0x0
#define SPSR_EL3_DATA      0x8
#define CNTXT_ID_DATA      0x10
#define START_ADDR_DATA    0x18
#define LINK_REG_DATA      0x20
#define GICC_CTLR_DATA     0x28
#define ABORT_FLAG_DATA    0x30
#define SCTLR_DATA         0x38
#define CPUECTLR_DATA      0x40
#define AUX_01_DATA        0x48  /* usage defined per SoC */
#define AUX_02_DATA        0x50  /* usage defined per SoC */
#define AUX_03_DATA        0x58  /* usage defined per SoC */
#define AUX_04_DATA        0x60  /* usage defined per SoC */
#define AUX_05_DATA        0x68  /* usage defined per SoC */
#define AUX_06_DATA        0x70  /* usage defined per SoC */
#define AUX_07_DATA        0x78  /* usage defined per SoC */
#define SCR_EL3_DATA       0x80
#define HCR_EL2_DATA       0x88
#define AUX_08_DATA  	   0x90  /* usage defined per SoC */
#define AUX_09_DATA	   0x98  /* usage defined per SoC */
#define AUX_11_DATA  	   0xa0  /* usage defined per SoC */

#endif /* _BL31_DATA_H */
