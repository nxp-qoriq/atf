/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <platform_def.h>
#include <arch.h>
#include <ccn.h>
#include <plat_arm.h>

static const unsigned char master_to_rn_id_map[] = {
	PLAT_CLUSTER_TO_CCN_ID_MAP
};

static const ccn_desc_t plat_ccn_desc = {
	.periphbase = NXP_CCN_ADDR,
	.num_masters = ARRAY_SIZE(master_to_rn_id_map),
	.master_to_rn_id_map = master_to_rn_id_map
};

CASSERT(NUMBER_OF_CLUSTERS == ARRAY_SIZE(master_to_rn_id_map),
		assert_invalid_cluster_count_for_ccn_variant);


/******************************************************************************
 * Helper function to initialize ARM CCN driver.
 *****************************************************************************/
void plat_ls_interconnect_init(void)
{
	ccn_init(&plat_ccn_desc);
}

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_ls_interconnect_enter_coherency(void)
{
	ccn_enter_snoop_dvm_domain(1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

	for (uint32_t index = 1; index < plat_ccn_desc.num_masters; index++) {
		ccn_enter_snoop_dvm_domain(1 << index);
	}
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_ls_interconnect_exit_coherency(void)
{
	ccn_exit_snoop_dvm_domain(1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
