/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Alison Wang <alison.wang@nxp.com>
 */

#ifndef __LS_OTA_H__
#define __LS_OTA_H__

#define OTA_OFFSET	0x3F00000
#define OTA_SIZE	0x200

enum ota_status {
	OTA_STATUS_ONLINE = '0',
	OTA_STATUS_INSTALLED = '1',
	OTA_STATUS_FAILED = '2',
	OTA_STATUS_IDLE = '3',
	OTA_STATUS_INSTALL = '4',
	OTA_STATUS_DEPLOY = '8',
};

struct OTA_parameter {
	char status;
	char str[OTA_SIZE - 1];
};

int ota_status_check(void);


#endif /* __LS_OTA_H__ */
