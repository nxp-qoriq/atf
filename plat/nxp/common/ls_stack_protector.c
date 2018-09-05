/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta <pankaj.gupta@nxp.com>
 */

#include <arch_helpers.h>
#include <platform.h>
#include <stdint.h>

#define RANDOM_CANARY_VALUE ((u_register_t) 3288484550995823360ULL)

u_register_t plat_get_stack_protector_canary(void)
{
	/*
	 * TBD: Generate Random Number from NXP CAAM Block.
	 */
	return RANDOM_CANARY_VALUE ^ read_cntpct_el0();
}
