/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 */

/*---------------------------------------------------------------------------*/

#include <asm_macros.S>
#include <platform_def.h>
#include <psci.h>
#include <bl31_data.h>
#include <plat_psci.h>

/*---------------------------------------------------------------------------*/

#define DAIF_DATA         AUX_01_DATA
#define TIMER_CNTRL_DATA  AUX_02_DATA

#define CPUACTLR_DATA_OFFSET  0x1C

#define IPSTPACK_RETRY_CNT    0x10000
#define DDR_SLEEP_RETRY_CNT   0x10000

#define DLL_LOCK_MASK   0x3
#define DLL_LOCK_VALUE  0x2

#define ERROR_DDR_SLEEP       -1
#define ERROR_DDR_WAKE        -2
#define ERROR_NO_QUIESCE      -3

/*---------------------------------------------------------------------------*/

.global soc_init_start
.global soc_init_percpu
.global soc_init_finish
.global _soc_core_release
.global _soc_core_restart
.global _soc_ck_disabled
.global _soc_sys_reset
.global _soc_sys_off
.global _soc_set_start_addr
.global _getGICC_BaseAddr
.global _getGICD_BaseAddr
.global _soc_core_prep_off
.global _soc_core_entr_off
.global _soc_core_exit_off
.global _soc_core_prep_stdby
.global _soc_core_entr_stdby
.global _soc_core_exit_stdby
.global _soc_core_prep_pwrdn
.global _soc_core_entr_pwrdn
.global _soc_core_exit_pwrdn

//.global _soc_get_start_addr
//.global _soc_core_exit_stdby
//.global _soc_core_entr_pwrdn
//.global _soc_core_exit_pwrdn
//.global _soc_clstr_entr_stdby
//.global _soc_clstr_exit_stdby
//.global _soc_clstr_entr_pwrdn
//.global _soc_clstr_exit_pwrdn
//.global _soc_sys_entr_stdby
//.global _soc_sys_exit_stdby
//.global _soc_sys_entr_pwrdn
//.global _soc_sys_exit_pwrdn
//.global _soc_core_phase1_clnup
//.global _soc_core_phase2_clnup

//.global _get_current_mask

//.global _set_platform_security
//.global _soc_exit_boot_svcs

/*---------------------------------------------------------------------------*/

 /* Note: this macro will only work correctly if the registers used are
  *       32-bit (w-form) */

 /* generate a core mask msb */
.macro CoreMaskMsb $p1, $p2
     /* input:  $p1 = core mask lsb
      * output: $p1 = core mask msb
      * 0x8000_0000 >> (31 - leading_zeroes_lsb) */
    clz  \$p2, \$p1
    mov  \$p1, #31
    sub  \$p1, \$p1, \$p2
    mov  \$p2, #0x80000000
    lsr  \$p1, \$p2, \$p1
.endm

/*---------------------------------------------------------------------------*/

 /* this function starts the initialization tasks of the soc, using secondary cores
  * if they are available
  * in: 
  * out: 
  * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11 */
func soc_init_start
    mov   x11, x30

#if 0
     /* init the task flags */
    bl  _init_task_flags   // 0-1

     /* save start address */
    bl  _soc_get_start_addr   // 0-2
    mov  x1, x0
    mov  x0, #BOOTLOC_OFFSET
    bl   _set_global_data

     /* see if we are initializing ocram */
    ldr x0, =POLICY_USING_ECC
    cbz x0, 1f
     /* initialize the OCRAM for ECC */

     /* get a secondary core to initialize ocram */
    bl  _find_core      // 0-4
    cbz x0, 2f
    bl  init_task_1     // 0-5   
3:
     /* wait til task 1 has started */
    bl   _get_task1_start // 0-1
    cbz  x0, 3b
    b    4f
2:
     /* there are no secondary cores available, so the
      * boot core will have to init ocram */
    bl  _ocram_init // 0-10
4:
     /* clear bootlocptr */
    mov  x0, xzr
    bl    _soc_set_start_addr

#endif

1:
    mov   x30, x11
    ret
endfunc soc_init_start

/*---------------------------------------------------------------------------*/

 /* this function completes the initialization tasks of the soc
  * in: 
  * out: 
  * uses x0, x1, x2, x3, x4 */
func soc_init_finish

#if 0

    mov   x4, x30

     /* are we initializing ocram? */
    ldr x0, =POLICY_USING_ECC
    cbz x0, 2f

     /* if the ocram init is not completed, wait til it is */
1:
    bl   _get_task1_done
    cbz  x0, 1b

2:
     /* restore bootlocptr */
    mov  x0, #BOOTLOC_OFFSET
    bl   _get_global_data
     /* x0 = saved start address */
    bl   _soc_set_start_addr

    mov  x30, x4
