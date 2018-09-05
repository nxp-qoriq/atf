/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <platform_def.h>
#include <arch.h>
#include <cci.h>
#include <plat_arm.h>
#include <utils.h>

static const int cci_map[] = {
	NXP_CCI_CLUSTER0_SL_IFACE_IX,
};

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way ARM CCI driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_interconnect_init
#pragma weak plat_arm_interconnect_enter_coherency
#pragma weak plat_arm_interconnect_exit_coherency


/******************************************************************************
 * Helper function to initialize ARM CCI driver.
 *****************************************************************************/
void plat_ls_interconnect_init(void)
{
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));
}

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_ls_interconnect_enter_coherency(void)
{
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_ls_interconnect_exit_coherency(void)
{
	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
