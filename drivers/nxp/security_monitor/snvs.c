/*
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <snvs.h>

uint32_t get_snvs_state(uintptr_t nxp_snvs_addr)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (nxp_snvs_addr);

	return (snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST);
}

void transition_snvs_non_secure(uintptr_t nxp_snvs_addr)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (nxp_snvs_addr);
	uint32_t sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
	uint32_t fetch_cnt = 16;
	uint32_t val;

	switch (sts) {
		/* If initial state is check or Non-Secure, then
		 * set the Software Security Violation Bit and
		 * transition to Non-Secure State.
		 */
	case HPSTS_CHECK_SSM_ST:
		val = snvs_read32(&snvs->hp_com) | HPCOM_SW_SV;
		snvs_write32(&snvs->hp_com, val);
		/* polling loop till SNVS is in Non Secure state */
		do {
			sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
		} while ((sts != HPSTS_NON_SECURE_SSM_ST) && --fetch_cnt);
		break;

		/* If initial state is Trusted, Secure or Soft-Fail, then
		 * first set the Software Security Violation Bit and
		 * transition to Soft-Fail State.
		 */
	case HPSTS_TRUST_SSM_ST:
	case HPSTS_SECURE_SSM_ST:
	case HPSTS_SOFT_FAIL_SSM_ST:
		val = snvs_read32(&snvs->hp_com) | HPCOM_SW_SV;
		snvs_write32(&snvs->hp_com, val);
		/* polling loop till SNVS is in Soft-Fail state */
		do {
			sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
		} while ((sts != HPSTS_SOFT_FAIL_SSM_ST) && --fetch_cnt);

		/* If SSM Soft Fail to Non-Secure State Transition
		 * Disable is not set, then set SSM_ST bit and
		 * transition to Non-Secure State.
		 */
		if ((snvs_read32(&snvs->hp_com) & HPCOM_SSM_SFNS_DIS) == 0) {
			val = snvs_read32(&snvs->hp_com) | HPCOM_SSM_ST;
			snvs_write32(&snvs->hp_com, val);
			fetch_cnt = 16;
			/* polling loop till SNVS is in Non Secure*/
			do {
				sts = snvs_read32(&snvs->hp_stat) &
					HPSTS_MASK_SSM_ST;
			} while ((sts != HPSTS_NON_SECURE_SSM_ST) &&
								--fetch_cnt);
		}
		break;
	default:
		break;
	}
}

void transition_snvs_soft_fail(uintptr_t nxp_snvs_addr)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (nxp_snvs_addr);
	uint32_t sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
	uint32_t val = snvs_read32(&snvs->hp_com) | HPCOM_SW_FSV;
	uint32_t fetch_cnt = 16;

	snvs_write32(&snvs->hp_com, val);
	/* polling loop till SNVS is in SOFT-FAIL state */
	do {
		sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
	} while ((sts != HPSTS_SOFT_FAIL_SSM_ST && --fetch_cnt));
}

uint32_t transition_snvs_trusted(uintptr_t nxp_snvs_addr)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (nxp_snvs_addr);
	uint32_t sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
	uint32_t val;
	uint32_t fetch_cnt = 16;

	switch (sts) {
		/* If initial state is check, set the SSM_ST bit to
		 * change the state to trusted.
		 */
	case HPSTS_CHECK_SSM_ST:
		val = snvs_read32(&snvs->hp_com) | HPCOM_SSM_ST;
		snvs_write32(&snvs->hp_com, val);
		/* polling loop till SNVS is in Trusted state */
		do {
			sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
		} while ((sts != HPSTS_TRUST_SSM_ST) && --fetch_cnt);
		break;
		/* If SSM Secure to Trusted State Transition Disable
		 * is not set, then set SSM_ST bit and
		 * transition to Trusted State.
		 */
	case HPSTS_SECURE_SSM_ST:
		if ((snvs_read32(&snvs->hp_com) & HPCOM_SSM_ST_DIS) == 0) {
			val = snvs_read32(&snvs->hp_com) | HPCOM_SSM_ST;
			snvs_write32(&snvs->hp_com, val);
			/* polling loop till SNVS is in Trusted*/
			do {
				sts = snvs_read32(&snvs->hp_stat) &
					HPSTS_MASK_SSM_ST;
			} while ((sts != HPSTS_TRUST_SSM_ST) && --fetch_cnt);
		}
		break;
		/* If initial state is Soft-Fail or Non-Secure, then
		 * transition to Trusted is not Possible.
		 */
	default:
		break;
	}

	return sts;
}

uint32_t transition_snvs_secure(uintptr_t nxp_snvs_addr)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (nxp_snvs_addr);
	uint32_t sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
	uint32_t val;
	uint32_t fetch_cnt = 16;

	if (sts == HPSTS_SECURE_SSM_ST)
		return sts;

	if (sts != HPSTS_TRUST_SSM_ST) {
		sts = transition_snvs_trusted(nxp_snvs_addr);
		if (sts != HPSTS_TRUST_SSM_ST)
			return sts;
	}

	/* Set the SSM_ST bit to change the state from Trusted to SECURE */
	val = snvs_read32(&snvs->hp_com) | HPCOM_SSM_ST;
	snvs_write32(&snvs->hp_com, val);

	/* polling loop till SNVS is in Secure State state */
	do {
		sts = snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST;
	} while ((sts != HPSTS_SECURE_SSM_ST) && --fetch_cnt);

	return sts;
}

void snvs_write_lp_gpr(uintptr_t nxp_snvs_addr, uint32_t offset,
			   uint32_t bit_pos, bool flag_val)
{
	if (flag_val  == true)
		snvs_write32(nxp_snvs_addr + offset,
			     (snvs_read32(nxp_snvs_addr + offset))
			     | (1 << bit_pos));
	else
		snvs_write32(nxp_snvs_addr + offset,
			     (snvs_read32(nxp_snvs_addr + offset))
			     & ~(1 << bit_pos));
}

uint32_t snvs_read_lp_gpr(uintptr_t nxp_snvs_addr,
			  uint32_t offset, uint32_t bit_pos)
{
	return ((snvs_read32(nxp_snvs_addr + offset)) & (1 << bit_pos));
}

void snvs_disable_zeroize_lp_gpr(uintptr_t nxp_snvs_addr)
{
	snvs_write_lp_gpr(nxp_snvs_addr,
			  NXP_LPCR_OFFSET,
			  NXP_GPR_Z_DIS_BIT,
			  true);
}
#ifdef NXP_WARM_BOOT
uint32_t snvs_warm_boot_status(uintptr_t nxp_snvs_addr)
{
	return snvs_read_lp_gpr(nxp_snvs_addr,
				NXP_LP_GPR0_OFFSET,
				NXP_WARM_RST_FLAG_BIT);
}

void snvs_clr_warm_boot_flag(uintptr_t nxp_snvs_addr)
{
	snvs_write_lp_gpr(nxp_snvs_addr,
			  NXP_LP_GPR0_OFFSET,
			  NXP_WARM_RST_FLAG_BIT,
			  false);
}
#endif