#endif

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
    mov  x3, x30

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
    mov  x30, x3
    ret
endfunc soc_init_percpu

/*---------------------------------------------------------------------------*/

 /* part of CPU_ON
  * this function releases a secondary core from reset
  * in:   x0 = core_mask_lsb
  * out:  none
  * uses: x0, x1, x2, x3 */
_soc_core_release:

#if (SIMULATOR_BUILD)
     // x0 = core mask lsb

    mov  w2, w0
    CoreMaskMsb w2, w3

     // x0 = core mask lsb
     // x2 = core mask msb

#else
     // x0 = core mask lsb

    mov  x2, x0

#endif
     // write COREBCR 
    mov   x1, #NXP_SCFG_ADDR
    rev   w3, w2
    str   w3, [x1, #SCFG_COREBCR_OFFSET]
    isb

     // x0 = core mask lsb

     // read-modify-write BRR
    mov  x1, #NXP_DCFG_ADDR
    ldr  w2, [x1, #DCFG_BRR_OFFSET]
    rev  w3, w2
    orr  w3, w3, w0
    rev  w2, w3
    str  w2, [x1, #DCFG_BRR_OFFSET]
    isb

     // send event
    sev
    isb
    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_ON
  * this function restarts a core shutdown via _soc_core_entr_off
  * in:  x0 = core mask lsb (of the target cpu)
  * out: x0 == 0, on success
  *      x0 != 0, on failure
  * uses x0, x1, x2, x3, x4, x5
  */
_soc_core_restart:
    mov  x5, x30
    mov  x3, x0

     /* x3 = core mask lsb */

     /* unset ph20 request in RCPM_PCPH20CLEARR
      * this is an lsb-0 register
      */
    ldr   x1, =NXP_RCPM_ADDR
    rev   w2, w3
    str   w2, [x1, #RCPM_PCPH20CLRR_OFFSET]
    dsb sy
    isb

    bl   _getGICD_BaseAddr
    mov  x4, x0

     /* x3 = core mask lsb */
     /* x4 = GICD base addr */

     /* enable forwarding of group 0 interrupts by setting GICD_CTLR[0] = 1 */
    ldr  w1, [x4, #GICD_CTLR_OFFSET]
    orr  w1, w1, #GICD_CTLR_EN_GRP0
    str  w1, [x4, #GICD_CTLR_OFFSET]
    dsb sy
    isb

     /* x3 = core mask lsb */
     /* x4 = GICD base addr */

     /* fire SGI by writing to GICD_SGIR the following values:
      * [25:24] = 0x0 (forward interrupt to the CPU interfaces specified in CPUTargetList field)
      * [23:16] = core mask lsb[7:0] (forward interrupt to target cpu)
      * [15]    = 0 (forward SGI only if it is configured as group 0 interrupt)
      * [3:0]   = 0xF (interrupt ID = 15)
      */
    lsl  w1, w3, #16
    orr  w1, w1, #0xF
    str  w1, [x4, #GICD_SGIR_OFFSET]
    dsb sy
    isb

     /* load '0' on success */
    mov  x0, xzr

    mov  x30, x5
    ret

/*---------------------------------------------------------------------------*/

 /* this function determines if a core is disabled via COREDISR
  * in:  w0  = core_mask_lsb
  * out: w0  = 0, core not disabled
  *      w0 != 0, core disabled
  * uses x0, x1, x2 */
_soc_ck_disabled:

     /* get base addr of dcfg block */
    mov  x1, #NXP_DCFG_ADDR

     /* read COREDISR */
    ldr  w1, [x1, #DCFG_COREDISR_OFFSET]
    rev  w2, w1

     /* test core bit */
    and  w0, w2, w0
    ret

/*---------------------------------------------------------------------------*/

 /* this function resets the system via SoC-specific methods
  * in:  none
  * out: none
  * uses x0, x1, x2, x3 */
_soc_sys_reset:

    ldr  x2, =NXP_DCFG_ADDR

     /* make sure the mask is cleared in the reset request mask register */
    mov  w1, wzr
    str  w1, [x2, #DCFG_RSTRQMR1_OFFSET]

     /* x2 = NXP_DCFG_ADDR */

     /* set the reset request */
    ldr  w1, =RSTCR_RESET_REQ
    ldr  x3, =DCFG_RSTCR_OFFSET
    rev  w0, w1
    str  w0, [x2, x3]

     /* x2 = NXP_DCFG_ADDR */
     /* x3 = DCFG_RSTCR_OFFSET */

     /* just in case this address range is mapped as cacheable,
      * flush the write out of the dcaches */
    add  x3, x2, x3
    dc   cvac, x3
    dsb  st
    isb

     /* Note: this function does not return */
1:
    wfi
    b  1b    

/*---------------------------------------------------------------------------*/

 /* part of SYSTEM_OFF
  * this function turns off the SoC clocks
  * Note: this function is not intended to return, and the only allowable
  *       recovery is POR
  * in:  none
  * out: none
  * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9 */
_soc_sys_off:

     /* mask interrupts at the core */
    mrs  x1, DAIF
    mov  x0, #DAIF_SET_MASK
    orr  x0, x1, x0
    msr  DAIF, x0

     /* disable icache, dcache, mmu @ EL1 */
    mov  x1, #SCTLR_I_C_M_MASK
    mrs  x0, sctlr_el1
    bic  x0, x0, x1
    msr  sctlr_el1, x0

     /* disable dcache for EL3 */
    mrs x1, SCTLR_EL3
    bic x1, x1, #SCTLR_C_MASK
     /* make sure icache is enabled */
    orr x1, x1, #SCTLR_I_MASK
    msr SCTLR_EL3, x1
    isb

     /* enable dynamic retention control (CPUECTLR[2:0]) and SMP (CPUECTLR[6]) */
    mrs  x0, CPUECTLR_EL1
    orr  x0, x0, #CPUECTLR_TIMER_8TICKS
    orr  x0, x0, #CPUECTLR_SMPEN_EN
    msr  CPUECTLR_EL1, x0

     /* set WFIL2EN in SCFG_CLUSTERPMCR */
    ldr  x0, =SCFG_COREPMCR_OFFSET
    ldr  x1, =COREPMCR_WFIL2
    bl   write_reg_scfg

     /* request LPM20 */
    mov  x0, #RCPM_POWMGTCSR_OFFSET
    bl   read_reg_rcpm
    orr  x1, x0, #RCPM_POWMGTCSR_LPM20_REQ
    mov  x0, #RCPM_POWMGTCSR_OFFSET
    bl   write_reg_rcpm

    dsb  sy
    isb
1:
    wfi
    b    1b

/*---------------------------------------------------------------------------*/

 /* write a register in the RCPM block
  * in:  x0 = offset
  * in:  w1 = value to write
  * uses x0, x1, x2, x3 */
write_reg_rcpm:
    ldr  x2, =NXP_RCPM_ADDR
     /* swap for BE */
    rev  w3, w1
    str  w3, [x2, x0]
    ret
/*---------------------------------------------------------------------------*/

 /* read a register in the RCPM block
  * in:  x0 = offset
  * out: w0 = value read
  * uses x0, x1, x2 */
read_reg_rcpm:
    ldr  x2, =NXP_RCPM_ADDR
    ldr  w1, [x2, x0]
     /* swap for BE */
    rev  w0, w1
    ret

/*---------------------------------------------------------------------------*/

 /* write a register in the SCFG block
  * in:  x0 = offset
  * in:  w1 = value to write
  * uses x0, x1, x2, x3 */
write_reg_scfg:
    mov  x2, #NXP_SCFG_ADDR
     /* swap for BE */
    rev  w3, w1
    str  w3, [x2, x0]
    ret

/*---------------------------------------------------------------------------*/

 /* read a register in the SCFG block
  * in:  x0 = offset
  * out: w0 = value read
  * uses x0, x1, x2 */
read_reg_scfg:
    mov  x2, #NXP_SCFG_ADDR
    ldr  w1, [x2, x0]
     /* swap for BE */
    rev  w0, w1
    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_OFF
  * this function programs SoC & GIC registers in preparation for shutting down
  * the core
  * in:  x0 = core mask lsb
  * out: none
  * uses x0, x1, x2, x3, x4, x5, x6, x7
  */
_soc_core_prep_off:
    mov  x7, x30
    mov  x6, x0

     /* x6 = core mask lsb */

     /* set retention control in CPUECTLR
      * make sure smpen bit is set */
    mrs   x4, CPUECTLR_EL1
    bic   x4, x4, #CPUECTLR_RET_MASK
    orr   x4, x4, #CPUECTLR_TIMER_8TICKS
    orr   x4, x4, #CPUECTLR_SMPEN_EN
    msr   CPUECTLR_EL1, x4

     /* x6 = core mask lsb */

     /* save timer control current value */
    mov   x5, #NXP_TIMER_ADDR
    ldr   w4, [x5, #SYS_COUNTER_CNTCR_OFFSET]
    mov   w2, w4
    mov   x0, x6
    mov   x1, #TIMER_CNTRL_DATA
    bl    _setCoreData

     /* w4 = counter ctl
      * x5 = sys counter base addr
      * x6 = core mask lsb */

     /* enable the timer */
    orr   w4, w4, #CNTCR_EN_MASK
    str   w4, [x5, #SYS_COUNTER_CNTCR_OFFSET]

    bl   _getGICC_BaseAddr
    mov  x5, x0

     /* x5 = GICC_BASE_ADDR */
     /* x6 = core mask lsb  */

     /* disable signaling of ints */
    ldr  w3, [x5, #GICC_CTLR_OFFSET]
    bic  w3, w3, #GICC_CTLR_EN_GRP0
    bic  w3, w3, #GICC_CTLR_EN_GRP1
    str  w3, [x5, #GICC_CTLR_OFFSET]
    dsb  sy
    isb

     /* x3 = GICC_CTRL
      * x5 = GICC_BASE_ADDR
      * x6 = core mask lsb
      */

     /* set retention control in SCFG_RETREQCR
      * Note: this register is msb 0
      */
    ldr  x4, =SCFG_RETREQCR_OFFSET
    mov  x0, x4
    bl   read_reg_scfg
    rbit w1, w6
    orr  w1, w0, w1
    mov  x0, x4
    bl   write_reg_scfg

     /* configure the cpu interface
      * x3 = GICC_CTRL
      * x5 = GICC_BASE_ADDR
      * x6 = core mask lsb
      */

     /* set the priority filter */
    ldr  w2, [x5, #GICC_PMR_OFFSET]
    orr  w2, w2, #GICC_PMR_FILTER
    str  w2, [x5, #GICC_PMR_OFFSET]

     /* setup GICC_CTLR */
    bic  w3, w3, #GICC_CTLR_ACKCTL_MASK
    orr  w3, w3, #GICC_CTLR_FIQ_EN_MASK
    orr  w3, w3, #GICC_CTLR_EOImodeS_MASK
    orr  w3, w3, #GICC_CTLR_CBPR_MASK
    str  w3, [x5, #GICC_CTLR_OFFSET]

     /* x3 = GICC_CTRL
      * x6 = core mask lsb
      */

     /* setup the banked-per-core GICD registers */
    bl   _getGICD_BaseAddr
    mov  x5, x0

     /* x3 = GICC_CTRL
      * x5 = GICD_BASE_ADDR
      * x6 = core mask lsb
      */

     /* define SGI15 as Grp0 */
    ldr  w2, [x5, #GICD_IGROUPR0_OFFSET]
    bic  w2, w2, #GICD_IGROUP0_SGI15
    str  w2, [x5, #GICD_IGROUPR0_OFFSET]

     /* set priority of SGI 15 to highest... */
    ldr  w2, [x5, #GICD_IPRIORITYR3_OFFSET]
    bic  w2, w2, #GICD_IPRIORITY_SGI15_MASK
    str  w2, [x5, #GICD_IPRIORITYR3_OFFSET]

     /* enable SGI 15 */
    ldr  w2, [x5, #GICD_ISENABLER0_OFFSET]
    orr  w2, w2, #GICD_ISENABLE0_SGI15
    str  w2, [x5, #GICD_ISENABLER0_OFFSET]

     /* x3 = GICC_CTRL
      * x5 = GICD base addr
      * x6 = core mask lsb
      */

     /* enable the cpu interface */

    bl   _getGICC_BaseAddr
    mov  x2, x0
    orr  w3, w3, #GICC_CTLR_EN_GRP0
    str  w3, [x2, #GICC_CTLR_OFFSET]

     /* x2 = GICC base addr
      * x3 = GICC_CTRL
      * x5 = GICD base addr
      * x6 = core mask lsb
      */

     /* clear any pending SGIs */
    ldr  x2, =GICD_CPENDSGIR_CLR_MASK
    add  x0, x5, #GICD_CPENDSGIR3_OFFSET
    str  w2, [x0]

     /* x6 = core mask lsb */

     /* set the PC_PH20_REQ bit in RCPM_PCPH20SETR
        this is an lsb-0 register */
    mov  x1, x6
    mov  x0, #RCPM_PCPH20SETR_OFFSET
    bl   write_reg_rcpm

    dsb  sy
    isb
    mov  x30, x7
    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_OFF
  * this function performs the final steps to shutdown the core
  * in:  x0 = core mask lsb
  * out: none
  * uses x0, x1, x2, x3, x4, x5
  */
_soc_core_entr_off:
    mov  x5, x30
    mov  x4, x0

    bl   _getGICD_BaseAddr
    mov  x3, x0

     /* x3 = GICD_BASE_ADDR */
     /* x4 = core mask (lsb) */

3:
     /* enter low-power state by executing wfi */
    wfi

     /* x3 = GICD_BASE_ADDR */
     /* x4 = core mask (lsb) */

     /* see if we got hit by SGI 15 */
    add   x0, x3, #GICD_SPENDSGIR3_OFFSET
    ldr   w2, [x0]
    and   w2, w2, #GICD_SPENDSGIR3_SGI15_MASK
    cbz   w2, 4f

     /* clear the pending SGI */
    ldr   x2, =GICD_CPENDSGIR_CLR_MASK
    add   x0, x3, #GICD_CPENDSGIR3_OFFSET
    str   w2, [x0]
4:
     /* check if core has been turned on */
    mov  x0, x4 
    bl   _getCoreState

     /* x0 = core state */

    cmp  x0, #CORE_WAKEUP
    b.ne 3b

     /* if we get here, then we have exited the wfi */

    dsb  sy
    isb
    mov  x30, x5
    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_OFF
  * this function starts the process of starting a core back up
  * in:  x0 = core mask lsb
  * out: none
  * uses x0, x1, x2, x3, x4, x5, x6
  */
_soc_core_exit_off:
    mov  x6, x30
    mov  x5, x0

     /* x5 = core mask (lsb) */

     /* clear ph20 request in RCPM_PCPH20CLRR - no need 
      * to do that here, it has been done in _soc_core_restart
      */

    bl   _getGICC_BaseAddr
    mov  x1, x0

     /* x1 = GICC_BASE_ADDR  */
     /* x5 = core mask (lsb) */

     /* read GICC_IAR */
    ldr  w0, [x1, #GICC_IAR_OFFSET]

     /* write GICC_EIOR - signal end-of-interrupt */
    str  w0, [x1, #GICC_EOIR_OFFSET]

     /* write GICC_DIR - disable interrupt */
    str  w0, [x1, #GICC_DIR_OFFSET]

     /* x1 = GICC_BASE_ADDR  */
     /* x5 = core mask (lsb) */

     /* disable signaling of grp0 ints */
    ldr  w3, [x1, #GICC_CTLR_OFFSET]
    bic  w3, w3, #GICC_CTLR_EN_GRP0
    str  w3, [x1, #GICC_CTLR_OFFSET]

     /* x5 = core mask (lsb) */

     /* unset retention request in SCFG_RETREQCR
      * Note: this register is msb-0
      */

    ldr  x4, =SCFG_RETREQCR_OFFSET
    mov  x0, x4
    bl   read_reg_scfg
    rbit w1, w5
    bic  w1, w0, w1
    mov  x0, x4
    bl   write_reg_scfg

     /* x5 = core mask (lsb) */

     /* restore timer ctrl */
    mov   x0, x5
    mov   x1, #TIMER_CNTRL_DATA
    bl    _getCoreData
     /* w0 = timer ctrl saved value */
    mov   x2, #NXP_TIMER_ADDR
    str   w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]

    dsb sy
    isb
    mov  x30, x6
    ret

/*---------------------------------------------------------------------------*/

 /* this function loads a 64-bit execution address of the core in the soc registers
  * BOOTLOCPTRL/H
  * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
  * uses x0, x1, x2, x3  */
_soc_set_start_addr:
     /* get the 64-bit base address of the scfg block */
    ldr  x2, =NXP_SCFG_ADDR

     /* write the 32-bit BOOTLOCPTRL register */
    mov  x1, x0
    rev  w3, w1
    str  w3, [x2, #SCFG_BOOTLOCPTRL_OFFSET]

     /* write the 32-bit BOOTLOCPTRH register */
    lsr  x1, x0, #32
    rev  w3, w1
    str  w3, [x2, #SCFG_BOOTLOCPTRH_OFFSET]
    ret

/*---------------------------------------------------------------------------*/

 /* this function returns the base address of the gic distributor
  * in:  none 
  * out: x0 = base address of gic distributor
  * uses x0
  */
_getGICD_BaseAddr:
#if (TEST_BL31)
	 /* defect in simulator - gic base addresses are on 4Kb boundary */
    ldr   x0, =NXP_GICD_4K_ADDR;
#else
    ldr   x0, =NXP_GICD_64K_ADDR;
#endif
    ret

/*---------------------------------------------------------------------------*/

 /* this function returns the base address of the gic controller
  * in:  none 
  * out: x0 = base address of gic controller
  * uses x0
  */
_getGICC_BaseAddr:
#if (TEST_BL31)
	 /* defect in simulator - gic base addresses are on 4Kb boundary */
    ldr   x0, =NXP_GICC_4K_ADDR;
#else
    ldr   x0, =NXP_GICC_64K_ADDR;
#endif
    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_SUSPEND
  * this function puts the calling core into standby state
  * in:  x0 = core mask lsb
  * out: none
  * uses x0
  */
_soc_core_entr_stdby:

     /* X0 = core mask lsb */

    dsb  sy
    isb
    wfi

    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_SUSPEND
  * this function performs SoC-specific programming prior to standby
  * in:  x0 = core mask lsb
  * out: none
  * uses none
  */
_soc_core_prep_stdby:

    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_SUSPEND
  * this function performs any SoC-specific cleanup after standby state
  * in:  x0 = core mask lsb
  * out: none
  * uses none
  */
_soc_core_exit_stdby:

    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_SUSPEND
  * this function performs SoC-specific programming prior to power-down
  * in:  x0 = core mask lsb
  * out: none
  * uses x0, x1, x2, x3, x4, x5
  */
_soc_core_prep_pwrdn:
    mov  x5, x30
    mov  x4, x0

     /* x4 = core mask lsb */

     /* enable CPU retention + set smp */
    mrs  x1, CPUECTLR_EL1
    orr  x1, x1, #0x1
    orr  x1, x1, #CPUECTLR_SMPEN_MASK
    msr  CPUECTLR_EL1, x1

     /* x4 = core mask lsb */

     /* set the retention request in SCFG_RETREQCR
        this is an msb-0 register */
    ldr  x3, =SCFG_RETREQCR_OFFSET
    mov  x0, x3
    bl   read_reg_scfg
    rbit w1, w4
    orr  w1, w0, w1
    mov  x0, x3
    bl   write_reg_scfg

     /* x4 = core mask lsb */

     /* set the PC_PH20_REQ bit in RCPM_PCPH20SETR
        this is an lsb-0 register */
    mov  x1, x4
    mov  x0, #RCPM_PCPH20SETR_OFFSET
    bl   write_reg_rcpm

    mov  x30, x5
    ret

/*---------------------------------------------------------------------------*/

 /* part of CPU_SUSPEND
  * this function puts the calling core into a power-down state
  * in:  x0 = core mask lsb
  * out: none
  * uses x0
  */
_soc_core_entr_pwrdn:

     /* X0 = core mask lsb */

    dsb  sy
    isb
    wfi

    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function cleans up after a core exits power-down
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, x1, x2, x3, x4, x5
_soc_core_exit_pwrdn:
    mov  x5, x30
    mov  x4, x0

     /* x4 = core mask lsb */

     /* set the PC_PH20_REQ bit in RCPM_PCPH20CLRR
        this is an lsb-0 register */
    mov  x1, x4
    mov  x0, #RCPM_PCPH20CLRR_OFFSET
    bl   write_reg_rcpm

     /* x4 = core mask lsb */

     /* unset the retention request in SCFG_RETREQCR
        this is an msb-0 register */
    ldr  x3, =SCFG_RETREQCR_OFFSET
    mov  x0, x3
    bl   read_reg_scfg
    rbit w1, w4
    bic  w1, w0, w1
    mov  x0, x3
    bl   write_reg_scfg

    mov  x30, x5
    ret

/*---------------------------------------------------------------------------*/

#if 0

 // part of CPU_SUSPEND
 // this function puts the cluster into a standby state
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, 
_soc_clstr_entr_stdby:
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function exits the cluster from a standby state
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, 
_soc_clstr_exit_stdby:
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function puts the calling core into a power-down state
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, 
_soc_clstr_entr_pwrdn:
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function cleans up after a cluster exits power-down
 // in:  x0 = core mask lsb
 // out: none
 // uses 
_soc_clstr_exit_pwrdn:
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function puts the system into a standby state
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, 
_soc_sys_entr_stdby:
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function exits the system from a standby state
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, 
_soc_sys_exit_stdby:
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function puts the calling core, and potentially the soc, into a
 // low-power state
 // in:  x0 = core mask lsb
 // out: x0 = 0, success
 //      x0 < 0, failure
 // uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10
_soc_sys_entr_pwrdn:
    mov  x10, x30

     // x0 = core mask lsb

     // save DAIF and mask ints
    mrs  x2, DAIF
    mov  x6, x2
    mov  x1, #DAIF_DATA
    bl   _setCoreData
    mov  x0, #DAIF_SET_MASK
    orr  x6, x6, x0
    msr  DAIF, x6

     // disable icache, dcache, mmu @ EL1
    mov  x1, #SCTLR_I_C_M_MASK
    mrs  x0, sctlr_el1
    bic  x0, x0, x1
    msr  sctlr_el1, x0

     // disable dcache for EL3
    mrs x1, SCTLR_EL3
    bic x1, x1, #SCTLR_C_MASK
     // make sure icache is enabled
    orr x1, x1, #SCTLR_I_MASK
    msr SCTLR_EL3, x1
    isb

     // clean/invalidate the dcache
    mov x0, #1
    bl  _cln_inv_all_dcache

     // IRQ taken to EL3, set SCR_EL3[IRQ]
    mrs  x0, SCR_EL3
    orr  x0, x0, #SCR_IRQ_MASK
    msr  SCR_EL3, x0

     // enable dynamic retention control (CPUECTLR[2:0]) and SMP (CPUECTLR[6])
    mrs  x0, CPUECTLR_EL1
    orr  x0, x0, #CPUECTLR_TIMER_8TICKS
    orr  x0, x0, #CPUECTLR_SMPEN_EN
    msr  CPUECTLR_EL1, x0

     // set WFIL2EN in SCFG_CLUSTERPMCR
    ldr  x0, =SCFG_COREPMCR_OFFSET
    ldr  x1, =COREPMCR_WFIL2EN
    bl   write_reg_scfg

     // request LPM20
    mov  x0, #RCPM_POWMGTCSR_OFFSET
    bl   read_reg_rcpm
    orr  x1, x0, #RCPM_POWMGTCSR_LPM20_REQ
    mov  x0, #RCPM_POWMGTCSR_OFFSET
    bl   write_reg_rcpm

    dsb  sy
    isb
    wfi

    mov  x0, #0
    mov  x30, x10
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_SUSPEND
 // this function performs any necessary cleanup after the soc has exited
 // a low-power state
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, 
_soc_sys_exit_pwrdn:
    mov  x10, x30

     // clear SCR_EL3[IRQ]
    mrs  x0, SCR_EL3
    bic  x0, x0, #SCR_IRQ_MASK
    msr  SCR_EL3, x0

    mov  x30, x10
    ret

/*---------------------------------------------------------------------------*/

 // part of CPU_OFF
 // this function cleans up from phase 2 of the core shutdown sequence
 // in:  x0 = core mask lsb
 // out: none
 // uses x0, x1, x2, x3, x4
_soc_core_phase2_clnup:
    mov  x4, x30

     // x0 = core mask lsb

     // w0 = core mask lsb

    CoreMaskMsb w0, w1

     // w0 = core mask msb

     // unset retention request in SCFG_RETREQCR
    mov  x2, #SCFG_BASE_ADDR
    ldr  w1, [x2, #SCFG_RETREQCR_OFFSET]
    rev  w3, w1
    eor  w3, w3, w0
    rev  w1, w3
    str  w1, [x2, #SCFG_RETREQCR_OFFSET]
    
    dsb  sy
    isb
    mov  x30, x4
    ret

/*---------------------------------------------------------------------------*/

 // this function returns a 64-bit execution address of the core in x0
 // out: x0, address found in BOOTLOCPTRL/H
 // uses x0, x1, x2 
_soc_get_start_addr:
     // get the 64-bit base address of the scfg block
    ldr  x1, =SCFG_BASE_ADDR

     // read the 32-bit BOOTLOCPTRL register (offset 0x604 in the scfg block)
    ldr  w0, [x1, #BOOTLOCPTRL_OFFSET]
     // swap bytes for BE
    rev  w2, w0

     // read the 32-bit BOOTLOCPTRH register (offset 0x600 in the scfg block)
    ldr  w0, [x1, #BOOTLOCPTRH_OFFSET]
    rev  w1, w0
     // create a 64-bit BOOTLOCPTR address
    orr  x0, x2, x1, LSL #32
    ret

/*---------------------------------------------------------------------------*/

 // this function enables/disables the SoC retention request for the core,
 // using a read-modify-write methodology
 // in:  w0 = core mask (msb)
 //      w1 = set or clear bit specified in core mask (0 = clear, 1 = set)
 // out: none
 // uses x0, x1, x2, x3
retention_ctrl:
    ldr  w2, =SCFG_BASE_ADDR
    ldr  w3, [x2, #SCFG_RETREQCR_OFFSET]

     // byte swap for BE
    rev  w3, w3
    bic  w3, w3, w0
    cmp  w1, #0
    b.eq 1f
    orr  w3, w3, w0
1:
    rev  w3, w3
    str  w3, [x2, #SCFG_RETREQCR_OFFSET]
    ret

/*---------------------------------------------------------------------------*/

 // this function returns the lsb bit mask corresponding to the current core
 // the mask is returned in w0.
 // this bit mask references the core in the SoC registers such as
 // BRR, COREDISR where the LSB represents core0
 // in:   none
 // out:  w0 = core mask
 // uses: x0, x1, x2
_get_current_mask:

     // get the cores mpidr value
    mrs  x0, MPIDR_EL1

     // generate a lsb-based mask for the core - this algorithm assumes 4 cores
     // per cluster, and must be adjusted if that is not the case
     // SoC core = ((cluster << 2) + core)
     // mask = (1 << SoC core)
    mov   w1, wzr
    mov   w2, wzr
    bfxil w1, w0, #8, #8  // extract cluster
    bfxil w2, w0, #0, #8  // extract cpu #
    lsl   w1, w1, #2
    add   w1, w1, w2
    mov   w2, #0x1
    lsl   w0, w2, w1
    ret

/*---------------------------------------------------------------------------*/

 // this function sets the security mechanisms in the SoC to implement the
 // Platform Security Policy
 // in:   none
 // out:  none
 // uses 
_set_platform_security:

#if (!SUPPRESS_TZC) && (CNFG_SPD)
    mov x8, x30
    bl arm_tzc400_setup
    mov  x30, x8
#endif

    ret

/*---------------------------------------------------------------------------*/

 // this function makes any needed soc-specific configuration changes when boot
 // services end
_soc_exit_boot_svcs:

    ret

/*---------------------------------------------------------------------------*/

 // write a register in the SCFG block
 // in:  x0 = offset
 // in:  w1 = value to write
 // uses x0, x1, x2
write_reg_scfg:
     // swap for BE
    rev  w2, w1
    mov  x1, #SCFG_BASE_ADDR
    str  w2, [x1, x0]
    ret

/*---------------------------------------------------------------------------*/

 // read a register in the SCFG block
 // in:  x0 = offset
 // out: w0 = value read
 // uses x0, x1, x2
read_reg_scfg:
    mov  x2, #SCFG_BASE_ADDR
    ldr  w1, [x2, x0]
     // swap for BE
    rev  w0, w1
    ret

/*---------------------------------------------------------------------------*/

 // write a register in the DCFG block
 // in:  x0 = offset
 // in:  w1 = value to write
 // uses x0, x1, x2
write_reg_dcfg:
     // swap for BE
    rev  w2, w1
    mov  x1, #DCFG_BASE_ADDR
    str  w2, [x1, x0]
    ret

/*---------------------------------------------------------------------------*/

 // read a register in the DCFG block
 // in:  x0 = offset
 // out: w0 = value read
 // uses x0, x1, x2
read_reg_dcfg:
    mov  x2, #DCFG_BASE_ADDR
    ldr  w1, [x2, x0]
     // swap for BE
    rev  w0, w1
    ret

/*---------------------------------------------------------------------------*/

 // write a register in the SYS_COUNTER block
 // in:  x0 = offset
 // in:  w1 = value to write
 // uses x0, x1, x2
write_reg_sys_counter:
     // swap for BE
    rev  w2, w1
    mov  x1, #SYS_COUNTER_BASE
    str  w2, [x1, x0]
    ret

/*---------------------------------------------------------------------------*/

 // read a register in the SYS_COUNTER block
 // in:  x0 = offset
 // out: w0 = value read
 // uses x0, x1, x2
read_reg_sys_counter:
    mov  x2, #SYS_COUNTER_BASE
    ldr  w1, [x2, x0]
     // swap for BE
    rev  w0, w1
    ret

/*---------------------------------------------------------------------------*/

 // write a register in the GIC400 distributor block
 // in:  x0 = offset
 // in:  w1 = value to write
 // uses x0, x1, x2
write_reg_gicd:
    ldr  x2, =GICD_BASE_ADDR
    str  w1, [x2, x0]
    ret

/*---------------------------------------------------------------------------*/

 // read a register in the GIC400 distributor block
 // in:  x0 = offset
 // out: w0 = value read
 // uses x0, x1, x2
read_reg_gicd:
    ldr  x2, =GICD_BASE_ADDR
    ldr  w0, [x2, x0]
    ret

/*---------------------------------------------------------------------------*/

 // write a register in the GIC400 CPU interface block
 // in:  x0 = offset
 // in:  w1 = value to write
 // uses x0, x1, x2
write_reg_gicc:
    ldr  x2, =GICC_BASE_ADDR
    str  w1, [x2, x0]
    ret

/*---------------------------------------------------------------------------*/

 // read a register in the GIC400 CPU interface block
 // in:  x0 = offset
 // out: w0 = value read
 // uses x0, x1, x2
read_reg_gicc:
    ldr  x2, =GICC_BASE_ADDR
    ldr  w0, [x2, x0]
    ret

/*---------------------------------------------------------------------------*/

 // this is soc initialization task 1
 // this function releases a secondary core
 // in:  x0 = core mask lsb of the secondary core to put to work
 // out: none
 // uses x0, x1, x2, x3, x4, x5
init_task_1:
    mov  x5, x30
    mov  x4, x0

     // x0 = core mask lsb
     // x4 = core mask lsb

     // save the core mask
    bl   _set_task1_core

     // load bootlocptr with start addr
    adr  x0, _prep_ocram_init
    bl   _soc_set_start_addr

     // x4 = core mask lsb

     // release secondary core
    mov  x0, x4
    bl  _soc_core_release

    mov  x30, x5
    ret

/*---------------------------------------------------------------------------*/

#endif
