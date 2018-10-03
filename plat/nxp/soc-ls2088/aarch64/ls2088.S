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
.global _soc_core_prep_stdby
.global _soc_core_entr_stdby
.global _soc_core_exit_stdby


.global _set_platform_security

//-----------------------------------------------------------------------------

.equ TZPCDECPROT_0_SET_BASE, 0x02200804
.equ TZPCDECPROT_1_SET_BASE, 0x02200810
.equ TZPCDECPROT_2_SET_BASE, 0x0220081C

.equ TZASC_REGION_ATTRIBUTES_0_0, 0x01100110
/*---------------------------------------------------------------------------*/
func soc_init_start
    ret
endfunc soc_init_start

func soc_init_finish
    ret
endfunc soc_init_finish

/*---------------------------------------------------------------------------*/

 /* void soc_init_percpu(void)
  * this function performs any soc-specific initialization that is needed on 
  * a per-core basis
  * in:  none
  * out: none
  * uses x0, x1, x2, x3 */
func soc_init_percpu
    stp  x4,  x30,  [sp, #-16]!

    bl   plat_my_core_mask
    mov  x2, x0

     /* x2 = core mask */

     /* see if this core is marked for prefetch disable */
    mov   x0, #PREFETCH_DIS_OFFSET
    bl    _get_global_data  /* 0-1 */
    tst   x0, x2
    b.eq  1f
    bl    _disable_ldstr_pfetch_A72  /* 0 */
1:
    bl enable_timer_base_to_cluster
    ldp  x4,  x30,  [sp], #16
    ret
endfunc soc_init_percpu

/*---------------------------------------------------------------------------*/

 // this function determines if a core is disabled via COREDISABLEDSR
 // in:  w0  = core_mask_lsb
 // out: w0  = 0, core not disabled
 //      w0 != 0, core disabled
 // uses x0, x1
_soc_ck_disabled:

     // get base addr of dcfg block
    ldr  x1, =NXP_DCFG_ADDR

     // read COREDISABLEDSR
    ldr  w1, [x1, #DCFG_COREDISABLEDSR_OFFSET]

     // test core bit
    and  w0, w1, w0

    ret

/*---------------------------------------------------------------------------*/
 // this function sets the security mechanisms in the SoC to implement the
 // Platform Security Policy
_set_platform_security:
    mov  x8, x30

#if (!SUPPRESS_TZC)
     // initialize the tzpc
    bl   init_tzpc
#endif

#if (!SUPPRESS_SEC)
     // initialize secmon
    bl  initSecMon
#endif

    mov  x30, x8
    ret

/*---------------------------------------------------------------------------*/

 // this function setc up the TrustZone Address Space Controller (TZASC)
 // in:  none
 // out: none
 // uses x0, x1
init_tzpc:

     // set Non Secure access for all devices protected via TZPC
	ldr	x1, =TZPCDECPROT_0_SET_BASE   // decode Protection-0 Set Reg
	mov	w0, #0xFF		              // set decode region to NS, Bits[7:0]
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_1_SET_BASE   // decode Protection-1 Set Reg
	mov	w0, #0xFF		              // set decode region to NS, Bits[7:0]
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_2_SET_BASE   // decode Protection-2 Set Reg
	mov	w0, #0xFF		              // set decode region to NS, Bits[7:0]
	str	w0, [x1]

	 // entire SRAM as NS
	ldr	x1, =NXP_OCRAM_TZPC_ADDR	              // secure RAM region size Reg
	mov	w0, #0x00000000		          // 0x00000000 = no secure region
	str	w0, [x1]

    ret

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

 // this function performs any needed initialization on SecMon for boot services
initSecMon:

     // read the register hpcomr
    ldr  x1, =NXP_SNVS_ADDR
    ldr  w0, [x1, #SECMON_HPCOMR_OFFSET]
     // turn off secure access for the privileged registers
    orr  w0, w0, #SECMON_HPCOMR_NPSWAEN
     // write back
    str  w0, [x1, #SECMON_HPCOMR_OFFSET]

    ret

/*---------------------------------------------------------------------------*/
