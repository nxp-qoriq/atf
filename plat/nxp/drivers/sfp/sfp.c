/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Authors:
 *  Sumit Garg <sumit.garg@nxp.com>
 *
 */

#include <platform_def.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <io.h>
#include <fsl_sec.h>
#include <sfp.h>
#include <sfp_error_codes.h>
#include <delay_timer.h>

static uint8_t barker[] = {0x68, 0x39, 0x27, 0x81};

static int error_handler(int error_code)
{
	 /* Dump error code in SCRATCH4 register */
	INFO("Error in Fuse Provisioning: %x\n", error_code);
	gur_out32((void *)(NXP_DCFG_ADDR + DCFG_SCRATCH4_OFFSET), error_code);

	return 0;
}

/* This function set GPIO pin for raising POVDD. */
static int set_gpio_bitnum(uint8_t * gpio_base_addr, uint32_t bit_num)
{
	uint32_t val = 0;
	uint8_t *gpdir = NULL;
	uint8_t *gpdat = NULL;

	gpdir = gpio_base_addr + GPDIR_REG_OFFSET;
	gpdat = gpio_base_addr + GPDAT_REG_OFFSET;

	/*
	 * Set the corresponding bit in direstion register
	 * to configure the GPIO as output.
	 */
	val = sfp_read32(gpdir);
	val = val | bit_num;
	sfp_write32(gpdir, val);

	/* Set the corresponding bit in GPIO data register */
	val = sfp_read32(gpdat);
	val = val | bit_num;
	sfp_write32(gpdat, val);

	val = sfp_read32(gpdat);

	if (!(val & bit_num))
		return ERROR_GPIO_SET_FAIL;

       /*
	* Add delay so that Efuse gets the power when GPIO is enabled.
	*/
	mdelay(EFUSE_POWERUP_DELAY_mSec);
	return 0;
}

/* This function reset GPIO pin set for raising POVDD. */
static int reset_gpio_bitnum(uint8_t * gpio_base_addr, uint32_t bit_num)
{
	uint32_t val = 0;
	uint8_t *gpdir = NULL;
	uint8_t *gpdat = NULL;

	gpdir = gpio_base_addr + GPDIR_REG_OFFSET;
	gpdat = gpio_base_addr + GPDAT_REG_OFFSET;

	/*
	 * Reset the corresponding bit in direction and data register
	 * to configure the GPIO as input.
	 */
	val = sfp_read32(gpdat);
	val = val & ~(bit_num);
	sfp_write32(gpdat, val);

	val = sfp_read32(gpdat);


	val = sfp_read32(gpdir);
	val = val & ~(bit_num);
	sfp_write32(gpdir, val);

	val = sfp_read32(gpdat);

	if (val & bit_num)
		return ERROR_GPIO_RESET_FAIL;

	return 0;
}

/*
 * This function program Super Root Key Hash (SRKH) in fuse
 * registers.
 */
static int prog_srkh(struct fuse_hdr_t *fuse_hdr,
		     struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int i;

	 /* Check if SRKH already blown or not */
	for (i = 0; i < 8; i++)
		if (sfp_read32(&sfp_ccsr_regs->srk_hash[i]))
			return ERROR_SRKH_ALREADY_BLOWN;

	 /* Write SRKH in mirror registers */
	for (i = 0; i < 8; i++)
		sfp_write32(&sfp_ccsr_regs->srk_hash[i], fuse_hdr->srkh[i]);

	 /* Read back to check if write success */
	for (i = 0; i < 8; i++)
		if (sfp_read32(&sfp_ccsr_regs->srk_hash[i]) !=
						fuse_hdr->srkh[i])
			return ERROR_SRKH_WRITE;

	return 0;
}

