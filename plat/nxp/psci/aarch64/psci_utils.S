
/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <psci.h>
#include "platform_def.h"
#include "plat_psci.h"
#include "bl31_data.h"

/*---------------------------------------------------------------------------*/

#define RESET_RETRY_CNT   800
#define PSCI_ABORT_CNT    100

/*---------------------------------------------------------------------------*/

//.global _psci_cpu_suspend

/******************************************************************************
 * int _psci_cpu_on(u_register_t core_mask)
 *****************************************************************************/

#if (SOC_CORE_RELEASE)

.global _psci_cpu_on

func _psci_cpu_on

     /* x0   = target cpu core mask */

     /* called from C, so save the non-volatile regs
      * save these as pairs of registers to maintain the
      *  required 16-byte alignment on the stack */
    stp  x4,  x5,  [sp, #-16]!
    stp  x6,  x7,  [sp, #-16]!
    stp  x8,  x9,  [sp, #-16]!
    stp  x10, x11, [sp, #-16]!
    stp  x12, x13, [sp, #-16]!
    stp  x14, x15, [sp, #-16]!
    stp  x16, x17, [sp, #-16]!
    stp  x18, x30, [sp, #-16]!

    mov  x6, x0

     /* x0   = core mask (lsb) */
     /* x6   = core mask (lsb) */

     /* check if core disabled */
    bl   _soc_ck_disabled   // 0-2
    cbnz w0, psci_disabled

     /* check core data area to see if core cannot be turned on
      * read the core state */
    mov  x0, x6
    bl   _getCoreState       // 0-5
    mov  x9, x0

     /* x6   = core mask (lsb)
      * x9   = core state (from data area) */

    cmp  x9, #CORE_DISABLED
    mov  x0, #PSCI_E_DISABLED
    b.eq cpu_on_done

    cmp  x9, #CORE_PENDING
    mov  x0, #PSCI_E_ON_PENDING
    b.eq cpu_on_done

    cmp  x9, #CORE_RELEASED
    mov  x0, #PSCI_E_ALREADY_ON
    b.eq cpu_on_done

8:
     /* x6   = core mask (lsb)
      * x9   = core state (from data area) */

    cmp  x9, #CORE_WFE
    b.eq core_in_wfe
    cmp  x9, #CORE_IN_RESET
    b.eq core_in_reset
    cmp  x9, #CORE_OFF
    b.eq core_is_off
    cmp  x9, #CORE_OFF_PENDING
     /* if state == CORE_OFF_PENDING, set abort */
    mov  x0, x6
    mov  x1, #ABORT_FLAG_DATA
    mov  x2, #CORE_ABORT_OP
    bl   _setCoreData   // 0-3, [13-15]

    ldr  x3, =PSCI_ABORT_CNT
7:
     /* watch for abort to take effect */
    mov  x0, x6
    bl   _getCoreState  // 0-5
    cmp  x0, #CORE_OFF
    b.eq core_is_off
    cmp  x0, #CORE_PENDING
    mov  x0, #PSCI_E_SUCCESS
    b.eq cpu_on_done

     /* loop til finished */
    sub  x3, x3, #1
    cbnz x3, 7b

     /* if we didn't see either CORE_OFF or CORE_PENDING, then this
      * core is in CORE_OFF_PENDING - exit with success, as the core will
      * respond to the abort request */
    mov  x0, #PSCI_E_SUCCESS
    b    cpu_on_done

 /* this is where we start up a core out of reset */
core_in_reset:
     /* see if the soc-specific module supports this op */
    ldr  x7, =SOC_CORE_RELEASE
    cbnz  x7, 3f

    mov  x0, #PSCI_E_NOT_SUPPORTED
    b    cpu_on_done

     /* x6   = core mask (lsb) */
3:
     /* set core state in data area */
    mov  x0, x6
    mov  x1, #CORE_PENDING
    bl   _setCoreState   // 0-3, [13-15]

     /* release the core from reset */
    mov   x0, x6
    bl    _soc_core_release // 0-3
    mov   x0, #PSCI_E_SUCCESS
    b     cpu_on_done

 /* this is where we start up a core that has been powered-down via CPU_OFF */
core_is_off:
     /* see if the soc-specific module supports this op */
    ldr  x7, =SOC_CORE_RESTART
    cbnz x7, 2f

    mov  x0, #PSCI_E_NOT_SUPPORTED
    b    cpu_on_done

     /* x6   = core mask (lsb) */
2:
     /* set core state in data area */
    mov  x0, x6
    mov  x1, #CORE_WAKEUP
    bl   _setCoreState   // 0-3, [13-15]

     /* put the core back into service */
    mov  x0, x6
    bl   _soc_core_restart    // 0-5
    mov  x0, #PSCI_E_SUCCESS
    b    cpu_on_done

 /* this is where we release a core that is being held in wfe */
core_in_wfe:
     /* x6   = core mask (lsb) */

     /* set core state in data area */
    mov  x0, x6
    mov  x1, #CORE_PENDING
    bl   _setCoreState   // 0-3, [13-15]
    dsb  sy
    isb

     /* put the core back into service */
    sev
    sev
    isb
    mov  x0, #PSCI_E_SUCCESS

cpu_on_done:
     // restore the aarch32/64 non-volatile registers
    ldp  x18, x30, [sp], #16
    ldp  x16, x17, [sp], #16
    ldp  x14, x15, [sp], #16
    ldp  x12, x13, [sp], #16
    ldp  x10, x11, [sp], #16
    ldp  x8,  x9,  [sp], #16
    ldp  x6,  x7,  [sp], #16
    ldp  x4,  x5,  [sp], #16
    b    psci_completed
endfunc _psci_cpu_on

#endif

#if (SOC_CORE_OFF)

/******************************************************************************
 * void _psci_cpu_prep_off(u_register_t core_mask)
 * this function performs the SoC-specific programming prior
 * to shutting the core down
 *****************************************************************************/

.global _psci_cpu_prep_off
.global _psci_cpu_off_wfi

func _psci_cpu_prep_off
    mov  x29, x30

     /* x0 = core_mask */
    mov  x10, x0

     /* the core does not return from cpu_off, so no need
      * to save/restore non-volatile registers */

     /* change state of core in data area */
    mov  x0, x10
    mov  x1, #CORE_OFF_PENDING
    bl   _setCoreState

     /* prep the core for shutdown - phase 1 */
    mov  x0, x10 
    bl   _soc_core_phase1_off

     /* prep the core for shutdown - phase 2 */
    mov  x0, x10
    bl   _soc_core_phase2_off

    mov  x30, x29
    b    psci_completed

endfunc _psci_cpu_prep_off

/******************************************************************************
 * void _psci_cpu_off_wfi(u_register_t core_mask, u_register_t resume_addr)
 * this function shuts down the core
 *****************************************************************************/

func _psci_cpu_off_wfi
     /* save the wakeup address */
    mov  x29, x1

     /* shutdown the core */
    bl   _soc_core_entr_off

     /* branch to resume execution */
    br   x29
endfunc _psci_cpu_off_wfi

#endif

/*---------------------------------------------------------------------------*/

#if (SOC_SYSTEM_RESET)

.global _psci_system_reset

func _psci_system_reset

     /* system reset is mandatory
      * system reset is soc-specific
      * Note: under no circumstances do we return from this call */
    bl   _soc_sys_reset
endfunc _psci_system_reset

#endif

/*---------------------------------------------------------------------------*/

#if (SOC_SYSTEM_OFF)

.global _psci_system_off

func _psci_system_off

     /* system off is mandatory
      * system off is soc-specific
      * Note: under no circumstances do we return from this call */
    b    _soc_sys_off
endfunc _psci_system_off

#endif

/******************************************************************************
 * void _psci_wakeup(u_register_t core_mask)
 * this function performs the SoC-specific programming
 * after a core wakes up from OFF
 *****************************************************************************/

#if (SOC_CORE_RESTART)

.global _psci_wakeup

func _psci_wakeup
    mov  x29, x30

     /* x0 = core mask */

     /* start the core back up */
    bl   _soc_core_exit_off

    mov  x30, x29
    b    psci_completed
endfunc _psci_wakeup

#endif

/*---------------------------------------------------------------------------*/

#if 0
func _psci_migrate
     /* the return value of this function must be in synch with the
      * return value of migrate_info
      */
    b  psci_unimplemented
endfunc _psci_migrate
#endif

/*---------------------------------------------------------------------------*/

#if 0
func _psci_migrate_info
     /* migrate not needed when Trusted OS not installed */
    mov  w0, #MIGRATE_TYPE_NMIGRATE
    b    psci_completed
endfunc _psci_migrate_info
#endif

/*---------------------------------------------------------------------------*/

#if 0
func _psci_migrate_info_upcpu
     /* the return value of this function must be in synch with the
      * return value of migrate_info
      */
    b  psci_unimplemented
endfunc _psci_migrate_info_upcpu
#endif

/*---------------------------------------------------------------------------*/

func _psci_cpu_suspend
#if 0
     /* x0 = function id
      * x1 = power state
      * x2 = entry point address
      * x3 = context id */
    mov  x8, x1
    mov  x9, x2
    mov  x10, x3
     /* x8  = power state
      * x9  = entry point address
      * x10 = context id */

     /* check parameters */
    ldr  x0, =POWER_STATE_MASK
    mvn  x0, x0
    and  x0, x1, x0
    cbnz x0, psci_invalid

     /* power level */
    ldr  x0, =POWER_LEVEL_MASK
    and  x0, x1, x0
    lsr  x0, x0, #24
    cmp  x0, #PWR_STATE_CORE_LVL
    b.eq power_state_core
    cmp  x0, #PWR_STATE_CLUSTER_LVL
    b.eq power_state_cluster
    cmp  x0, #PWR_STATE_SYSTEM_LVL
    b.eq power_state_system
    b    psci_invalid

 /* if it is a core power state */
power_state_core:
     /* x8  = power state
      * x9  = entry point address
      * x10 = context id */

    ldr  x0, =STATE_TYPE_MASK
    and  x0, x8, x0
    lsr  x0, x0, #16

    cmp  x0, #PWR_STATE_STANDBY
    b.eq core_in_standby
    cmp  x0, #PWR_STATE_PWR_DOWN
    b.eq core_in_powerdown
     /* else we have an invalid parameter */
    b    psci_invalid

core_in_standby:
     /* see if this functionality is supported in the soc-specific code */
    mov  x7, #SOC_CORE_STANDBY
    cbz  x7, psci_unimplemented

    mrs  x0, MPIDR_EL1
    bl   plat_core_mask
    mov  x11, x0
    mov  x1, #CORE_STANDBY
    bl   _setCoreState

     /* save cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    mrs  x2, CPUECTLR_EL1
    bl   _setCoreData

     /* x11 = core mask lsb */

     /* put the core into standby */
    mov  x0, x11
    bl   _soc_core_entr_stdby

     /* cleanup after the core exits standby */
    mov  x0, x11
    bl   _soc_core_exit_stdby

    mov  x0, x11
    mov  x1, #CORE_RELEASED
    bl   _setCoreState

     /* restore cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    bl   _getCoreData       // 0-5
    msr  CPUECTLR_EL1, x0

    b    psci_success

core_in_powerdown:
     /* see if this functionality is supported in the soc-specific code */
    mov  x7, #SOC_CORE_PWR_DWN
    cbz  x7, psci_unimplemented

     /* x9  = entry point address
      * x10 = context id */

    bl   plat_my_core_mask
    mov  x11, x0

     /* x9  = entry point address
      * x11 = core mask lsb */

    mov  x0, x11
    mov  x1, #CORE_PWR_DOWN
    bl   _setCoreState

     /* save cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    mrs  x2, CPUECTLR_EL1
    bl   _setCoreData

     /* enter power-down */
    mov  x0, x11
    bl   _soc_core_entr_pwrdn

     /* x11 = core mask lsb */

    mov  x0, x11
    bl   _soc_core_exit_pwrdn

    mov  x0, x11
    mov  x1, #CORE_RELEASED
    bl   _setCoreState

     /* restore cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    bl   _getCoreData       // 0-5
    msr  CPUECTLR_EL1, x0

     /* x11 = core mask lsb */

     /* return to entry point address */
    mov  x0, x11
    mov  x1, #START_ADDR_DATA
    bl   _getCoreData       // 0-5
    msr  ELR_EL3, x0

    mov  x0, x11
    mov  x1, #CNTXT_ID_DATA
    bl   _getCoreData       // 0-5

     /* we have a context id in x0 - don't overwrite this
      * with a status return code */
    b    psci_completed

     /*------------------------------------------ */

 /* if it is a cluster power state */
power_state_cluster:
     /* x8  = power state
      * x9  = entry point address
      * x10 = context id */

     /* get mpidr, extract cluster number */
    mrs  x0, mpidr_el1
    and  x0, x0, #MPIDR_CLUSTER_MASK

     /* x0 = cluster number in mpidr format */

     /* see if this is the last active core of the cluster */
    bl   _core_on_cnt_clstr

     /* if this is not the last active core of the cluster, return with error */
    cmp  x0, #1
    b.gt psci_invalid

     /* determine the power level */
    ldr  x0, =STATE_TYPE_MASK
    and  x0, x8, x0
    lsr  x0, x0, #16

    cmp  x0, #PWR_STATE_STANDBY
    b.eq cluster_in_stdby
    cmp  x0, #PWR_STATE_PWR_DOWN
    b.eq cluster_in_pwrdn
     /* else we have an invalid parameter */
    b    psci_invalid

cluster_in_stdby:
     /* see if this functionality is supported in the soc-specific code */
    mov  x7, #SOC_CLUSTER_STANDBY
    cbz  x7, psci_unimplemented

     /* to put the cluster in stdby, we also have to 
      * put this core in stdby */
    mrs  x0, MPIDR_EL1
    bl   plat_core_mask
    mov  x11, x0
    mov  x1, #CORE_STANDBY
    bl   _setCoreState

     /* save cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    mrs  x2, CPUECTLR_EL1
    bl   _setCoreData

     /* x11 = core mask lsb */

    mov  x0, x11
    bl   _soc_clstr_entr_stdby

     /* cleanup after the cluster exits standby */
    mov  x0, x11
    bl   _soc_clstr_exit_stdby

    mov  x0, x11
    mov  x1, #CORE_RELEASED
    bl   _setCoreState

     /* restore cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    bl   _getCoreData       // 0-5
    msr  CPUECTLR_EL1, x0

    b    psci_success

cluster_in_pwrdn:
     /* see if this functionality is supported in the soc-specific code */
    mov  x7, #SOC_CLUSTER_PWR_DWN
    cbz  x7, psci_unimplemented

     /* x10 = context id */

    bl   plat_my_core_mask
    mov  x11, x0

     /* x11 = core mask */

     /* to put the cluster in power down, we also
      * have to power-down this core */
    mov  x0, x11
    mov  x1, #CORE_PWR_DOWN
    bl   _setCoreState

     /* save cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    mrs  x2, CPUECTLR_EL1
    bl   _setCoreData

    mov  x0, x11
    bl   _soc_clstr_entr_pwrdn

     /* cleanup after the cluster exits power-down */
    mov  x0, x11
    bl   _soc_clstr_exit_pwrdn

    mov  x0, x11
    mov  x1, #CORE_RELEASED
    bl   _setCoreState

     /* restore cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    bl   _getCoreData       // 0-5
    msr  CPUECTLR_EL1, x0

     /* return to entry point address */
    mov  x0, x11
    mov  x1, #START_ADDR_DATA
    bl   _getCoreData       // 0-5
    msr  ELR_EL3, x0

    mov  x0, x11
    mov  x1, #CNTXT_ID_DATA
    bl   _getCoreData       // 0-5

     /* we have a context id in x0 - don't overwrite this
      * with a status return code */
    b    psci_completed

     /*------------------------------------------ */

 /* if it is a system power state */
power_state_system:
     /* x8  = power state
      * x9  = entry point address
      * x10 = context id */

     /* see if this is the last active core of the system */
    bl   core_on_cnt_sys

     /* if this is not the last active core of the system, return with error */
    cmp  x0, #1
    b.gt  psci_invalid

     /* determine the power level */
    ldr  x0, =STATE_TYPE_MASK
    and  x0, x8, x0
    lsr  x0, x0, #16

    cmp  x0, #PWR_STATE_STANDBY
    b.eq system_in_stdby
    cmp  x0, #PWR_STATE_PWR_DOWN
    b.eq system_in_pwrdn
     /* else we have an invalid parameter */
    b    psci_invalid

system_in_stdby:
     /* see if this functionality is supported in the soc-specific code */
    mov  x7, #SOC_SYSTEM_STANDBY
    cbz  x7, psci_unimplemented

     /* to put the system in stdby, we also have to 
      * put this core in stdby */
    mrs  x0, MPIDR_EL1
    bl   plat_core_mask
    mov  x11, x0
    mov  x1, #CORE_STANDBY
    bl   _setCoreState

     /* save cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    mrs  x2, CPUECTLR_EL1
    bl   _setCoreData

     /* x11 = core mask lsb */

    mov  x0, x11
    bl   _soc_sys_entr_stdby

     /* cleanup after the system exits standby */
    mov  x0, x11
    bl   _soc_sys_exit_stdby

    mov  x0, x11
    mov  x1, #CORE_RELEASED
    bl   _setCoreState

     /* restore cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    bl   _getCoreData       // 0-5
    msr  CPUECTLR_EL1, x0

    b    psci_success

system_in_pwrdn:
     /* see if this functionality is supported in the soc-specific code */
    mov  x7, #SOC_SYSTEM_PWR_DWN
    cbz  x7, psci_unimplemented

    bl   plat_my_core_mask
    mov  x11, x0

     /* x11 = core mask */

    mov  x0, x11
    mov  x1, #CORE_PWR_DOWN
    bl   _setCoreState

     /* save cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    mrs  x2, CPUECTLR_EL1
    bl   _setCoreData

     /* disable caches, mmu at EL1 */
    mrs  x0, sctlr_el1
    mov  x1, #SCTLR_I_C_M_MASK
    bic  x0, x0, x1
    msr  sctlr_el1, x0

    mov  x0, x11
    bl   _soc_sys_entr_pwrdn
     /* we have an return status code in x0 */
    mov  x7, x0
    cbnz x7, 2f

     /* cleanup after the system exits power-down */
    mov  x0, x11
    bl   _soc_sys_exit_pwrdn

     /* x11 = core mask lsb */
2:
    mov  x0, x11
    mov  x1, #CORE_RELEASED
    bl   _setCoreState

     /* restore cpuectlr */
    mov  x0, x11
    mov  x1, #CPUECTLR_DATA
    bl   _getCoreData       // 0-5
    msr  CPUECTLR_EL1, x0

     /* if we have an error, return to the caller rather
      * than the entry point address */
    cbz  x7, 1f
    b    psci_invalid

1:
     /* return to entry point address */
    mov  x0, x11
    mov  x1, #START_ADDR_DATA
    bl   _getCoreData       // 0-5
    msr  ELR_EL3, x0

    mov  x0, x11
    mov  x1, #CNTXT_ID_DATA
    bl   _getCoreData       // 0-5

     /* we have a context id in x0 - don't overwrite this
      * with a status return code */
#endif
    b    psci_completed
endfunc _psci_cpu_suspend

/*---------------------------------------------------------------------------*/

#if 0
func _psci_affinity_info
     /* x1 = target_affinity
      * x2 = lowest_affinity */

     /* core affinity? */
    mov   x0, #0
    cmp   x2, x0
    b.eq  affinity_info_0

     /* cluster affinity? */
    mov   x0, #1
    cmp   x2, x0
    b.eq  affinity_info_1

     /* no other processing elements are present */
    b   psci_not_present

affinity_info_0:
     /* status of an individual core
      * x1 = target_affinity */

    mov   x0, x1
    bl    plat_core_mask
    cbz   x0, psci_not_present

     /* x0 = core mask */

     /* process cores here */
    bl   _getCoreState

     /* x0 = core state */

     /* ck for core disabled */
    ldr   x1, =CORE_DISABLED
    cmp   x0, x1
    b.ne  1f
    b     psci_disabled

1:
     /* ck for core pending */
    ldr   x1, =CORE_PENDING
    cmp   x0, x1
    b.ne  2f
    b     affinity_lvl_pend

2:
     /* ck for core on */
    ldr   x1, =CORE_RELEASED
    cmp   x0, x1
    b.ne  3f
    b     affinity_lvl_on

3:
     /* must be core off */
    b     affinity_lvl_off

affinity_info_1:
     /* status of a cluster
      * x1 = target_affinity */

     /* isolate and check the cluster number */
    mov   x2, xzr
    bfxil x2, x1, #8, #8
    ldr   x3, =CLUSTER_COUNT
    cmp   x3, x2
    b.le  psci_not_present

     /* x2 = cluster number */

    bl    _get_cluster_state
    b     psci_success
endfunc _psci_affinity_info
#endif

/*---------------------------------------------------------------------------*/

#if 0
func _psci_version
    ldr  x0, =PSCI_VERSION
    b    psci_completed
endfunc _psci_version
#endif

/*---------------------------------------------------------------------------*/

#if 0
func _psci_features
    b  psci_unimplemented
endfunc _psci_features
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#if 0
 /* returns for affinity_info */

affinity_lvl_on:
    mov  x0, #AFFINITY_LEVEL_ON
    b    psci_completed

     /*------------------------------------------ */

affinity_lvl_off:
    mov  x0, #AFFINITY_LEVEL_OFF
    b    psci_completed

     /*------------------------------------------ */

affinity_lvl_pend:
    mov  x0, #AFFINITY_LEVEL_PEND
    b    psci_completed
#endif

/*---------------------------------------------------------------------------*/
 /* psci std returns */

psci_disabled:
    ldr  w0, =PSCI_E_DISABLED
    b    psci_completed

     /*------------------------------------------ */

psci_not_present:
    ldr  w0, =PSCI_E_NOT_PRESENT
    b    psci_completed

     /*------------------------------------------ */

psci_on_pending:
    ldr  w0, =PSCI_E_ON_PENDING
    b    psci_completed

     /*------------------------------------------ */

psci_already_on:
    ldr  w0, =PSCI_E_ALREADY_ON
    b    psci_completed

     /*------------------------------------------ */

psci_failure:
    ldr  w0, =PSCI_E_INTERN_FAIL
    b    psci_completed

     /*------------------------------------------ */

psci_unimplemented:
    ldr  w0, =PSCI_E_NOT_SUPPORTED
    b    psci_completed

     /*------------------------------------------ */

psci_denied:
    ldr  w0, =PSCI_E_DENIED
    b    psci_completed

     /*------------------------------------------ */

psci_invalid:
    ldr  w0, =PSCI_E_INVALID_PARAMS
    b    psci_completed

     /*------------------------------------------ */

psci_success:
    mov  x0, #PSCI_E_SUCCESS

psci_completed:
     /* x0 = status code */
    ret

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#if 0
 /* this function stores the sctlr_elx value of the calling entity
  * in:   w0 = core mask (lsb)
  *       w1 = SPSR EL-level (must be one of: SPSR_EL1, SPSR_EL2)
  * uses: x0, x1, x2, x3, x4 */
save_core_sctlr:
    mov   x4, x30

     /* x0 = core mask lsb */

    cmp   w1, #SPSR_EL1
    b.eq  1f
    mrs   x2, sctlr_el2
    b     2f
1:
    mrs   x2, sctlr_el1
2:

     /* x0 = core mask lsb */
     /* x2 = sctlr value to save */

    mov  x1, #SCTLR_DATA
    bl   _setCoreData

    mov   x30, x4 
    ret
#endif

/*---------------------------------------------------------------------------*/

#if 0
 /* this function processes a request to abort CPU_OFF - an abort request can
  * occur if we are processing CPU_OFF, and a CPU_ON is issued for the same core
  * in:   w0 = core mask (lsb)
  * out:  none
  * uses: x0, x1, x2, x3, x4, x5 */
psci_processAbort:
    mov  x5, x30

     /* x0 = core mask lsb */

     /* clear the abort flag */
    mov   x4, x0
    mov   x1, #ABORT_FLAG_DATA
    mov   x2, xzr
    bl    _setCoreData

     /* set the core state to CORE_PENDING */
    mov   x0, x4
    mov   x1, #CORE_PENDING
    bl    _setCoreState

    mov   x30, x5 
    ret
#endif

/*---------------------------------------------------------------------------*/

#if 0
 /* this function locates a core that is available to perform an
  * initialization task
  * in:  none
  * out: x0 = 0, no available core
  *      x0 = core mask lsb of available core
  * uses x0, x1, x2, x3, x4 */
_find_core:

    mov   x4, x30

     /* start the search at core 1 */
    mov   x3, #2
3:
     /* see if core is disabled */
    mov   x0, x3
    bl    _soc_ck_disabled
    cbnz  x0, 1f

     /* x3 = core mask lsb */

     /* get the state of the core */
    mov   x0, x3
    bl    _getCoreState
     
     /* x0 = core state */

     /* see if core is in reset - this is the state we want */
    mov   x1, #CORE_IN_RESET
    cmp   x0, x1
    mov   x0, x3
    b.eq  2f 
1:
    cmp   x3, #CORE_MASK_MAX
    mov   x0, xzr
    b.eq  2f

    lsl   x3, x3, #1
    b     3b
2:
    mov   x30, x4
    ret
#endif

/*---------------------------------------------------------------------------*/

#if 0
 /* this function returns the number of cores that are ON in the LS2080, based on
  * the core state read from each core's data area. The current core will be counted
  * as 'ON' if its data area indicates so
  * in:   none
  * out:  x0 = number of cores that are ON
  * uses: x0, x1, x2, x3, x4, x5 */
coreOnCount:
    mov x5, x30
   
    ldr x4, =CPU_MAX_COUNT 
    mov x2, xzr
    mov x3, #1

     /* x2 = number of cores on
      * x3 = core mask lsb
      * x4 = loop count */
1:  
    mov  x0, x3
    bl   _getCoreState

     /* x0 = core state */

     /* if core state <= CORE_OFF_MAX, core is OFF */
    cmp  x0, #CORE_OFF_MAX
    b.ls 2f
     /* core is not OFF, so increment count                   */
    add  x2, x2, #1

2:  
     /* decrement loop counter */
    sub  x4, x4, #1
    cbz  x4, 3f
     /* shift mask bit to select next core */
    lsl  x3, x3, #1
    b    1b

3:
     /* put result in R0, and restore link register */
    mov  x0, x2     
    mov  x30, x5     
    ret
#endif

/*---------------------------------------------------------------------------*/

#if 0
 /* this function returns the number of active cores in the system
  * in:  none
  * out: x0 = count of cores running
  * uses x0, x1, x2, x3, x4, x5, x6 */
core_on_cnt_sys:
    mov  x6, x30
    ldr  x3, =CPU_MAX_COUNT
    mov  x4, #1
    mov  x5, xzr

     /* x3 = loop count
      * x4 = core mask lsb
      * x5 = accumulated count of running cores
      * x6 = saved link reg */

3:
    mov  x0, x4
    bl   _getCoreState

     /* x0 = core state */

    cmp  x0, #CORE_OFF_MAX
    b.le 1f
    add  x5, x5, #1
1:
     /* decrement the loop count and exit if finished */
    sub  x3, x3, #1
    cbz  x3, 2f

     /* increment to the next core */
    lsl  x4, x4, #1
    b    3b
2:
     /* xfer the count to the output reg */
    mov  x0, x5
    mov  x30, x6
    ret
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

