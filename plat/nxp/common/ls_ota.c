/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Alison Wang <alison.wang@nxp.com>
 */

#include <debug.h>
#include <ls_ota.h>
#include <plat_common.h>
#include <platform_def.h>
#include <sd_mmc.h>
#include <soc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Private structure for MMC driver data */
static struct mmc mmc_drv_data;

int ota_status_check(void)
{
	struct mmc *mmc = NULL;
	struct OTA_parameter data;
	uintptr_t buffer;
	int result;
	static int lresult = 2;

	if ((lresult == 0) || (lresult == 1))
		return lresult;

	buffer = (uintptr_t)&data;
	mmc = &mmc_drv_data;
	memset(mmc, 0, sizeof(struct mmc));
	mmc->esdhc_regs = (struct esdhc_regs *)NXP_ESDHC_ADDR;

	esdhc_emmc_init(mmc, true);

	esdhc_read(mmc, OTA_OFFSET, buffer, OTA_SIZE);

	switch (data.status) {
	case OTA_STATUS_ONLINE:
		NOTICE("OTA_STATUS_ONLINE\n");
		result = 1;
		break;
	case OTA_STATUS_INSTALLED:
		NOTICE("OTA_STATUS_INSTALLED\n");
		result = 1;
		data.status = OTA_STATUS_FAILED;
		esdhc_write(mmc, buffer, OTA_OFFSET, OTA_SIZE);
		break;
	case OTA_STATUS_FAILED:
		NOTICE("OTA_STATUS_FAILED\n");
		result = 0;
		break;
	case OTA_STATUS_IDLE:
		NOTICE("OTA_STATUS_IDLE\n");
		result = 0;
		break;
	case OTA_STATUS_INSTALL:
		NOTICE("OTA_STATUS_INSTALL\n");
		result = 0;
		break;
	case OTA_STATUS_DEPLOY:
		NOTICE("OTA_STATUS_DEPLOY\n");
		result = 1;
		break;
	default:
		NOTICE("OTA_STATUS_INIT\n");
		result = 0;
	}

	wdt_init();

	lresult = result;
	return result;
}
