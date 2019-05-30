/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 */

#include <platform_def.h>
#include <assert.h>
#include <debug.h>
#include <plat_common.h>
#include <mmio.h>
#include <xlat_tables_v2.h>

int flexspi_init(void)
{
	return 0;
}

int flexspi_nor_io_setup(void)
{

	flexspi_init();
	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(NXP_FLEXSPI_FLASH_ADDR, NXP_FLEXSPI_FLASH_ADDR,
			NXP_FLEXSPI_FLASH_SIZE, MT_MEMORY | MT_RW);

	return plat_io_memmap_setup(NXP_FLEXSPI_FLASH_ADDR + PLAT_FIP_OFFSET);
}
