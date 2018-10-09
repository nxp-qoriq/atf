/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 */

  .section .text, "ax"

/*---------------------------------------------------------------------------*/

#include <asm_macros.S>
#include <platform_def.h>
#include <psci.h>
#include <bl31_data.h>
#include <plat_psci.h>

/*---------------------------------------------------------------------------*/

.global soc_init_start
.global soc_init_percpu
.global soc_init_finish

.global _soc_core_release
.global _soc_core_restart
.global _soc_ck_disabled
.global _soc_sys_reset

//.global _soc_core_prep_stdby
//.global _soc_core_entr_stdby
//.global _soc_core_exit_stdby

.global _set_platform_security
.global _soc_set_start_addr

//-----------------------------------------------------------------------------

.equ TZPCDECPROT_0_SET_BASE, 0x02200804
.equ TZPCDECPROT_1_SET_BASE, 0x02200810
.equ TZPCDECPROT_2_SET_BASE, 0x0220081C

.equ TZASC_REGION_ATTRIBUTES_0_0, 0x01100110

/*---------------------------------------------------------------------------*/

 /* this function starts the initialization tasks of the soc, using secondary cores
  * if they are available
  * in: 
  * out: 
  * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11
  */
func soc_init_start
    mov   x11, x30

     /* make sure the personality has been established by releasing cores
      * that are marked "to-be-disabled" from reset
      */
    bl  release_disabled  // 0-8

     /* init the task flags */
    bl  _init_task_flags   // 0-1

#if 0

     /* save start address */
    bl  _soc_get_start_addr   // 0-2
    mov  x1, x0
    mov  x0, #BOOTLOC_OFFSET
    bl   _set_global_data

     /* see if we are initializing ocram */
    ldr x0, =POLICY_USING_ECC
    cbz x0, 1f
     /* initialize the OCRAM for ECC */

     /* get a secondary core to initialize the upper half of ocram */
    bl  _find_core      // 0-4
    cbz x0, 2f
    bl  init_task_1     // 0-5   
5:
     /* wait til task 1 has started */
    bl  _get_task1_start // 0-1
    cbnz x0, 4f
    b    5b
4:
     /* get a secondary core to initialize the lower
      * half of ocram
      */
    bl  _find_core      // 0-4
    cbz x0, 3f
    bl  init_task_2     // 0-5
6:
     /* wait til task 2 has started */
    bl  _get_task2_start // 0-1
    cbnz x0, 7f
    b    6b
2:
     /* there are no secondary cores available, so the
      * boot core will have to init upper ocram
      */
    bl  _ocram_init_upper // 0-10
3:
     /* there are no secondary cores available, so the
      * boot core will have to init lower ocram
      */
    bl  _ocram_init_lower // 0-10
    b   1f
7:

     /* clear bootlocptr */
    mov  x0, xzr
    bl    _soc_set_start_addr

#endif

     /* set SCRATCHRW7 to 0x0 */
    ldr  x0, =DCFG_SCRATCHRW7_OFFSET
    mov  x1, xzr
    bl   _write_reg_dcfg

1:
    mov   x30, x11
    ret
endfunc soc_init_start

/*---------------------------------------------------------------------------*/

 /* this function completes the initialization tasks of the soc
  * in: 
  * out: 
  * uses x0, x1, x2, x3, x4
  */
func soc_init_finish
    mov   x4, x30

#if 0
     /* are we initializing ocram? */
    mov x0, #POLICY_USING_ECC
    cbz x0, 4f

     /* if the ocram init is not completed, wait til it is */
1:
    bl   _get_task1_done
    cbnz x0, 2f
    wfe
    b    1b    
2:
    bl   _get_task2_done
    cbnz x0, 3f
    wfe
    b    2b    
3:
     /* set the task 1 core state to IN_RESET */
    bl   _get_task1_core
    cbz  x0, 5f
     /* x0 = core mask lsb of the task 1 core */
    mov  x1, #CORE_STATE_DATA
    mov  x2, #CORE_WFE
    bl   _setCoreData
5:
     /* set the task 2 core state to IN_RESET */
    bl   _get_task2_core
    cbz  x0, 4f
     /* x0 = core mask lsb of the task 2 core */
    mov  x1, #CORE_STATE_DATA
    mov  x2, #CORE_WFE
    bl   _setCoreData
4:
     /* restore bootlocptr */
    mov  x0, #BOOTLOC_OFFSET
    bl   _get_global_data
     /* x0 = saved start address */
    bl    _soc_set_start_addr
#endif

    mov   x30, x4
    ret
endfunc soc_init_finish

/*---------------------------------------------------------------------------*/

 /* void soc_init_percpu(void)
  * this function performs any soc-specific initialization that is needed on 
  * a per-core basis
  * in:  none
  * out: none
  * uses x0, x1, x2, x3
  */
