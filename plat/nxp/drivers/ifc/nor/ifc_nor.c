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

int nor_init(void)
{
	/* Add FTIM initializations */
	return 0;
}

int ifc_nor_io_setup(void)
{
	nor_init();

	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(NXP_NOR_FLASH_ADDR, NXP_NOR_FLASH_ADDR,
			NXP_NOR_FLASH_SIZE, MT_MEMORY | MT_RW);

	return plat_io_memmap_setup(NXP_NOR_FLASH_ADDR + PLAT_FIP_OFFSET);
}
