/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 */

#include <assert.h>
#include <bl_common.h>
#include <desc_image_load.h>
#include <plat_common.h>
#include <xlat_tables_v2.h>

/******************************************************************************
 * This function can be used to load DDR PHY Images
 *
 * @param [in] image_id		 Image ID to be loaded
 *
 * @param [in,out]  image_base   Location at which the image should be loaded
 *				 In case image is prepended by a CSF header,
 *				 image_base is pointer to actual image after
 *				 the header
 *
 * @param [in,out]  image_size   User should pass the maximum size of the image
 *				 possible.(Buffer size starting from image_base)
 *				 Actual size of the image loaded is returned
 *				 back.
 *****************************************************************************/
int load_ddr_phy_img(unsigned int image_id, uintptr_t *image_base,
		      uint32_t *image_size)
{
	int err = 0;

	image_desc_t ddr_phy_info = {
		.image_id = image_id,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				VERSION_2, image_info_t, 0),
#ifdef CSF_HEADER_PREPENDED
		.image_info.image_base = *image_base - CSF_HDR_SZ,
		.image_info.image_max_size = *image_size + CSF_HDR_SZ,
#else
		.image_info.image_base = *image_base,
		.image_info.image_max_size = *image_size,
#endif
	};

	/* Create MMU entry for the CSF header */
#ifdef CSF_HEADER_PREPENDED
	mmap_add_dynamic_region(ddr_phy_info.image_info.image_base,
			ddr_phy_info.image_info.image_base,
			CSF_HDR_SZ,
			MT_MEMORY | MT_RW | MT_SECURE);
#endif

	VERBOSE("BL2: Loading DDR_PHY_IMG\n");
	err = load_auth_image(image_id, &ddr_phy_info.image_info);
	if (err != 0) {
		VERBOSE("Failed to load DDR PHY IMG %d\n", image_id);
		return err;
	}

#ifdef CSF_HEADER_PREPENDED
	*image_base = ddr_phy_info.image_info.image_base + CSF_HDR_SZ;
	*image_size = ddr_phy_info.image_info.image_size - CSF_HDR_SZ;
	mmap_remove_dynamic_region(ddr_phy_info.image_info.image_base,
				   CSF_HDR_SZ);
#else
	*image_base = ddr_phy_info.image_info.image_base;
	*image_size = ddr_phy_info.image_info.image_size;
#endif

	return err;
}
