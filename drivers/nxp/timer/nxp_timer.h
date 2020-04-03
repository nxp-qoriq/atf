/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#
#ifndef NXP_TIMER_H
#define NXP_TIMER_H

/* Ticks elapsed in one second by a signal of 1 KHz */
#define KHZ_TICKS_PER_SEC 1000

uint64_t get_timer_val(uint64_t start);

/*
 * Initialise the nxp on-chip free rolling usec counter as the delay
 * timer.
 */
void delay_timer_init(uintptr_t nxp_timer_addr);

#endif /* NXP_TIMER_H */
