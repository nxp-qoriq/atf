/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 */

#ifndef __ERRATA_H__
#define __ERRATA_H__

#include <sys/types.h>

void erratum_a008850_early(void);
void erratum_a008850_post(void);
void erratum_a009660(void);
void erratum_a010539(void);

#endif /* __ERRATA_H__ */
