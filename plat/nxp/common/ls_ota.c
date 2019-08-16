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

	esdhc_emmc_init(mmc);

	esdhc_read(mmc, OTA_OFFSET, buffer, OTA_SIZE);

	NOTICE("OTA Status = %c\n", data.status);
	switch (data.status) {
	case OTA_STATUS_ONLINE:
		result = 1;
		break;
	case OTA_STATUS_INSTALLED:
		result = 1;
		data.status = OTA_STATUS_FAILED;
		esdhc_write(mmc, buffer, OTA_OFFSET, OTA_SIZE);
		esdhc_read(mmc, OTA_OFFSET, buffer, OTA_SIZE);
		NOTICE("Update OTA Status = %c\n", data.status);
		break;
	case OTA_STATUS_FAILED:
		result = 0;
		break;
	case OTA_STATUS_IDLE:
		result = 0;
		break;
	case OTA_STATUS_INSTALL:
		result = 0;
		break;
	case OTA_STATUS_DEPLOY:
		result = 1;
		break;
	default:
		result = 0;
	}

	wdt_init();

	lresult = result;
	return result;
}
