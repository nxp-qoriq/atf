/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch.h>
#include <asm_macros.S>

#include <platform_def.h>

.globl	plat_secondary_cold_boot_setup
.globl	plat_is_my_cpu_primary
.globl	plat_reset_handler
.globl  platform_mem_init


func platform_mem1_init
	ret
endfunc platform_mem1_init


func platform_mem_init
	ret
endfunc	platform_mem_init


func apply_platform_errata

	ret
endfunc apply_platform_errata


func plat_reset_handler
	mov x29, x30
	bl  apply_platform_errata
	/* Apply ARM core specific erratas */
	/*TODO: Other platform specific settings */
	/*
	 * May be cntfrq_el0 needs to be assigned
	 * the value COUNTER_FREQUENCY
	 */

#if defined(IMAGE_BL31)
	ldr x0, =POLICY_SMMU_PAGESZ_64K
	cbz x0, 1f
	/* Set the SMMU page size in the sACR register */
	bl _set_smmu_pagesz_64
#endif
1:
	mov x30, x29

	ret
endfunc plat_reset_handler


/* void plat_secondary_cold_boot_setup (void);
 *
 * This function performs any platform specific actions
 * needed for a secondary cpu after a cold reset e.g
 * mark the cpu's presence, mechanism to place it in a
 * holding pen etc.
 * TODO: Should we read the PSYS register to make sure
 * that the request has gone through.
 */
func plat_secondary_cold_boot_setup
	/* lx2160a does not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup


/* unsigned int plat_is_my_cpu_primary (void);
 *
 * Find out whether the current cpu is the primary
 * cpu.
 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, 0x0
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

.global _soc_core_in_wfe
.global _soc_reset_rel_to_wfe

.equ 	SCRATCHRW14, 0x234

func _soc_core_in_wfe
	bl plat_my_core_mask

	/* read-modify-write BRRL to release core */
	mov  x1, #NXP_DCFG_ADDR

1:
	wfe

	ldr  w2, [x1, #SCRATCHRW14]

	/* x0 = core mask */
	and  w2, w2, w0
	cbz  w2, 1b

	/* Get the 64-bit base address of the dcfg block */
	ldr  w2, [x1, #DCFG_BOOTLOCPTRH_OFFSET]
	lsr  x3, x2, #32
	ldr  w2, [x1, #DCFG_BOOTLOCPTRL_OFFSET]
	orr  w3, w3, w2

	mov   x30, x3
	ret
endfunc _soc_core_in_wfe


func   _soc_reset_rel_to_wfe
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	/* Get the 64-bit base address of the dcfg block */
	ldr  x2, =NXP_DCFG_ADDR

	/* write the 32-bit BOOTLOCPTRL register */
	mov  x1, x0
	str  w1, [x2, #DCFG_BOOTLOCPTRL_OFFSET]

	/* write the 32-bit BOOTLOCPTRH register */
	lsr  x1, x0, #32
	str  w1, [x2, #DCFG_BOOTLOCPTRH_OFFSET]

	/* get the number of cpus on this device */
	mov   x6, #PLATFORM_CORE_COUNT
	/* load the core mask for the first core */
	mov  x7, #1

2:
	ldr  x1, =NXP_SEC_REGFILE_ADDR
	/* write to CORE_HOLD to tell
	 * the bootrom that this core is
	 * expected to run.
	 */
	str  w7, [x1, #CORE_HOLD_OFFSET]

	/* read-modify-write BRRL to release core */
	mov  x1, #NXP_RESET_ADDR
	ldr  w2, [x1, #BRR_OFFSET]

	/* x0 = core mask */
	orr  w2, w2, w7
	str  w2, [x1, #BRR_OFFSET]

	dsb  sy
	isb

	/* decrement the counter */
	subs  x6, x6, #1
	b.le  3f

	/* shift the core mask to the next core */
	lsl   x7, x7, #1
	/* continue */
	b	 2b
3:
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	ret
endfunc   _soc_reset_rel_to_wfe
