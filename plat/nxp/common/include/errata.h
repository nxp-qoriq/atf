/*
 * Copyright 2018, 2020 NXP
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
#ifdef ERRATA_PLAT_A050426
void erratum_a050426(void);
#endif

#endif /* __ERRATA_H__ */
