/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <caam.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <sfp.h>
#include <sfp_error_codes.h>

void set_sfp_wr_disable(uintptr_t nxp_sfp_addr)
{
	/*
	 * Mark SFP Write Disable and Write Disable Lock
	 * Bit to prevent write to SFP fuses like
	 * OUID's, Key Revocation fuse etc
	 */
	void *sfpcr = (void *)(nxp_sfp_addr + SFP_SFPCR_OFFSET);
	uint32_t sfpcr_val;

	sfpcr_val = sfp_read32(sfpcr);
	sfpcr_val |= (SFP_SFPCR_WD | SFP_SFPCR_WDL);
	sfp_write32(sfpcr, sfpcr_val);
}

int sfp_program_fuses(uintptr_t nxp_sfp_addr)
{
	uint32_t ingr;
	uint32_t sfp_cmd_status = 0;
	int ret = 0;

	/* Program SFP fuses from mirror registers */
	sfp_write32((void *)(nxp_sfp_addr + SFP_INGR_OFFSET),
		    SFP_INGR_PROGFB_CMD);

	/* Wait until fuse programming is successful */
	do {
		ingr = sfp_read32(nxp_sfp_addr + SFP_INGR_OFFSET);
	} while (ingr & SFP_INGR_PROGFB_CMD);

	/* Check for SFP fuse programming error */
	sfp_cmd_status = sfp_read32(nxp_sfp_addr + SFP_INGR_OFFSET)
			 & SFP_INGR_ERROR_MASK;

	if (sfp_cmd_status != 0)
		return ERROR_PROGFB_CMD;

	return ret;
}

uint32_t sfp_read_oem_uid(uint8_t oem_uid, uintptr_t nxp_sfp_addr)
{
	uint32_t val = 0;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	if (oem_uid > MAX_OEM_UID) {
		ERROR("Invalid OEM UID received.\n");
		return ERROR_OEMUID_WRITE;
	}

	val = sfp_read32(&sfp_ccsr_regs->oem_uid[oem_uid]);

	return val;
}

/*
 * return val:  0 - No update required.
 *              1 - successful update done.
 *              ERROR_OEMUID_WRITE - Invalid OEM UID
 */
uint32_t sfp_write_oem_uid(uint8_t oem_uid, uint32_t sfp_val,
			   uintptr_t nxp_sfp_addr)
{
	uint32_t val = 0;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	val = sfp_read_oem_uid(oem_uid, nxp_sfp_addr);

	if (val == ERROR_OEMUID_WRITE)
		return ERROR_OEMUID_WRITE;

	/* Counter already set. No need to do anything */
	if (val & sfp_val)
		return 0;

	val |= sfp_val;

	INFO("SFP Value is %x for setting sfp_val = %d\n", val, sfp_val);

	sfp_write32(&sfp_ccsr_regs->oem_uid[oem_uid], val);

	return 1;
}

int sfp_check_its(uintptr_t nxp_sfp_addr)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	if (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_ITS_MASK)
		return 1;
	else
		return 0;
}

int sfp_check_oem_wp(uintptr_t nxp_sfp_addr)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	if (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_WP_MASK)
		return 1;
	else
		return 0;
}

/* This function returns ospr's key_revoc values.*/
uint32_t get_key_revoc(uintptr_t nxp_sfp_addr)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	return (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_KEY_REVOC_MASK) >>
						OSPR_KEY_REVOC_SHIFT;
}
