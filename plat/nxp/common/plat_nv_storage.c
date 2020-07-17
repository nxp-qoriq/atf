#include <errno.h>
#include <assert.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common/debug.h>
#ifndef NXP_COINED_BB
#include <fspi_api.h>
#include <fspi.h>
#include <flash_info.h>
#else
#include <snvs.h>
#endif
#include <lib/mmio.h>
#include <plat_nv_storage.h>

/*This structure will be a static structure and
 * will be populated as first step of BL2 booting-up.
 * fspi_strorage.c . To be located in the fspi driver folder.
 */

static nv_app_data_t nv_app_data;

uint8_t read_nv_app_data(uintptr_t nv_base_addr)
{
	int ret = 0;

#ifdef NXP_COINED_BB
	uint8_t *nv_app_data_array = (uint8_t *) &nv_app_data;
	uint8_t offset = 0;

	ret = snvs_read_app_data(NXP_SNVS_ADDR);
	do {
		nv_app_data_array[offset] = snvs_read_app_data_bit(NXP_SNVS_ADDR, offset);
		offset++;

	} while (offset < APP_DATA_MAX_OFFSET);
	snvs_clear_app_data(NXP_SNVS_ADDR);
#else
#ifndef FLEXSPI_NOR_BOOT
	ret = fspi_init(NXP_FLEXSPI_ADDR);

	if (ret) {
		ERROR("Failed to initialized driver flexspi-nor.\n");
		ERROR("exiting warm-reset request.\n");
		return -ENODEV;
	}
#endif
	xspi_read(nv_base_addr,
		  (uint32_t *)&nv_app_data, sizeof(nv_app_data_t));
	xspi_sector_erase((uint32_t) nv_base_addr,
				F_SECTOR_ERASE_SZ);
#endif
	return ret;
}

uint8_t wr_nv_app_data(uintptr_t nv_base_addr,
			int data_offset,
			uint8_t *data,
			int data_size)
{
	int ret = 0;
#ifdef NXP_COINED_BB
#if !TRUSTED_BOARD_BOOT
	snvs_disable_zeroize_lp_gpr(NXP_SNVS_ADDR);
#endif
	/* In case LP SecMon General purpose register,
	 * only 1 bit flags can be saved.
	 */
	if ((data_size > 1) || (*data != DEFAULT_SET_VALUE)) {
		ERROR("Only binary value is allowed to be written.\n");
		ERROR("Use flash instead of SNVS GPR as NV location.\n");
		return -ENODEV;
	}
	snvs_write_app_data_bit(NXP_SNVS_ADDR,
			  data_offset);
#else
	uint8_t read_val[sizeof(nv_app_data_t)];
	uint8_t ready_to_write_val[sizeof(nv_app_data_t)];

	assert((nv_base_addr + data_offset + data_size) > (nv_base_addr + F_SECTOR_ERASE_SZ));

#if (!defined(FLEXSPI_NOR_BOOT) && defined(IMAGE_BL2)) || defined(IMAGE_BL31)
	ret = fspi_init(NXP_FLEXSPI_ADDR);

	if (ret) {
		ERROR("Failed to initialized driver flexspi-nor.\n");
		ERROR("exiting warm-reset request.\n");
		return -ENODEV;
	}
#endif

	ret = xspi_read(nv_base_addr + data_offset, (uint32_t *)read_val, data_size);

	memset(ready_to_write_val, READY_TO_WRITE_VALUE, ARRAY_SIZE(ready_to_write_val));

	if (memcmp(read_val, ready_to_write_val, data_size) == 0)
		xspi_write(nv_base_addr + data_offset, data, data_size);
#endif

	return ret;
}

const nv_app_data_t *get_nv_data(void)
{
	return (const nv_app_data_t *) &nv_app_data;
}
