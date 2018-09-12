/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Jiafei Pan <jiafei.pan@nxp.com>
 *        Sumit Garg <sumit.garg@nxp.com>
 *	  Rod Dorris <rod.dorris@nxp.com>
 */
	
#include <platform_def.h>
#include <arch_helpers.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <assert.h>
#include <platform.h>
#include <psci.h>
#include <mmio.h>
#include <sys/endian.h>
#include <delay_timer.h>
#include <plat_psci.h>
#include <plat_common.h>
#include <context_mgmt.h>
#include "psci_private.h"

void el3_exit(void);

#if (SOC_CORE_RELEASE)
 /* the entry for core warm boot */
static uintptr_t warmboot_entry = (uintptr_t) NULL;

static int _pwr_domain_on(u_register_t mpidr)
{
	int core_pos = plat_core_pos(mpidr);
	int rc = PSCI_E_INVALID_PARAMS;
	u_register_t core_mask;

	if (core_pos >= 0 && core_pos < PLATFORM_CORE_COUNT) {

		_soc_set_start_addr(warmboot_entry);

		dsb();
		isb();

		core_mask = (1 << core_pos);
		rc = _psci_cpu_on(core_mask);
	}

	return (rc);
}
#endif

#if (SOC_CORE_OFF)
static void _pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t core_mask = plat_my_core_mask();

	_psci_cpu_prep_off(core_mask);
}

static void __dead2 _pwr_down_wfi(const psci_power_state_t *target_state)
{
	u_register_t core_mask = plat_my_core_mask();


	_psci_cpu_off_wfi(core_mask, warmboot_entry);

}
#endif

#if (SOC_CORE_RELEASE || SOC_CORE_RESTART)
static void _pwr_domain_wakeup(const psci_power_state_t *target_state)
{
	u_register_t core_mask  = plat_my_core_mask();
	u_register_t core_state = _getCoreState(core_mask);

	switch (core_state) {
	case CORE_PENDING : /* this core is coming out of reset */

		 /* soc per cpu setup */
		soc_init_percpu();

		 /* gic per cpu setup */
		plat_ls_gic_pcpu_init();

		 /* set core state in internal data */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
		break;

#if (SOC_CORE_RESTART)
	case CORE_WAKEUP :

		 /* this core is waking up from OFF */
		_psci_wakeup(core_mask);

		 /* set core state in internal data */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);

		break;
#endif
	}
}
#endif

static plat_psci_ops_t _psci_pm_ops = {
#if (SOC_SYSTEM_OFF)
	.system_off = _psci_system_off,
#endif
#if (SOC_SYSTEM_RESET)
	.system_reset = _psci_system_reset,
#endif
#if (SOC_CORE_RELEASE || SOC_CORE_RESTART)
	 /* core released or restarted */
	.pwr_domain_on_finish = _pwr_domain_wakeup,
#endif
#if (SOC_CORE_OFF)
	 /* core shutting down */
	.pwr_domain_off	= _pwr_domain_off,
	.pwr_domain_pwr_down_wfi = _pwr_down_wfi,
#endif
#if (SOC_CORE_RELEASE)
	 /* core executing psci_cpu_on */
	.pwr_domain_on	= _pwr_domain_on
#endif
};

#if (SOC_CORE_RELEASE)
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	warmboot_entry = sec_entrypoint;
	*psci_ops = &_psci_pm_ops;
	return 0;
}

#else

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &_psci_pm_ops;
	return 0;
}
#endif

