/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <platform_def.h>
#include <plat_common.h>
#include <mmio.h>
#include <debug.h>

void enable_timer_base_to_cluster(void)
{
	uint32_t *ccsr_svr = (uint32_t *)(NXP_DCFG_ADDR + DCFG_SVR_OFFSET);
	uint32_t soc_dev_id = FETCH_SOC_ID(mmio_read_32((uintptr_t)ccsr_svr));
	uint32_t *cltbenr = NULL;
	uint32_t cltbenr_val = 0;

	if (soc_dev_id == INVALID_SOC_ID) {
		NOTICE("ERROR: Unknown DCFG endianness: SVR = 0x%x\n",
								soc_dev_id);
		return;
	}


	cltbenr = (uint32_t *)(NXP_PMU_ADDR
				+ CLUST_TIMER_BASE_ENBL_OFFSET);

	cltbenr_val = mmio_read_32((uintptr_t)cltbenr);

	cltbenr_val = cltbenr_val
			| (1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

	mmio_write_32((uintptr_t)cltbenr, cltbenr_val);

	VERBOSE("Enable cluster time base\n");
}

/*
 * Enable core timebase.  In certain Layerscape SoCs, the clock for each core's
 * has an enable bit in the PMU Physical Core Time Base Enable
 * Register (PCTBENR), which allows the watchdog to operate.
 */

void enable_core_tb(void)
{
	uint32_t *pctbenr = (uint32_t *) (NXP_PMU_ADDR + CORE_TIMEBASE_ENBL_OFFSET);

	mmio_write_32((uintptr_t)pctbenr, 0xff);
}
