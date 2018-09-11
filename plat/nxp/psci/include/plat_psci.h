/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Rod Dorris <rod.dorris@nxp.com>
 */

#ifndef __PLAT_PSCI_H__
#define __PLAT_PSCI_H__


 /* core abort current op */
#define CORE_ABORT_OP     0x1

 /* core state */
 /* OFF states 0x0 - 0xF */
#define CORE_IN_RESET     0x0
#define CORE_DISABLED     0x1
#define CORE_OFF          0x2
#define CORE_STANDBY      0x3
#define CORE_PWR_DOWN     0x4
#define CORE_WFE          0x6
#define CORE_WFI          0x7
#define CORE_LAST		  0x8
#define CORE_OFF_PENDING  0x9
#define CORE_WORKING_INIT 0xA

 /* ON states 0x10 - 0x1F */
#define CORE_PENDING      0x10
#define CORE_RELEASED     0x11
#define CORE_WAKEUP       0x12
 /* highest off state */
#define CORE_OFF_MAX	  0xF
 /* lowest on state */
#define CORE_ON_MIN       CORE_PENDING

#define  DAIF_SET_MASK          0x3C0
#define  SCTLR_I_C_M_MASK       0x00001005
#define  SCTLR_C_MASK           0x00000004
#define  SCTLR_I_MASK           0x00001000
#define  CPUACTLR_L1PCTL_MASK   0x0000E000
#define  DCSR_RCPM2_BASE        0x20170000
#define  CPUECTLR_SMPEN_MASK    0x40
#define  CPUECTLR_SMPEN_EN      0x40
#define  CPUECTLR_RET_MASK      0x7
#define  CPUECTLR_RET_SET       0x2
#define  CPUECTLR_TIMER_MASK    0x7
#define  CPUECTLR_TIMER_8TICKS  0x2
#define  SCR_FIQ_MASK           0x4

#define CPUECTLR_EL1  S3_1_C15_C2_1
#define CPUACTLR_EL1  S3_1_C15_C2_0

#ifndef __ASSEMBLER__

void __dead2 _psci_system_reset(void);
void __dead2 _psci_system_off(void);
int _psci_cpu_on(u_register_t core_mask);
void _psci_cpu_prep_off(u_register_t core_mask);
void __dead2 _psci_cpu_off_wfi(u_register_t core_mask);
void _psci_wakeup(u_register_t core_mask);

#endif

#endif /* __PLAT_PSCI_H__ */
