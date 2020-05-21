/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

.section .text, "ax"

#include <asm_macros.S>
#ifdef NXP_COINED_BB
#include <snvs.h>
#else
#include <fspi.h>
#include <flash_info.h>
#endif
#include <platform_def.h>
#include <plat_warm_rst.h>

.global _soc_sys_warm_reset

/* Function requests a reset of the entire SOC
 * in:  none
 * out: none
 * uses: x0, x1, x2, x3, x4, x5, x6
 */
func _soc_sys_warm_reset
	mov  x6, x30

#ifdef NXP_COINED_BB
	/* read the register hpcomr */
	ldr  x1, =NXP_SNVS_ADDR
	ldr  w0, [x1, #NXP_LP_GPR0_OFFSET]
	/* turn off secure access for the privileged registers */
	orr  w0, w0, #NXP_WARM_RST_FLAG
	/* write back */
	str  w0, [x1, #NXP_LP_GPR0_OFFSET]
#elif !(ERLY_WRM_RST_FLG_FLSH_UPDT)
	ldr  x1, =NXP_FLEXSPI_ADDR
	ldr  w0, [x1, #FSPI_IPCMD]
	orr  w0, w0, #FSPI_IPCMD_TRG_MASK
	str  w0, [x1, #FSPI_IPCMD]
1:
	ldr  w0, [x1, #FSPI_INTR]
	and  w0, w0, #FSPI_INTR_IPCMDDONE_MASK
	cmp  w0, #0
	b.eq 1b

	ldr  w0, [x1, #FSPI_IPTXFCR]
	orr  w0, w0, #FSPI_IPTXFCR_CLR
	str  w0, [x1, #FSPI_IPTXFCR]

	ldr  w0, [x1, #FSPI_INTR]
	orr  w0, w0, #FSPI_INTR_IPCMDDONE_MASK
	str  w0, [x1, #FSPI_INTR]

	/* flexspi driver has an api
	 * is_flash_busy().
	 * Impelementation of the api will not
	 * fit-in in 1 cache line.
	 * instead a nop-cycles are introduced to
	 * simulate the wait time for flash write
	 * completion.
	 *
	 * Note: This wait time varies from flash to flash.
	 */

	mov    x0, #FLASH_WR_COMP_WAIT_BY_NOP_COUNT
2:
	sub x0, x0, #1
	nop
	cmp x0, #0
	b.ne	2b
#endif

	/* initiate the sw reset request */
	mov  w0, #SW_RST_REQ_INIT
	ldr  x2, =NXP_RST_ADDR
	str  w0, [x2, #RSTCNTL_OFFSET]

	/* In case this address range is mapped as cacheable,
	 * flush the write out of the dcaches.
	 */
	add  x2, x2, #RSTCNTL_OFFSET
	dc   cvac, x2
	dsb  st
	isb

	/* Function does not return */
	b  .
	mov  x30, x6
	ret
endfunc _soc_sys_warm_reset
