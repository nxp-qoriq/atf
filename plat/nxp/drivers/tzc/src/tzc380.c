/*
 * Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta  <pankaj.gupta@nxp.com>
 */

#include <platform_def.h>
#include <debug.h>
#include <mmio.h>
#include <endian.h>
#include <tzc380.h>

static uint32_t tzc380_read_build_config(unsigned int base)
{
	return mmio_read_32(base + TZC380_BUILD_CONFIG_OFF);
}

void tzc380_set_action(enum tzc380_action action)
{
	// No Handler defined either for Error or Interrupt.
	mmio_write_32(NXP_TZC_ADDR + TZC380_ACTION_OFF, action);
}

void tzc380_init(void *tzc380_ptr, unsigned int base)
{
	uint32_t tzc380_build;
	struct tzc380_instance *tzc380 = (struct tzc380_instance *) tzc380_ptr;

	tzc380->base = base;

	/* Save values we will use later. */
	tzc380_build = tzc380_read_build_config(tzc380->base);
	tzc380->addr_width  = ((tzc380_build >> TZC380_BUILD_CONFIG_AW_SHIFT) &
               TZC380_BUILD_CONFIG_AW_MASK) + 1;
	tzc380->num_regions = ((tzc380_build >> TZC380_BUILD_CONFIG_NR_SHIFT) &
               TZC380_BUILD_CONFIG_NR_MASK) + 1;
}

int tzc380_set_region(unsigned int tzasc_base, unsigned int region_id,
		unsigned int enabled, unsigned int low_addr,
		unsigned int high_addr, unsigned int size,
		unsigned int security, unsigned int subreg_disable_mask)
{
	unsigned int reg;
	unsigned int reg_base;
	unsigned int attr_value;

	reg_base = (tzasc_base + TZASC_REGIONS_REG + (region_id << 4));
#ifdef HH_DEBUG
	VERBOSE("reg_base = %x\n", reg_base);
#endif
	if (region_id == 0) {
		reg = (reg_base + TZASC_REGION_ATTR_OFFSET);
		mmio_write_32(reg, ((security & 0xF) << 28));
	} else {
		reg = reg_base + TZASC_REGION_LOWADDR_OFFSET;
		mmio_write_32((uintptr_t)reg,
				(low_addr & TZASC_REGION_LOWADDR_MASK));

		reg = reg_base + TZASC_REGION_HIGHADDR_OFFSET;
		mmio_write_32((uintptr_t)reg, high_addr);

		reg = reg_base + TZASC_REGION_ATTR_OFFSET;
		attr_value = ((security & 0xF) << 28) |
			((subreg_disable_mask & 0xFF) << 8) |
			((size & 0x3F) << 1) | (enabled & 0x1);
		mmio_write_32((uintptr_t)reg, attr_value);

	}
	return 0;
}
