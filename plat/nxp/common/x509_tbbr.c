/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Authors: Ruchika Gupta <ruchika.gupta@nxp.com>
 *
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/cassert.h>
#include <plat/common/platform.h>
#include <tools_share/tbbr_oid.h>
#include <sfp.h>

extern char nxp_rotpk_hash[], nxp_rotpk_hash_end[];

#pragma weak board_enable_povdd
#pragma weak board_disable_povdd

void board_enable_povdd(void)
{
}

void board_disable_povdd(void)
{
}

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
	const char *oid;
	uint32_t uid_num;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs =
				(void *)(NXP_SFP_ADDR + SFP_FUSE_REGS_OFFSET);
	uint32_t val = 0;

	assert(cookie != NULL);
	assert(nv_ctr != NULL);

	oid = (const char *)cookie;
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0)
		uid_num = 3;
	else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0)
		uid_num = 4;
	else
		return 1;

	val = sfp_read32(&sfp_ccsr_regs->oem_uid[uid_num]);
	INFO("SFP Value read is %x from UID %d\n", val, uid_num);
	if (!val)
		*nv_ctr = 0;
	else
		*nv_ctr = (32 - __builtin_clz(val));

	INFO("NV Counter value for UID %d is %d\n", uid_num, *nv_ctr);
	return 0;

}

int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	const char *oid;
	uint32_t uid_num, val = 0, sfp_val;
	uint32_t ingr, sfp_cmd_status;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs =
				(void *)(NXP_SFP_ADDR + SFP_FUSE_REGS_OFFSET);

	assert(cookie != NULL);

	/* Counter values upto 32 are supported */
	if (nv_ctr > 32)
		return 1;

	oid = (const char *)cookie;
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0)
		uid_num = 3;
	else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0)
		uid_num = 4;
	else
		return 1;

	sfp_val = (1 << (nv_ctr - 1));

	val = sfp_read32(&sfp_ccsr_regs->oem_uid[uid_num]);

	/* Counter already set. No need to do anything */
	if (val & sfp_val)
		return 0;

	val |= sfp_val;

	INFO("SFP Value is %x for setting NV CTR %d\n", val, nv_ctr);

	sfp_write32(&sfp_ccsr_regs->oem_uid[uid_num], val);

	/* Enable POVDD on board */
	board_enable_povdd();

	 /* Program SFP fuses from mirror registers */
	sfp_write32((void *)(NXP_SFP_ADDR + SFP_INGR_OFFSET),
		    SFP_INGR_PROGFB_CMD);

	 /* Wait until fuse programming is successful */
	do {
		ingr = sfp_read32(NXP_SFP_ADDR + SFP_INGR_OFFSET);
	} while (ingr & SFP_INGR_PROGFB_CMD);

	 /* Check for SFP fuse programming error */
	sfp_cmd_status = sfp_read32(NXP_SFP_ADDR + SFP_INGR_OFFSET)
			 & SFP_INGR_ERROR_MASK;
	if (sfp_cmd_status != 0)
		return 1;

	/* Disable POVDD on board */
	board_disable_povdd();

	return 0;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
