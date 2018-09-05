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
#include <endian.h>

void enable_timer_base_to_cluster(void)
{
	uint32_t *ccsr_svr = (uint32_t *)(NXP_DCFG_ADDR + DCFG_SVR_OFFSET);
	uint32_t soc_dev_id = mmio_read_32((uintptr_t)ccsr_svr);
	uint32_t val = 0;

	if ((soc_dev_id & 0xff000000) == 0x87000000)
		val = le32toh(soc_dev_id) >> 8;
	else if ((soc_dev_id & 0xff) == 0x87)
		val = be32toh(soc_dev_id) >> 8;
	else
		ERROR("Unknown DCFG endianness: SVR = 0x%x\n", soc_dev_id);

	uint32_t *cltbenr = (uint32_t *)(NXP_PMU_ADDR
					+ CLUST_TIMER_BASE_ENBL_OFFSET);

	if (val == SVR_LX2160A) {

		uint32_t cltbenr_val = mmio_read_32((uintptr_t)cltbenr);

		cltbenr_val = cltbenr_val
				| (1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
		mmio_write_32((uintptr_t)cltbenr, cltbenr_val);

		VERBOSE("Enable cluster time base\n");
	}
}