func soc_init_percpu
    stp  x4,  x30,  [sp, #-16]!

    bl   enable_timer_base_to_cluster
    ldp  x4,  x30,  [sp], #16
    ret
endfunc soc_init_percpu

/*---------------------------------------------------------------------------*/

 /* this function sets the security mechanisms in the SoC to implement the
  * Platform Security Policy
  */
func _set_platform_security
    mov  x3, x30

#if (!SUPPRESS_TZC)
     /* initialize the tzpc */
    bl  init_tzpc
#endif

#if (!SUPPRESS_SEC)
     /* initialize secmon */
    bl  initSecMon
#endif

    mov  x30, x3
    ret
endfunc _set_platform_security

/*---------------------------------------------------------------------------*/

 /* this function writes a 64-bit address to bootlocptrh/l
  * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
  * uses x0, x1, x2
  */
func _soc_set_start_addr
     /* get the 64-bit base address of the dcfg block */
    ldr  x2, =NXP_DCFG_ADDR

     /* write the 32-bit BOOTLOCPTRL register */
    mov  x1, x0
    str  w1, [x2, #DCFG_BOOTLOCPTRL_OFFSET]

     /* write the 32-bit BOOTLOCPTRH register */
    lsr  x1, x0, #32
    str  w1, [x2, #DCFG_BOOTLOCPTRH_OFFSET]
    ret
endfunc _soc_set_start_addr

/*---------------------------------------------------------------------------*/

 /* part of CPU_ON
  * this function releases a secondary core from reset
  * in:   x0 = core_mask_lsb
  * out:  none
  * uses: x0, x1, x2, x3
  */
_soc_core_release:
    mov   x3, x30

     /* x0 = core mask */

    ldr  x1, =NXP_SEC_REGFILE_ADDR
     /* write to CORE_HOLD to tell the bootrom that we want this core
      * to run
      */
    str  w0, [x1, #CORE_HOLD_OFFSET]

     /* x0 = core mask */

     /* read-modify-write BRRL to release core */
    mov  x1, #NXP_RESET_ADDR
    ldr  w2, [x1, #BRR_OFFSET]
    orr  w2, w2, w0
    str  w2, [x1, #BRR_OFFSET]
    dsb  sy
    isb

     /* send event */
    sev
    isb

    mov   x30, x3
    ret

/*---------------------------------------------------------------------------*/

 /* this function determines if a core is disabled via COREDISABLEDSR
  * in:  w0  = core_mask_lsb
  * out: w0  = 0, core not disabled
  *      w0 != 0, core disabled
  * uses x0, x1
  */
_soc_ck_disabled:

     /* get base addr of dcfg block */
    ldr  x1, =NXP_DCFG_ADDR

     /* read COREDISABLEDSR */
    ldr  w1, [x1, #DCFG_COREDISABLEDSR_OFFSET]

     /* test core bit */
    and  w0, w1, w0

    ret

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

 /* this function checks to see if cores which are to be disabled have been
  * released from reset - if not, it releases them
  * in:  none
  * out: none
  * uses x0, x1, x2, x3, x4, x5, x6, x7, x8
  */
release_disabled:
    mov  x8, x30

     /* read COREDISABLESR */
    mov  x0, #NXP_DCFG_ADDR
    ldr  w4, [x0, #DCFG_COREDISABLEDSR_OFFSET]

     /* get the number of cpus on this device */
    mov   x6, #PLATFORM_CORE_COUNT

    mov  x0, #NXP_RESET_ADDR
    ldr  w5, [x0, #BRR_OFFSET]

     /* load the core mask for the first core */
    mov  x7, #1

     /* x4 = COREDISABLESR
      * x5 = BRR
      * x6 = loop count
      * x7 = core mask bit
      */
2:
     /* check if the core is to be disabled */
    tst  x4, x7
    b.eq 1f

     /* see if disabled cores have already been released from reset */
    tst  x5, x7
    b.ne 1f

     /* if core has not been released, then release it (0-3) */
    mov  x0, x7
    bl   _soc_core_release

     /* record the core state in the data area (0-3) */
    mov  x0, x7
    mov  x1, #CORE_DISABLED
    bl   _setCoreState

1:
     /* decrement the counter */
    subs  x6, x6, #1
    b.le  3f
    
     /* shift the core mask to the next core */
    lsl   x7, x7, #1
     /* continue */
    b     2b
3:
    mov  x30, x8
    ret

/*---------------------------------------------------------------------------*/

 /* write a register in the DCFG block
  * in:  x0 = offset
  * in:  w1 = value to write
  * uses x0, x1, x2
  */
_write_reg_dcfg:
    ldr  x2, =NXP_DCFG_ADDR
    str  w1, [x2, x0]
    ret

/*---------------------------------------------------------------------------*/

 /* read a register in the DCFG block
  * in:  x0 = offset
  * out: w0 = value read
  * uses x0, x1
  */
_read_reg_dcfg:
    ldr  x1, =NXP_DCFG_ADDR
    ldr  w0, [x1, x0]
    ret

/*---------------------------------------------------------------------------*/

 /* this function setc up the TrustZone Address Space Controller (TZASC)
  * in:  none
  * out: none
  * uses x0, x1
  */
init_tzpc:

    	 /* set Non Secure access for all devices protected via TZPC */
	ldr	x1, =TZPCDECPROT_0_SET_BASE   // decode Protection-0 Set Reg
	mov	w0, #0xFF		              // set decode region to NS, Bits[7:0]
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_1_SET_BASE   // decode Protection-1 Set Reg
	mov	w0, #0xFF		              // set decode region to NS, Bits[7:0]
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_2_SET_BASE   // decode Protection-2 Set Reg
	mov	w0, #0xFF		              // set decode region to NS, Bits[7:0]
	str	w0, [x1]

	 /* entire SRAM as NS */
	ldr	x1, =NXP_OCRAM_TZPC_ADDR	              // secure RAM region size Reg
	mov	w0, #0x00000000		          // 0x00000000 = no secure region
	str	w0, [x1]

    ret

/*---------------------------------------------------------------------------*/

 /* this function performs initialization on SecMon for boot services */
initSecMon:

     /* read the register hpcomr */
    ldr  x1, =NXP_SNVS_ADDR
    ldr  w0, [x1, #SECMON_HPCOMR_OFFSET]
     /* turn off secure access for the privileged registers */
    orr  w0, w0, #SECMON_HPCOMR_NPSWAEN
     /* write back */
    str  w0, [x1, #SECMON_HPCOMR_OFFSET]

    ret

/*---------------------------------------------------------------------------*/

