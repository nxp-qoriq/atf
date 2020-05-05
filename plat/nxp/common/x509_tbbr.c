/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Authors: Ruchika Gupta <ruchika.gupta@nxp.com>
 *
 */

#include <plat/common/platform.h>

extern char nxp_rotpk_hash[], nxp_rotpk_hash_end[];

int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	*key_ptr = nxp_rotpk_hash;
	*key_len = nxp_rotpk_hash_end - nxp_rotpk_hash;
	*flags = ROTPK_IS_HASH;

	return 0;
}

int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	/*
	 * No support for non-volatile counter.  Update the ROT key to protect
	 * the system against rollback.
	 */
	*nv_ctr = 0;

	return 0;
}

int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	return 0;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
