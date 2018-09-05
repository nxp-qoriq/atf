/*
 * Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Pankaj Gupta  <pankaj.gupta@nxp.com>
 */

#ifndef _TZC380_H_
#define _TZC380_H_

#define TZC380_BUILD_CONFIG_OFF        0x000
#define TZC380_ACTION_OFF        0x004
#define TZC380_BUILD_CONFIG_AW_SHIFT    8
#define TZC380_BUILD_CONFIG_AW_MASK    0x3f
#define TZC380_BUILD_CONFIG_NR_SHIFT    0
#define TZC380_BUILD_CONFIG_NR_MASK    0xf

#define TZASC_CONFIGURATION_REG	0x000
#define TZASC_SECURITY_INV_REG	0x034
#define TZASC_SECURITY_INV_EN	0x1
#define TZASC_REGIONS_REG	0x100

/* As region address should address atleast 32KB memory. */
#define TZASC_REGION_LOWADDR_MASK	0xFFFF8000
#define TZASC_REGION_LOWADDR_OFFSET	0x0
#define TZASC_REGION_HIGHADDR_OFFSET	0x4
#define TZASC_REGION_ATTR_OFFSET	0x8
#define TZASC_REGION_ENABLED		1
#define TZASC_REGION_DISABLED		0
#define TZASC_REGION_SIZE_32KB		0xE
#define TZASC_REGION_SIZE_64KB		0xF
#define TZASC_REGION_SIZE_128KB		0x10
#define TZASC_REGION_SIZE_256KB		0x11
#define TZASC_REGION_SIZE_512KB		0x12
#define TZASC_REGION_SIZE_1MB		0x13
#define TZASC_REGION_SIZE_2MB		0x14
#define TZASC_REGION_SIZE_4MB		0x15
#define TZASC_REGION_SIZE_8MB		0x16
#define TZASC_REGION_SIZE_16MB		0x17
#define TZASC_REGION_SIZE_32MB		0x18
#define TZASC_REGION_SIZE_64MB		0x19
#define TZASC_REGION_SIZE_128MB		0x1A
#define TZASC_REGION_SIZE_256MB		0x1B
#define TZASC_REGION_SIZE_512MB		0x1C
#define TZASC_REGION_SIZE_1GB		0x1D
#define TZASC_REGION_SIZE_2GB		0x1E
#define TZASC_REGION_SIZE_4GB		0x1F
#define TZASC_REGION_SIZE_8GB		0x20
#define TZASC_REGION_SIZE_16GB		0x21
#define TZASC_REGION_SIZE_32GB		0x22
#define TZASC_REGION_SECURITY_SR	(1 << 3)
#define TZASC_REGION_SECURITY_SW	(1 << 2)
#define TZASC_REGION_SECURITY_SRW	(TZASC_REGION_SECURITY_SR| \
					TZASC_REGION_SECURITY_SW)
#define TZASC_REGION_SECURITY_NSR	(1 << 1)
#define TZASC_REGION_SECURITY_NSW	1
#define TZASC_REGION_SECURITY_NSRW	(TZASC_REGION_SECURITY_NSR| \
					TZASC_REGION_SECURITY_NSW)

struct tzc380_instance {
	unsigned int base;
	uint8_t addr_width;
	uint8_t num_regions;
};

struct tzc380_reg {
	unsigned int secure;
	unsigned int enabled;
	unsigned int low_addr;
	unsigned int high_addr;
	unsigned int size;
	unsigned int sub_mask;
};

/*
 * Type of actions on an access violation:
 *  -- Memory requested is zeroed
 *  -- Raise Interrupt(INT) or/and Error Exception(ERR) ->
 *     sync external Data Abort to inform the system
 *
 *    -TZC380_ACTION_NONE-     No Interrupt   , No Exception
 *    -TZC380_ACTION_ERR -     No Interrupt   , Raise exception
 *                             sync external Data Abort
 *    -TZC380_ACTION_INT     - Raise Interrupt, No exceptio
 *    -TZC380_ACTION_ERR_INT - Raise Interrupt, Raise Exception
 */
enum tzc380_action {
    TZC380_ACTION_NONE = 0,
    TZC380_ACTION_ERR = 1,
    TZC380_ACTION_INT = 2,
    TZC380_ACTION_ERR_INT = (TZC380_ACTION_ERR | TZC380_ACTION_INT)
};

void tzc380_set_action(enum tzc380_action action);

void tzc380_init(void *tzc380_ptr, unsigned int base);

int tzc380_set_region(unsigned int tzasc_base, unsigned int region_id,
		unsigned int enabled, unsigned int low_addr,
		unsigned int high_addr, unsigned int size,
		unsigned int security, unsigned int subreg_disable_mask);
#endif /* _TZC380_H_ */
