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
#include <firmware_image_package.h>
#include <io_driver.h>
#include <io_fip.h>
#include <io_memmap.h>
#include <io_storage.h>
#include <io_block.h>
#include <plat_common.h>
#include <platform.h>
#include <string.h>
#include <utils.h>

/* TBD - Move these defined to the platform_def.h file.
 * Keeping them for reference here
 */
#define PLAT_DDR_FIP_OFFSET	0x800000
#define PLAT_DDR_FIP_MAX_SIZE	0x32000

extern uintptr_t backend_dev_handle;

static uint32_t ddr_fip;

static uintptr_t ddr_fip_dev_handle;

static io_block_spec_t ddr_fip_block_spec = {
	.offset = PLAT_DDR_FIP_OFFSET,
	.length = PLAT_DDR_FIP_MAX_SIZE
};

static const io_uuid_spec_t ddr_imem_udimm_1d_uuid_spec = {
	.uuid = UUID_DDR_IMEM_UDIMM_1D,
};

static const io_uuid_spec_t ddr_imem_udimm_2d_uuid_spec = {
	.uuid = UUID_DDR_IMEM_UDIMM_2D,
};

static const io_uuid_spec_t ddr_dmem_udimm_1d_uuid_spec = {
	.uuid = UUID_DDR_DMEM_UDIMM_1D,
};

static const io_uuid_spec_t ddr_dmem_udimm_2d_uuid_spec = {
	.uuid = UUID_DDR_DMEM_UDIMM_2D,
};

static const io_uuid_spec_t ddr_imem_rdimm_1d_uuid_spec = {
	.uuid = UUID_DDR_IMEM_RDIMM_1D,
};

static const io_uuid_spec_t ddr_imem_rdimm_2d_uuid_spec = {
	.uuid = UUID_DDR_IMEM_RDIMM_2D,
};

static const io_uuid_spec_t ddr_dmem_rdimm_1d_uuid_spec = {
	.uuid = UUID_DDR_DMEM_RDIMM_1D,
};

static const io_uuid_spec_t ddr_dmem_rdimm_2d_uuid_spec = {
	.uuid = UUID_DDR_DMEM_RDIMM_2D,
};

static int open_ddr_fip(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy ddr_policies[] = {
	[DDR_FIP_IMAGE_ID] = {
		&backend_dev_handle,
		(uintptr_t)&ddr_fip_block_spec,
		open_backend
	},
	[DDR_IMEM_UDIMM_1D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_imem_udimm_1d_uuid_spec,
		open_ddr_fip
	},
	[DDR_IMEM_UDIMM_2D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_imem_udimm_2d_uuid_spec,
		open_ddr_fip
	},
	[DDR_DMEM_UDIMM_1D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_dmem_udimm_1d_uuid_spec,
		open_ddr_fip
	},
	[DDR_DMEM_UDIMM_2D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_dmem_udimm_2d_uuid_spec,
		open_ddr_fip
	},
	[DDR_IMEM_RDIMM_1D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_imem_rdimm_1d_uuid_spec,
		open_ddr_fip
	},
	[DDR_IMEM_RDIMM_2D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_imem_rdimm_2d_uuid_spec,
		open_ddr_fip
	},
	[DDR_DMEM_RDIMM_1D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_dmem_rdimm_1d_uuid_spec,
		open_ddr_fip
	},
	[DDR_DMEM_RDIMM_2D_IMAGE_ID] = {
		&ddr_fip_dev_handle,
		(uintptr_t)&ddr_dmem_rdimm_2d_uuid_spec,
		open_ddr_fip
	},
};

static int open_ddr_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(ddr_fip_dev_handle, (uintptr_t)DDR_FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(ddr_fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

/* The image can be one of the DDR PHY images, which can be sleected via DDR
 * policies
 */
int plat_get_alt_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(ddr_policies));

	policy = &ddr_policies[image_id];
	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}
	return result;
}

int ddr_fip_setup(io_dev_connector_t *fip_dev_con, unsigned int boot_dev)
{
	int io_result;
	size_t ddr_fip_offset = PLAT_DDR_FIP_OFFSET;

	/* Open connections to ddr fip and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)&ddr_fip,
				&ddr_fip_dev_handle);
	assert(io_result == 0);

	switch (boot_dev) {
#if QSPI_BOOT
	case BOOT_DEVICE_QSPI:
		ddr_fip_offset += NXP_QSPI_FLASH_ADDR;
		break;
#endif
#if NOR_BOOT
	case BOOT_DEVICE_IFC_NOR:
		ddr_fip_offset += NXP_NOR_FLASH_ADDR;
		break;
#endif
#if FLEXSPI_NOR_BOOT
	case BOOT_DEVICE_FLEXSPI_NOR:
		ddr_fip_offset += NXP_FLEXSPI_FLASH_ADDR;
		break;
#endif
	default:
		break;
	}

	ddr_fip_block_spec.offset = ddr_fip_offset;

	return io_result;
}