/* This function program OEMUID[0-4] in fuse registers. */
static int prog_oemuid(struct fuse_hdr_t *fuse_hdr,
		       struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int i;

	for (i = 0; i < 5; i++) {
		 /* Check OEMUIDx to be blown or not */
		if ((fuse_hdr->flags >> (FLAG_OUID0_SHIFT + i)) & 0x1) {
			 /* Check if OEMUID[i] already blown or not */
			if (sfp_read32(&sfp_ccsr_regs->oem_uid[i]))
				return ERROR_OEMUID_ALREADY_BLOWN;

			 /* Write OEMUID[i] in mirror registers */
			sfp_write32(&sfp_ccsr_regs->oem_uid[i],
					fuse_hdr->oem_uid[i]);

			 /* Read back to check if write success */
			if (sfp_read32(&sfp_ccsr_regs->oem_uid[i]) !=
							fuse_hdr->oem_uid[i])
				return ERROR_OEMUID_WRITE;
		}
	}

	return 0;
}

/* This function program DCV[0-1], DRV[0-1] in fuse registers. */
static int prog_debug(struct fuse_hdr_t *fuse_hdr,
		      struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int i;

	 /* Check DCV to be blown or not */
	if ((fuse_hdr->flags >> (FLAG_DCV0_SHIFT)) & 0x3) {
		 /* Check if DCV[i] already blown or not */
		for (i = 0; i < 2; i++)
			if (sfp_read32(&sfp_ccsr_regs->dcv[i]))
				return ERROR_DCV_ALREADY_BLOWN;

		 /* Write DCV[i] in mirror registers */
		for (i = 0; i < 2; i++)
			sfp_write32(&sfp_ccsr_regs->dcv[i], fuse_hdr->dcv[i]);

		 /* Read back to check if write success */
		for (i = 0; i < 2; i++)
			if (sfp_read32(&sfp_ccsr_regs->dcv[i]) !=
							fuse_hdr->dcv[i])
				return ERROR_DCV_WRITE;
	}

	 /* Check DRV to be blown or not */
	if ((fuse_hdr->flags >> (FLAG_DRV0_SHIFT)) & 0x3) {
		 /* Check if DRV[i] already blown or not */
		for (i = 0; i < 2; i++)
			if (sfp_read32(&sfp_ccsr_regs->drv[i]))
				return ERROR_DRV_ALREADY_BLOWN;

		 /* Write DRV[i] in mirror registers */
		for (i = 0; i < 2; i++)
			sfp_write32(&sfp_ccsr_regs->drv[i], fuse_hdr->drv[i]);

		 /* Check for DRV hamming error */
		if (sfp_read32((void *)SFP_SVHESR_ADDR) & SFP_SVHESR_DRV_MASK)
			return ERROR_DRV_HAMMING_ERROR;
	}

	return 0;
}

 /*
  * Turn a 256-bit random value (32 bytes) into an OTPMK code word
  * modifying the input data array in place
  */
static void otpmk_make_code_word_256(uint8_t *otpmk, uint8_t minimal_flag)
{
	int i;
	uint8_t parity_bit;
	uint8_t code_bit;

	if (minimal_flag) {
		 /*
		  * Force bits 252, 253, 254 and 255 to 1
		  * This is because these fuses may have already been blown
		  * and the OTPMK cannot force them back to 0
		  */
		otpmk[252/8] |= (1 << (252%8));
		otpmk[253/8] |= (1 << (253%8));
		otpmk[254/8] |= (1 << (254%8));
		otpmk[255/8] |= (1 << (255%8));
	}

	 // Generate the hamming code for the code word
	parity_bit = 0;
	code_bit = 0;
	for (i = 0; i < 256; i += 1) {
		if ((otpmk[i/8] & (1 << (i%8))) != 0) {
			parity_bit ^= 1;
			code_bit   ^= i;
		}
	}

	 // Inverting otpmk[code_bit] will cause the otpmk
	 // to become a valid code word (except for overall parity)
	if (code_bit < 252) {
		otpmk[code_bit/8] ^= (1 << (code_bit % 8));
		parity_bit  ^= 1;  // account for flipping a bit changing parity
	} else {
		 // Invert two bits:  (code_bit - 4) and 4
		 // Because we invert two bits, no need to touch the parity bit
		otpmk[(code_bit - 4)/8] ^= (1 << ((code_bit - 4) % 8));
		otpmk[4/8] ^= (1 << (4 % 8));
	}

	 // Finally, adjust the overall parity of the otpmk
	otpmk[0] ^= parity_bit;  // otpmk bit 0
}

 // This function program One Time Programmable Master Key (OTPMK)
 // in fuse registers.
