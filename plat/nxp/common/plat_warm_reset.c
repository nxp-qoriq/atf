/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>

#include <common/debug.h>
#include <ddr.h>
#ifndef NXP_COINED_BB
#include <fspi_api.h>
#include <fspi.h>
#include <flash_info.h>
#else
#include <snvs.h>
#endif
#include <lib/mmio.h>

#include <plat_warm_rst.h>
#include <platform_def.h>

#if defined(IMAGE_BL2)
static void __unused clr_warm_boot_flag(void)
{
#ifndef NXP_COINED_BB
	xspi_sector_erase((uint32_t)WARM_BOOT_FLAG_BASE_ADDR,
				F_SECTOR_ERASE_SZ);
#else
	snvs_clr_warm_boot_flag(NXP_SNVS_ADDR);
#endif
}

uint32_t is_warm_boot(void)
{
	int ret = mmio_read_32(NXP_RESET_ADDR + RST_RSTRQSR1_OFFSET)
				& ~(RSTRQSR1_SWRR);

	if (ret == 0) {
		INFO("Not a SW(Warm) triggered reset.\n");
		clr_warm_boot_flag();
		return 0;
	}

#ifndef NXP_COINED_BB
	uint32_t warm_reset;

#ifndef FLEXSPI_NOR_BOOT
	ret = fspi_init(NXP_FLEXSPI_ADDR);

	if (ret) {
		ERROR("Failed to initialized driver flexspi-nor.\n");
		ERROR("exiting warm-reset request.\n");
		return -ENODEV;
	}
#endif
	xspi_read(WARM_BOOT_FLAG_BASE_ADDR,
		  (uint32_t *)&warm_reset, sizeof(warm_reset));
	ret = (warm_reset == WARM_BOOT_SUCCESS) ? 1 : 0;
#else
	ret = snvs_warm_boot_status(NXP_SNVS_ADDR);
#endif
	if (ret) {
		INFO("Warm Reset was triggered..\n");
		clr_warm_boot_flag();
	} else
		INFO("Warm Reset was not triggered..\n");

	return ret;
}

#endif

#if defined(IMAGE_BL31)
int prep_n_execute_warm_reset(void)
{
#ifdef NXP_COINED_BB
#if !TRUSTED_BOARD_BOOT
	snvs_disable_zeroize_lp_gpr(NXP_SNVS_ADDR);
#endif
#else
	int ret;
	uint32_t warm_reset = WARM_BOOT_SUCCESS;

	ret = fspi_init(NXP_FLEXSPI_ADDR);

	if (ret) {
		ERROR("Failed to initialized driver flexspi-nor.\n");
		ERROR("exiting warm-reset request.\n");
		return -ENODEV;
	}

	/* Set the warm-reset flag and store. */
	ret = xspi_sector_erase((uint32_t)WARM_BOOT_FLAG_BASE_ADDR,
				F_SECTOR_ERASE_SZ);
	if (ret)
		return -EINVAL;

#if (ERLY_WRM_RST_FLG_FLSH_UPDT)
	ret = xspi_write((uint32_t)WARM_BOOT_FLAG_BASE_ADDR,
			 &warm_reset,
			 sizeof(warm_reset));
#else
	/* Preparation for writing the Warm reset flag. */
	ret = xspi_wren((uint32_t)WARM_BOOT_FLAG_BASE_ADDR);

	/* IP Control Register0 - SF Address to be read */
	fspi_out32((NXP_FLEXSPI_ADDR + FSPI_IPCR0),
		   (uint32_t) WARM_BOOT_FLAG_BASE_ADDR);

	while (!(fspi_in32(NXP_FLEXSPI_ADDR + FSPI_INTR)
		& FSPI_INTR_IPTXWE_MASK))
		;
	/* Write TX FIFO Data Register */
	fspi_out32(NXP_FLEXSPI_ADDR + FSPI_TFDR, (uint32_t) warm_reset);

	fspi_out32(NXP_FLEXSPI_ADDR + FSPI_INTR, FSPI_INTR_IPTXWE);

	/* IP Control Register1 - SEQID_WRITE operation, Size = 1 Byte */
	fspi_out32(NXP_FLEXSPI_ADDR + FSPI_IPCR1,
		   (uint32_t)(fspiWRITE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) |
		   (uint16_t) sizeof(warm_reset));

	/* Trigger XSPI-IP-Write cmd only if:
	 *  - Putting DDR in-self refresh mode is successfully.
	 *    to complete the writing of the warm-reset flag
	 *    to flash.
	 *
	 * This code is as part of assembly.
	 */
#endif
#endif
	INFO("Doing DDR Self refresh.\n");
	_soc_sys_warm_reset();

	/* Expected behaviour is to do the power cycle */
	while (1)
		;
	return -1;
}
#endif
