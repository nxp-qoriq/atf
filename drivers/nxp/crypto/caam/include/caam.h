/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CAAM_H
#define CAAM_H

#include "caam_io.h"
#include "sec_jr_driver.h"

#include <platform_def.h>

/* Job ring 3 is reserved for usage by sec firmware */
#define DEFAULT_JR	3

#if defined(CONFIG_CHASSIS_3_2)
#define CAAM_JR0_OFFSET			0x10000
#define CAAM_JR1_OFFSET			0x20000
#define CAAM_JR2_OFFSET			0x30000
#define CAAM_JR3_OFFSET			0x40000
#endif

enum sig_alg {
	RSA,
	ECC
};

/* This function does basic SEC Initialization */
int sec_init(uintptr_t nxp_caam_addr);

/* This function is used to submit jobs to JR */
int run_descriptor_jr(struct job_descriptor *desc);

/* This function is used to instatiate the HW RNG is already not instantiated */
int hw_rng_instantiate(uintptr_t nxp_caam_addr);

/* This function is used to return random bytes of byte_len from HW RNG */
int get_rand_bytes_hw(uint8_t *bytes, int byte_len, uintptr_t nxp_caam_addr);

/* This function is used to set the hw unique key from HW CAAM */
int get_hw_unq_key_blob_hw(uint8_t *hw_key, int size);

/* This function is used to fetch random number from
 * CAAM of length either of 4 bytes or 8 bytes depending
 * rngWidth value.
 */
unsigned long long get_random(int rngWidth, uintptr_t nxp_caam_addr);

#endif /* CAAM_H */
