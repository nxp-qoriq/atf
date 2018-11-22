/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Authors: Jiafei Pan <jiafei.pan@nxp.com>
 *          Rod Dorris <rod.dorris@nxp.com>
 */

#include <platform_def.h>
#include <asm_macros.S>
#include <console.h>

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.weak	platform_mem_init
	.globl	arm_disable_spe
	.globl  plat_core_pos
	.globl  plat_my_core_pos
	.globl  plat_core_mask
	.globl  plat_my_core_mask
	.globl  plat_core_pos_by_mpidr
	.globl _ocram_init
	.globl _get_ocram_2_init
	.globl _disable_ldstr_pfetch_A53
	.globl _disable_ldstr_pfetch_A72
	.global	_set_smmu_pagesz_64


#define  OCRAM_REGION_LOWER  0
#define  OCRAM_REGION_UPPER  1
#define  OCRAM_REGION_ALL    2

#define CPUACTLR_EL1		S3_1_C15_C2_0
#define CPUACTLR_L1PCTL_MASK	0x0000E000
#define CPUACTLR_DIS_LS_HW_PRE	0x100000000000000

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */

#if MULTI_CONSOLE_API
	/* -----------------------------------------------------
	 * int plat_crash_console_init(void)
	 * Use normal console by default. Switch it to crash
	 * mode so serial consoles become active again.
	 * NOTE: This default implementation will only work for
	 * crashes that occur after a normal console (marked
	 * valid for the crash state) has been registered with
	 * the console framework. To debug crashes that occur
	 * earlier, the platform has to override these functions
	 * with an implementation that initializes a console
	 * driver with hardcoded parameters. See
	 * docs/porting-guide.rst for more information.
	 * -----------------------------------------------------
	 */
func plat_crash_console_init
	mov	x3, x30
	mov	x0, #CONSOLE_FLAG_CRASH
	bl	console_switch_state
	mov	x0, #1
	ret	x3
endfunc plat_crash_console_init

	/* -----------------------------------------------------
	 * void plat_crash_console_putc(int character)
	 * Output through the normal console by default.
	 * -----------------------------------------------------
	 */
func plat_crash_console_putc
	b	console_putc
endfunc plat_crash_console_putc

	/* -----------------------------------------------------
	 * void plat_crash_console_flush(void)
	 * Flush normal console by default.
	 * -----------------------------------------------------
	 */
func plat_crash_console_flush
	b	console_flush
endfunc plat_crash_console_flush

#else	/* MULTI_CONSOLE_API */

	/* -----------------------------------------------------
	 * In the old API these are all no-op stubs that need to
	 * be overridden by the platform to be useful.
	 * -----------------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, PLAT_LS_UART_BASE
	mov_imm	x1, PLAT_LS_UART_CLOCK
	mov_imm	x2, PLAT_LS_UART_BAUDRATE
	b	console_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, PLAT_LS_UART_BASE
	b	console_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * int plat_crash_console_flush()
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0 - r1
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x1, PLAT_LS_UART_BASE
	b	console_core_flush
endfunc plat_crash_console_flush
#endif
	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on LS
	 * platforms. The Secure SRAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init


/******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 *****************************************************************************/
func plat_core_pos_by_mpidr

	b  plat_core_pos

endfunc plat_core_pos_by_mpidr

#if (SYMMETRICAL_CLUSTERS)
	/* ---------------------------------------------------------------------
     * unsigned int plat_my_core_mask(void)
	 *  generate a mask bit for this core
	 * ---------------------------------------------------------------------
	 */
func plat_my_core_mask
    mrs  x0, MPIDR_EL1
    b    plat_core_mask
endfunc plat_my_core_mask

	/* ---------------------------------------------------------------------
     * unsigned int plat_core_mask(u_register_t mpidr)
	 *  generate a mask bit for the core specified by mpidr in x0
	 * ---------------------------------------------------------------------
	 */
func plat_core_mask
     /* generate a lsb-based mask for the core
      * SoC core = ((cluster * cpu_per_cluster) + core)
      * mask = (1 << SoC core)
      */
    mov   w1, wzr
    mov   w2, wzr
    bfxil w1, w0, #8, #8  // extract cluster
    bfxil w2, w0, #0, #8  // extract cpu #

    mov   w0, wzr

     // error checking
    cmp   w1, #NUMBER_OF_CLUSTERS
    b.ge  1f
    cmp   w2, #CORES_PER_CLUSTER
    b.ge  1f

    mov   w0, #CORES_PER_CLUSTER
    mul   w1, w1, w0
    add   w1, w1, w2
    mov   w2, #0x1
    lsl   w0, w2, w1
1:
    ret
endfunc plat_core_mask

	/* ---------------------------------------------------------------------
     * unsigned int plat_my_core_pos(void)
	 *  generate a linear core number for this core
	 * ---------------------------------------------------------------------
	 */
func plat_my_core_pos
    mrs  x0, MPIDR_EL1
    b    plat_core_pos
endfunc plat_my_core_pos

	/* ---------------------------------------------------------------------
     * unsigned int plat_core_pos(u_register_t mpidr)
	 *  generate a linear core number for the core specified by mpidr
     *  returns -1 if mpidr invalid
	 * ---------------------------------------------------------------------
	 */
