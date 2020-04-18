/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>

#include <plat_common.h>

/*
 * Error handler
 */
void plat_error_handler(int err)
{
	switch (err) {
	case -ENOENT:
	case -EAUTH:
		printf("Authentication failure\n");
		break;
	default:
		/* Unexpected error */
		break;
	}

	/* Loop until the watchdog resets the system */
	for (;;)
		wfi();
}