static int prog_otpmk(struct fuse_hdr_t *fuse_hdr,
					  struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int i;
	uint32_t otpmk_flags;
	uint32_t otpmk_random[8];

	otpmk_flags = (fuse_hdr->flags >> (FLAG_OTPMK_SHIFT)) & FLAG_OTPMK_MASK;

	switch (otpmk_flags) {
	case PROG_OTPMK_MIN:
		 // Check if OTPMK already blown or not
		for (i = 0; i < 8; i++)
			if (sfp_read32(&sfp_ccsr_regs->otpmk[i]))
				return ERROR_OTPMK_ALREADY_BLOWN;

		memset(fuse_hdr->otpmk, 0, sizeof(fuse_hdr->otpmk));

		 // Minimal OTPMK value (252-255 bits set to 1)
		fuse_hdr->otpmk[0] |= OTPMK_MIM_BITS_MASK;
		break;

	case PROG_OTPMK_RANDOM:
		 // Check if OTPMK already blown or not
		for (i = 0; i < 8; i++)
			if (sfp_read32(&sfp_ccsr_regs->otpmk[i]))
				return ERROR_OTPMK_ALREADY_BLOWN;

		if (CHECK_SEC_DISABLED != 0)
			return ERROR_OTPMK_SEC_DISABLED;
		else
			sec_init();
		 // Generate Random number using CAAM for OTPMK
		memset(otpmk_random, 0, sizeof(otpmk_random));
		get_rand_bytes_hw((uint8_t *)otpmk_random,
				   sizeof(otpmk_random));

		 // Run hamming over random no. to make OTPMK
		otpmk_make_code_word_256((uint8_t *)otpmk_random, 0);

		 // Swap OTPMK
		fuse_hdr->otpmk[0] = otpmk_random[7];
		fuse_hdr->otpmk[1] = otpmk_random[6];
		fuse_hdr->otpmk[2] = otpmk_random[5];
		fuse_hdr->otpmk[3] = otpmk_random[4];
		fuse_hdr->otpmk[4] = otpmk_random[3];
		fuse_hdr->otpmk[5] = otpmk_random[2];
		fuse_hdr->otpmk[6] = otpmk_random[1];
		fuse_hdr->otpmk[7] = otpmk_random[0];
		break;

	case PROG_OTPMK_USER:
		 // Check if OTPMK already blown or not
		for (i = 0; i < 8; i++)
			if (sfp_read32(&sfp_ccsr_regs->otpmk[i]))
				return ERROR_OTPMK_ALREADY_BLOWN;
		break;

	case PROG_OTPMK_RANDOM_MIN:
		 // Here assumption is that user is aware of minimal OTPMK
		 //already blown.

		 // Generate Random number using CAAM for OTPMK
		if (CHECK_SEC_DISABLED != 0)
			return ERROR_OTPMK_SEC_DISABLED;
		else
			sec_init();

		memset(otpmk_random, 0, sizeof(otpmk_random));
		get_rand_bytes_hw((uint8_t *)otpmk_random,
				  sizeof(otpmk_random));

		 // Run hamming over random no. to make OTPMK
		otpmk_make_code_word_256((uint8_t *)otpmk_random, 1);

		 // Swap OTPMK
		fuse_hdr->otpmk[0] = otpmk_random[7];
		fuse_hdr->otpmk[1] = otpmk_random[6];
		fuse_hdr->otpmk[2] = otpmk_random[5];
		fuse_hdr->otpmk[3] = otpmk_random[4];
		fuse_hdr->otpmk[4] = otpmk_random[3];
		fuse_hdr->otpmk[5] = otpmk_random[2];
		fuse_hdr->otpmk[6] = otpmk_random[1];
		fuse_hdr->otpmk[7] = otpmk_random[0];
		break;

	case PROG_OTPMK_USER_MIN:
		 /*
		  * Here assumption is that user is aware of minimal OTPMK
		  * already blown. Check if minimal bits are set in user
		  * supplied OTPMK.
		  */
		if ((fuse_hdr->otpmk[0] & OTPMK_MIM_BITS_MASK) !=
							OTPMK_MIM_BITS_MASK)
			return ERROR_OTPMK_USER_MIN;
		break;

	default:
		return 0;
	}

	 // Write OTPMK in mirror registers
	for (i = 0; i < 8; i++)
		sfp_write32(&sfp_ccsr_regs->otpmk[i], fuse_hdr->otpmk[i]);

	 // Check for DRV hamming error
	if (sfp_read32((void *)SFP_SVHESR_ADDR) & SFP_SVHESR_OTPMK_MASK)
		return ERROR_OTPMK_HAMMING_ERROR;

	return 0;
}

 // This function program OSPR1 in fuse registers.
