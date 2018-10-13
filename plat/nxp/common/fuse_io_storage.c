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

/* Can be overridden from platform_def.h file.
 */
#ifndef PLAT_FUSE_FIP_OFFSET
#define PLAT_FUSE_FIP_OFFSET	0x880000
#endif
#ifndef PLAT_FUSE_FIP_MAX_SIZE
#define PLAT_FUSE_FIP_MAX_SIZE	0x80000
#endif

extern uintptr_t backend_dev_handle;

static uint32_t fuse_fip;

static uintptr_t fuse_fip_dev_handle;

static io_block_spec_t fuse_fip_block_spec = {
	.offset = PLAT_FUSE_FIP_OFFSET,
	.length = PLAT_FUSE_FIP_MAX_SIZE
};

static const io_uuid_spec_t fuse_prov_uuid_spec = {
	.uuid = UUID_FUSE_PROV,
};

static const io_uuid_spec_t fuse_up_uuid_spec = {
	.uuid = UUID_FUSE_UP,
};

static int open_fuse_fip(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy fuse_policies[] = {
	[FUSE_FIP_IMAGE_ID] = {
		&backend_dev_handle,
		(uintptr_t)&fuse_fip_block_spec,
		open_backend
	},
	[FUSE_PROV_IMAGE_ID] = {
		&fuse_fip_dev_handle,
		(uintptr_t)&fuse_prov_uuid_spec,
		open_fuse_fip
	},
	[FUSE_UP_IMAGE_ID] = {
		&fuse_fip_dev_handle,
		(uintptr_t)&fuse_up_uuid_spec,
		open_fuse_fip
	}
};

static int open_fuse_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fuse_fip_dev_handle, (uintptr_t)FUSE_FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fuse_fip_dev_handle, spec, &local_image_handle);
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
int plat_get_fuse_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(fuse_policies));

	policy = &fuse_policies[image_id];
	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}
	return result;
}

int fuse_fip_setup(io_dev_connector_t *fip_dev_con, unsigned int boot_dev)
{
	int io_result;
	size_t fuse_fip_offset = PLAT_FUSE_FIP_OFFSET;

	/* Open connections to fuse fip and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)&fuse_fip,
				&fuse_fip_dev_handle);

	assert(io_result == 0);

	switch (boot_dev) {
#if QSPI_BOOT
	case BOOT_DEVICE_QSPI:
		fuse_fip_offset += NXP_QSPI_FLASH_ADDR;
		break;
#endif
#if NOR_BOOT
	case BOOT_DEVICE_IFC_NOR:
		fuse_fip_offset += NXP_NOR_FLASH_ADDR;
		break;
#endif
#if FLEXSPI_NOR_BOOT
	case BOOT_DEVICE_FLEXSPI_NOR:
		fuse_fip_offset += NXP_FLEXSPI_FLASH_ADDR;
		break;
#endif
	default:
		break;
	}

	fuse_fip_block_spec.offset = fuse_fip_offset;

	return io_result;
}
