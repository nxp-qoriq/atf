/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 */

#include <platform_def.h>
#include <io.h>
#include <assert.h>
#include <debug.h>
#include <plat_common.h>
#include <mmio.h>
#include <xlat_tables_v2.h>

int qspi_io_setup(void)
{
	uint32_t qspi_mcr_val = qspi_in32(CHS_QSPI_MCR);

	/* Enable and change endianness of QSPI IP */
	qspi_out32(CHS_QSPI_MCR, (qspi_mcr_val | CHS_QSPI_64LE));

	/* Adding QSPI Memory Map in XLAT Table */
	mmap_add_region(NXP_QSPI_FLASH_ADDR, NXP_QSPI_FLASH_ADDR,
			NXP_QSPI_FLASH_SIZE, MT_MEMORY | MT_RW);

	return plat_io_memmap_setup(NXP_QSPI_FLASH_ADDR + PLAT_FIP_OFFSET);
}