static int prog_ospr1(struct fuse_hdr_t *fuse_hdr,
		      struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	uint32_t ospr1 = 0;

	ospr1 = sfp_read32(&sfp_ccsr_regs->ospr1);

#ifdef NXP_SFP_VER_3_4
	if ((fuse_hdr->flags >> FLAG_MC_SHIFT) & 0x1) {
		 // Check if MC already blown or not
		if (ospr1 & OSPR1_MC_MASK)
			return ERROR_OSPR1_ALREADY_BLOWN;

		ospr1 = ospr1 | (fuse_hdr->ospr1 & OSPR1_MC_MASK);
	}
#endif

	if ((fuse_hdr->flags >> FLAG_DBG_LVL_SHIFT) & 0x1) {
		 // Check if DBG LVL already blown or not
		if (ospr1 & OSPR1_DBG_LVL_MASK)
			return ERROR_OSPR1_ALREADY_BLOWN;

		ospr1 = ospr1 | (fuse_hdr->ospr1 & OSPR1_DBG_LVL_MASK);
	}

	 // Write OSPR1 in corresponding mirror register
	sfp_write32(&sfp_ccsr_regs->ospr1, ospr1);

	 // Read back to check if write success
	if (sfp_read32(&sfp_ccsr_regs->ospr1) != ospr1)
		return ERROR_OSPR1_WRITE;

	return 0;
}

 // This function program SYSCFG in fuse
 // registers.
static int prog_syscfg(struct fuse_hdr_t *fuse_hdr,
		       struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	 // Check if SYSCFG already blown or not
	if (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR0_SC_MASK)
		return ERROR_SC_ALREADY_BLOWN;

	 // Write SYSCFG in OSPR0 mirror register
	sfp_write32(&sfp_ccsr_regs->ospr, (fuse_hdr->sc & OSPR0_SC_MASK));

	 // Read back to check if write success
	if ((sfp_read32(&sfp_ccsr_regs->ospr) & OSPR0_SC_MASK) !=
		(fuse_hdr->sc & OSPR0_SC_MASK))
		return ERROR_SC_WRITE;

	return 0;
}

 // This function does fuse provisioning.
