/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author: Tang Yuantian <andy.tang@nxp.com>
 */

#include <assert.h>
#include <console.h>
#include <io.h>
#include <mmio.h>
#include <mmu_def.h>
#include <plat_common.h>
#include <uart_16550.h>

#define LSCH_BASE_MASK		0xFFFF0000
#define LSCH_OFFSET_MASK	0x0000FFFF
#define	LSCH_OFFSET_DEVDISR1	0x70
#define	LSCH_OFFSET_DEVDISR7	0x88

/*******************************************************************************
 * Disabling the corresponding IP's clock
 * x1: 1: disable, 0: enable
 * x2: Endianness, 0: big endian, 1: little endian
 * x3: Address that needs to be written
 * x4: Bits that corresponding to the IP
 ******************************************************************************/
uint32_t ls_sip_dynamic_pm(uint64_t disable, uint64_t endian, uint64_t addr, uint64_t bits)
{
	uint32_t address = addr & LSCH_BASE_MASK;
	uint32_t offset = addr & LSCH_OFFSET_MASK;

	/* check if address belongs to DCFG */
	if (address != NXP_DCFG_ADDR)
		return -1;

	/* offset of DEVDISRx register */
	if (offset > LSCH_OFFSET_DEVDISR7 || offset < LSCH_OFFSET_DEVDISR1)
		return -2;

	if (disable) { /* disable IP */
		if (endian) /* little endian */
			setbits_le32((void *)addr, bits);
		else
			setbits_be32((void *)addr, bits);
	} else { /* enable IP */
		if (endian) /* big endian */
			clrbits_le32((void *)addr, bits);
		else
			clrbits_be32((void *)addr, bits);
	}

	INFO("SMC- Dis:%lx, Endian:%lx, Address:%lx, Val:%lx.\n", disable, endian, addr, bits);

	return 0;
}
