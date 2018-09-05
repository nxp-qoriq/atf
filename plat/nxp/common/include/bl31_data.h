/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 */


#ifndef _BL31_DATA_H
#define	_BL31_DATA_H

#define DATA_IN_OCRAM   0
#define DATA_IN_DDR     1
#define DATA_LOC        DATA_IN_OCRAM

#define OCRAM_BASE_ADDR      NXP_OCRAM_ADDR
#define OCRAM_SIZE_IN_BYTES  NXP_OCRAM_SIZE

/*-----------------------------------------------------------------------------
 *
 * bl31 runtime 128KB OCRAM layout
 *
 * 0x1801_FFFF --------------------------------------------------------> top of OCRAM
 *             vectors 2048 bytes
 * 0x1801_F800 ------------------------------------> EL3_VECTOR_BASE (VBAR_EL3)
 *             vector global data 16 bytes
 * 
 *             dead_loop_addr   .8byte
 *             synch_hndlr_addr .8byte
 * 
 * 0x1801_F7F0 ------------------------------------> VECTOR_GLBL_BASE
 *             smc global data 80 bytes
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
 * 0x1801_F7A0 ------------------------------------> SMC_GLBL_BASE
 *             membank data region 256 bytes 
 *             .
 *             .
 *             .
 *              -------------------------------
 *             .8byte struct.bankSizeInBytes
 *             .8byte struct.bankStartAddress
 *             .4byte struct.bankType           2nd membank struc
 *             .4byte struct.bankState
 *              -------------------------------
 *             .8byte struct.bankSizeInBytes
 *             .8byte struct.bankStartAddress
 *             .4byte struct.bankType           1st membank struc
 *             .4byte struct.bankState
 *              -------------------------------
 *             .8byte membankCount
 * 0x1800_F6A0 ------------------------------------> MEMBANK_BASE
 *             bootcore psci data 144 bytes
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
 * 0x1801_F610 ------------------------------------> BC_PSCI_BASE, BC_STACK_TOP
 *             bootcore stack 2048 bytes
 * 0x1801_EE10 -------------------------
 *             integrity buffer 16 bytes
 * 
 *             0x0000000000000000 .8byte
 *             0xFFFFFFFFFFFFFFFF .8byte
 * 0x1801_EE00 ------------------------------------> SECONDARY_TOP
 *             secondary-core psci data 144 bytes
 * 0x1801_ED70 ----------------------------------
 *             secondary-core stack    1024 bytes  per-secondary 1168 bytes (0x490)
 * 0x1801_E970 ----------------------------------
 *             integrity buffer 16 bytes
 * 0x1801_E960 ------------------------------------> STACK_BASE_ADDR
 * .
 * .
 * .
 * .
 * 0x1800_A000 --------------------------------------------------------> 
 * .
 * .                                 1st 40KB reserved for bootrom until 
 * .                                          first image is validated
 * .
 * 0x1800_7010 ------------------------------------> 
 *             EL3 page tables 28KB + 16 bytes (level 0 degenerate table)
 * 
 *             Level 0  ----> 16 bytes --> table @ 0x1800_7000 --> TTB_EL3
 *             Level 3b ----> 4KB -------> table @ 0x1800_6000
 *             Level 3a ----> 4KB -------> table @ 0x1800_5000
 *             Level 2c ----> 4KB -------> table @ 0x1800_4000
 *             Level 2b ----> 4KB -------> table @ 0x1800_3000
 *             Level 2a ----> 4KB -------> table @ 0x1800_2000
 *             Level 1b ----> 4KB -------> table @ 0x1800_1000
 *             Level 1a ----> 4KB -------> table @ 0x1800_0000
 * 0x1800_0000 --------------------------------------------------------> bottom of OCRAM
 *
 * to access secondary stack top
 * coreN_stack_top = SECONDARY_TOP - SEC_DATA_OFFSET - ((N-1) * SEC_REGION_OFFSET)
 *
 * to access secondary core data area
 * coreN_data_base = SECONDARY_TOP - SEC_DATA_OFFSET - ((N-1) * SEC_REGION_OFFSET)
 *
 *----------------------------------------------------------------------------*/

#define  INCREMENT_8KB   0x0000000000002000
#define  ALIGN_MASK_8KB  0xFFFFFFFFFFFFE000

#define SECURE_DATA_SIZE     NXP_OCRAM_SIZE

#define EL3_VECTOR_SIZE      0x800
#define VECTOR_GLBL_SIZE     0x10
#define SMC_REGION_SIZE      0x50
#define BC_PSCI_DATA_SIZE    0x90
#define BC_STACK_SIZE        0x0
#define INTEGRITY_BUFF_SIZE  0x0
#define SEC_PSCI_DATA_SIZE   0x90
#define SEC_STACK_SIZE       0x0
#define EL3_PAGE_TABLE_SIZE  0x0
#define SEC_REGION_SIZE      (INTEGRITY_BUFF_SIZE+SEC_PSCI_DATA_SIZE+SEC_STACK_SIZE)
#define SEC_REGION_OFFSET    SEC_REGION_SIZE
#define SEC_DATA_OFFSET      SEC_PSCI_DATA_SIZE
#define SECONDARY_CORE_CNT   (PLATFORM_CORE_COUNT-1)

#define EL3_DEAD_LP_OFFSET   0x0
#define EL3_SYNCH_OFFSET     0x8