int provision_fuses(unsigned long long fuse_scr_addr)
{
	struct fuse_hdr_t *fuse_hdr = NULL;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)SFP_FUSE_REGS_ADDR;
	uint32_t sfp_cmd_status = 0;
	uint32_t ingr;
	uint32_t povdd_gpio_val = 0;
	uint32_t gpio_num = 0;
	uint32_t bit_num = 0;
	uint8_t *gpio = NULL;
	int ret = 0;

	fuse_hdr = (struct fuse_hdr_t *)fuse_scr_addr;

	 /* Check barker code */
	if (memcmp(fuse_hdr->barker, barker, sizeof(barker)))
		return error_handler(ERROR_FUSE_BARKER);

	 /* Check if GPIO pin to be set for POVDD */
	if ((fuse_hdr->flags >> FLAG_POVDD_SHIFT) & 0x1) {
		/*
		 * Subtract 1 from fuse_hdr povdd_gpio value as
		 * for 0x1 value, bit 0 is to be set
		 * for 0x20 value i.e 32, bit 31 i.e. 0x1f is to be set.
		 * 0x1f - 0x00 : GPIO_1
		 * 0x3f - 0x20 : GPIO_2
		 * 0x5f - 0x40 : GPIO_3
		 * 0x7f - 0x60 : GPIO_4
		 */
		povdd_gpio_val = (fuse_hdr->povdd_gpio - 1) & GPIO_SEL_MASK;

		/* Right shift by 5 to divide by 32 */
		gpio_num = povdd_gpio_val >> 5;
		bit_num = 1 << (31 - (povdd_gpio_val & GPIO_BIT_MASK));

		switch (gpio_num) {
			case 0:
				gpio = (uint8_t *)NXP_GPIO1_ADDR;
				break;
			case 1:
				gpio = (uint8_t *)NXP_GPIO2_ADDR;
				break;
			case 2:
				gpio = (uint8_t *)NXP_GPIO3_ADDR;
				break;
			case 3:
				gpio = (uint8_t *)NXP_GPIO4_ADDR;
				break;
			default:
				ret = ERROR_POVDD_GPIO_FAIL;
				return error_handler(ret);
		}

		ret = set_gpio_bitnum(gpio, bit_num);
		if (ret != 0)
			return error_handler(ret);
	}

	/*
	 * Check for Write Protect (WP) fuse. If blown then do
	 *  no fuse provisioning.
	 */
	if (sfp_read32(&sfp_ccsr_regs->ospr) & 0x1)
		return 0;

	 /* Check if SRKH to be blown or not */
	if ((fuse_hdr->flags >> FLAG_SRKH_SHIFT) & 0x1) {
		INFO("Fuse: Program SRKH\n");
		ret = prog_srkh(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0)
			return error_handler(ret);
	}

	 /* Check if OEMUID to be blown or not */
	if ((fuse_hdr->flags >> FLAG_OUID0_SHIFT) & FLAG_OUID_MASK) {
		INFO("Fuse: Program OEMUIDs\n");
		ret = prog_oemuid(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0)
			return error_handler(ret);
	}

	 /* Check if Debug values to be blown or not */
	if ((fuse_hdr->flags >> FLAG_DCV0_SHIFT) & FLAG_DEBUG_MASK) {
		INFO("Fuse: Program Debug values\n");
		ret = prog_debug(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0)
			return error_handler(ret);
	}

	 /* Check if OTPMK values to be blown or not */
	if (((fuse_hdr->flags >> FLAG_OTPMK_SHIFT) & PROG_NO_OTPMK) !=
		PROG_NO_OTPMK) {
		INFO("Fuse: Program OTPMK\n");
		ret = prog_otpmk(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0)
			return error_handler(ret);
	}


	 /* Check if MC or DBG LVL to be blown or not */
	if (((fuse_hdr->flags >> FLAG_MC_SHIFT) & 0x1) ||
		((fuse_hdr->flags >> FLAG_DBG_LVL_SHIFT) & 0x1)) {
		INFO("Fuse: Program OSPR1\n");
		ret = prog_ospr1(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0)
			return error_handler(ret);
	}

	 /* Check if SYSCFG to be blown or not */
	if ((fuse_hdr->flags >> FLAG_SYSCFG_SHIFT) & 0x1) {
		INFO("Fuse: Program SYSCFG\n");
		ret = prog_syscfg(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0)
			return error_handler(ret);
	}
	 /* Program SFP fuses from mirror registers */
	sfp_write32((void *)SFP_INGR_ADDR, SFP_INGR_PROGFB_CMD);

	 /* Wait until fuse programming is successful */
	do {
		ingr = sfp_read32(SFP_INGR_ADDR);
	} while (ingr & SFP_INGR_PROGFB_CMD);

	 /* Check for SFP fuse programming error */
	sfp_cmd_status = sfp_read32(SFP_INGR_ADDR) & SFP_INGR_ERROR_MASK;
	if (sfp_cmd_status != 0)
		return error_handler(ERROR_PROGFB_CMD);

	/* Reset the gpio pin set to enable povdd */
	if ((fuse_hdr->flags >> FLAG_POVDD_SHIFT) & 0x1) {
		ret = reset_gpio_bitnum(gpio, bit_num);
		if (ret != 0)
			return error_handler(ret);
	}

	return 0;
}

int sfp_check_its(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)SFP_FUSE_REGS_ADDR;

	if (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_ITS_MASK)
		return 1;
	else
		return 0;
}

int sfp_check_oem_wp(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)SFP_FUSE_REGS_ADDR;

	if (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_WP_MASK)
		return 1;
	else
		return 0;
}