func plat_core_pos
     /* generate a linear number for the core
      * SoC core = ((cluster * cpu_per_cluster) + core)
      */
    mov   w1, wzr
    mov   w2, wzr
    bfxil w1, w0, #8, #8  /* extract cluster */
    bfxil w2, w0, #0, #8  /* extract cpu #   */

    mov   w0, #-1

     /* error checking */
    cmp   w1, #NUMBER_OF_CLUSTERS
    b.ge  1f
    cmp   w2, #CORES_PER_CLUSTER
    b.ge  1f

    mov   w0, #CORES_PER_CLUSTER
    mul   w1, w1, w0
    add   w0, w1, w2
1:
    ret
endfunc plat_core_pos

#else
	/* ---------------------------------------------------------------------
	 * if the clusters are not symmetrical, then add appropriate
     * functions here
	 * ---------------------------------------------------------------------
	 */
#endif

/******************************************************************************
 * This function initializes ocram for ecc checking
 *****************************************************************************/

 /* in:  none
  * out: none
  * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10 */
_ocram_init:
    mov  x10, x30

     // set the start flag
    mov  x0, #1
    bl   _set_task1_start

     // get the start address and size of the OCRAM
    mov  x0, #OCRAM_REGION_ALL
    bl   _get_ocram_2_init

     // x0 = start address
     // x1 = size in bytes

     // convert bytes to 64-byte chunks (using quad load/store pair ops)
    lsr  x1, x1, #6

     // x0 = start address
     // x1 = size in 64-byte chunks
1:
     // for each location, read and write-back
    dc   ivac, x0
    isb
    ldp  x2, x3, [x0]
    ldp  x4, x5, [x0, #16]
    ldp  x6, x7, [x0, #32]
    ldp  x8, x9, [x0, #48]
    stp  x2, x3, [x0]
    stp  x4, x5, [x0, #16]
    stp  x6, x7, [x0, #32]
    stp  x8, x9, [x0, #48]
    dc   cvac, x0

    sub  x1, x1, #1
    cbz  x1, 2f
    add  x0, x0, #64
    b    1b

2:
     // set the done flag
    mov  x0, #1
    bl   _set_task1_done

     // make sure the data accesses are complete
    dsb  sy
    isb

     // restore link register
    mov  x30, x10

     // clean the registers
    mov  x0,  #0
    mov  x1,  #0
    mov  x2,  #0
    mov  x3,  #0
    mov  x4,  #0
    mov  x5,  #0
    mov  x6,  #0
    mov  x7,  #0
    mov  x8,  #0
    mov  x9,  #0
    mov  x10, #0

    ret

/*---------------------------------------------------------------------------*/

 /* this function returns the start address and size of two equal regions
  * of ocram for initialization purposes. If the stack area is in the top
  * of ocram, this is left out of the ocram regions to be initialized
  * in:  x0 = 0, return start addr and size of lower ocram region
  *         = 1, return start addr and size of upper ocram region
  *         = 2, return start addr and size of entire ocram region
  * out: x0 = start address of region
  *      x1 = size of region in bytes
  * uses x0, x1, x2, x3, x4, x5 */
_get_ocram_2_init:
    mov  x5, x30
    mov  x4, x0


    mov  x1, #NXP_OCRAM_SIZE
    mov  x2, #NXP_OCRAM_ADDR

1:
     /* x1 = size of ocram to initialize
      * x2 = NXP_OCRAM_ADDR */

    cmp  x4, #2
    b.ne 4f
     /* the request is for all of ocram */
    mov  x0, x2
    b    3f

4:
     /* divide size in half */
    lsr  x1, x1, #1

     /* determine if the upper or lower region of ocram is requested */
    cbz  x4, 2f

     /* process the upper region of ocram */

     /* x1 = size of ocram to initialize
      * x2 = NXP_OCRAM_ADDR */
    
     /* add size to base addr to get start addr of upper half */
    add  x0, x2, x1
    b    3f

2:   /* process the lower region of ocram */

     /* x1 = size of ocram to initialize
      * x2 = NXP_OCRAM_ADDR */

     /* get start address of lower region */
    mov  x0, x2

3:
    mov  x30, x5
    ret

/*---------------------------------------------------------------------------*/

 /* this function disables the load-store prefetch of the calling core
  * Note: this function is for A53 cores ONLY
  * in:  none
  * out: none
  * uses x0 */
_disable_ldstr_pfetch_A53:

    mrs   x0, CPUACTLR_EL1
    tst   x0, #CPUACTLR_L1PCTL_MASK
    b.ne  1f
    b     2f

.align 6
1:   // disable L1 prefetch for this A53 core
    dsb   sy
    isb
    bic   x0, x0, #CPUACTLR_L1PCTL_MASK
    msr   CPUACTLR_EL1, x0
    isb

2:
    ret

 //----------------------------------------------------------------------------

 /* this function disables the load-store prefetch of the calling core
  * Note: this function is for A72 cores ONLY
  * in:  none
  * out: none
  * uses x0 */
_disable_ldstr_pfetch_A72:

    mrs   x0, CPUACTLR_EL1
    tst   x0, #CPUACTLR_DIS_LS_HW_PRE
    b.eq  1f
    b     2f

.align 6
1:   /* disable prefetch for this A72 core */
    dsb   sy
    isb
    orr   x0, x0, #CPUACTLR_DIS_LS_HW_PRE
    msr   CPUACTLR_EL1, x0
    isb

2:
    ret

 //----------------------------------------------------------------------------

/*
 * this function sets the scXR pagesize to 64k
 */
_set_smmu_pagesz_64:

	ldr	x1, =NXP_SMMU_ADDR
	ldr	w0, [x1, #0x10]
	orr	w0, w0, #1 << 16  /* set sACR.pagesize to indicate 64K page */
	str	w0, [x1, #0x10]

	ret

 //----------------------------------------------------------------------------
