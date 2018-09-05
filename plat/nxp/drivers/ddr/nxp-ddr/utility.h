/*
 * Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author York Sun <york.sun@nxp.com>
 */

#include <plat_common.h>

unsigned long get_ddr_freq(struct sysinfo *sys, int ctrl_num);
unsigned int get_memory_clk_ps(unsigned long clk);
unsigned int picos_to_mclk(unsigned long data_rate, unsigned int picos);
unsigned int get_ddrc_version(const struct ccsr_ddr *ddr);
void print_ddr_info(struct ccsr_ddr *ddr);
