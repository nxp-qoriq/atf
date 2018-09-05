/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <platform_def.h>
#include <arch.h>
#include <plat_common.h>

/*******************************************************************************
 * This function validates an MPIDR by checking whether it falls within the
 * acceptable bounds. An error code (-1) is returned if an incorrect mpidr
 * is passed.
 ******************************************************************************/
int ls_check_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;
	uint64_t valid_mask;

	valid_mask = ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK);
	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	mpidr &= MPIDR_AFFINITY_MASK;
	if (mpidr & valid_mask)
		return -1;

	if (cluster_id >= NUMBER_OF_CLUSTERS)
		return -1;

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= plat_ls_get_cluster_core_count(mpidr))
		return -1;

	return 0;
}
