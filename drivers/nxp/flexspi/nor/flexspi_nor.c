/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

int flexspi_init(void)
{
	return 0;
}

int flexspi_nor_io_setup(uintptr_t nxp_flexspi_flash_addr,
			 size_t nxp_flexspi_flash_size)
{
	int ret = 0;

	ret = flexspi_init();
	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(nxp_flexspi_flash_addr, nxp_flexspi_flash_addr,
			nxp_flexspi_flash_size, MT_MEMORY | MT_RW);

	return ret;
}