#define SMC_REGION_OFFSET    (EL3_VECTOR_SIZE+VECTOR_GLBL_SIZE+SMC_REGION_SIZE)
#define BOOTLOC_OFFSET       0x0
#define BOOT_SVCS_OSET       0x8
#define PREFETCH_DIS_OFFSET  0x10  // offset to prefetch disable mask
#define LAST_SMC_GLBL_OFFSET 0x18  // must reference last smc global entry

#define SMC_MISC_OFFSET      0x20
#define SMC_TASK_OFFSET      0xC
#define TSK_START_OFFSET     0x0
#define TSK_DONE_OFFSET      0x4
#define TSK_CORE_OFFSET      0x8

 /* max size of the membank data region in bytes */
 /* Note: must be a 64-bit multiple */
#define MEMBANK_REGION_SIZE  0
#define MEMBANK_DATA_OFFSET  0x0
#define MEMBANK_CNT_SIZE     0x0
#define MEMBANK_CNT_OFFSET   0x0

 /* temporary stack space for bootcore */
#define INITIAL_BC_STACK_ADDR      (NXP_OCRAM_ADDR + NXP_OCRAM_SIZE - 16)
#define INITIAL_BC_STACK_SIZE      0x0

#if (DATA_LOC == DATA_IN_DDR)
   /* the following defs create an enumerated type for base address queries */
  #define SECURE_DATA_BASE_QUERY      0
  #define EL3_PAGE_TABLE_BASE_QUERY   1
  #define EL3_VECTOR_BASE_QUERY       2
  #define SMC_GLBL_BASE_QUERY         3
  #define SMC_TASK1_BASE_QUERY        4
  #define SMC_TASK2_BASE_QUERY        5
  #define SMC_TASK3_BASE_QUERY        6
  #define SMC_TASK4_BASE_QUERY        7
  #define MEMBANK_BASE_QUERY          8
  #define BC_PSCI_BASE_QUERY          9
  #define BC_STACK_TOP_QUERY         10
  #define SECONDARY_TOP_QUERY        11
  #define STACK_BASEADDR_QUERY       12
#else
  #define SECURE_DATA_BASE     NXP_OCRAM_ADDR
  #define EL3_PAGE_TABLE_BASE  SECURE_DATA_BASE
  #define EL3_VECTOR_BASE      (SECURE_DATA_BASE+SECURE_DATA_SIZE) - EL3_VECTOR_SIZE
  #define SMC_GLBL_BASE        ((SECURE_DATA_BASE+SECURE_DATA_SIZE) - SMC_REGION_OFFSET)
  #define SMC_TASK1_BASE       (SMC_GLBL_BASE + 32)
  #define SMC_TASK2_BASE       (SMC_TASK1_BASE + SMC_TASK_OFFSET)
  #define SMC_TASK3_BASE       (SMC_TASK2_BASE + SMC_TASK_OFFSET)
  #define SMC_TASK4_BASE       (SMC_TASK3_BASE + SMC_TASK_OFFSET)
  #define MEMBANK_BASE         (SMC_GLBL_BASE - MEMBANK_REGION_SIZE)
  #define BC_PSCI_BASE         (MEMBANK_BASE - BC_PSCI_DATA_SIZE)
  #define BC_STACK_TOP         BC_PSCI_BASE
  #define SECONDARY_TOP        (BC_STACK_TOP - (BC_STACK_SIZE + INTEGRITY_BUFF_SIZE))
  #define STACK_BASE_ADDR      (SECONDARY_TOP - (SECONDARY_CORE_CNT * SEC_REGION_SIZE))
  #define VECTOR_GLBL_BASE     (EL3_VECTOR_BASE - VECTOR_GLBL_SIZE)
#endif

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

/*----------------------------------------------------------------------------- */

 /* structure for recording memory bank data
  * in 'C' this looks like:
  * struct MemDataStruc {
  *          uint32_t  bankState,        // (0=invalid, 1=valid)
  *          uint32_t  bankType,         // (1=ddr, 2=sram, 3=peripheral)
  *          uint64_t  bankStartAddress, // physical address
  *          uint64_t  bankSizeInBytes
  *        } 
  * this structure will be populated by the platform-specific
  * ddr initialization, and consumed by the smc function SIP_MEMBANK_64
  */

 /* keep these in synch with MemDataStruc */
#define MEMBANK_DATA_SIZE      0x18  /* size in bytes of MemDataStruc */
#define MEMDATA_STATE_OFFSET   0x0
#define MEMDATA_TYPE_OFFSET    0x4
#define MEMDATA_ADDR_OFFSET    0x8
#define MEMDATA_SIZE_OFFSET    0x10

#define MEMBANK_STATE_INVALID  0x0
#define MEMBANK_STATE_VALID    0x1

#define MEMBANK_TYPE_DDR       0x1
#define MEMBANK_TYPE_SRAM      0x2
#define MEMBANK_TYPE_SPEC      0x3

#define MEMBANK_LAST           0x0
#define MEMBANK_NOT_LAST       0x8

 /* field data */
#define MEMBANK_INVALID        0x0  /* bank state bit[0] */
#define MEMBANK_VALID          0x1  /* bank state bit[0] */
#define MEMBANK_DDR            0x2  /* bank state bit[2:1] */
#define MEMBANK_SRAM           0x4  /* bank state bit[2:1] */
#define MEMBANK_SPECIAL        0x6  /* bank state bit[2:1] */
#define MEMBANK_NOT_LAST       0x8  /* bank state bit[3] */

/*----------------------------------------------------------------------------- */

#endif /* _BL31_DATA_H */
