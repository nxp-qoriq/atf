/*
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DDR_IO_H
#define DDR_IO_H

#include <lib/mmio.h>

#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

/* macro for memory barrier */
#define mb()		asm volatile("dsb sy" : : : "memory")

#ifdef NXP_DDR_BE
#define ddr_in32(a)			bswap32(mmio_read_32((uintptr_t)(a)))
#define ddr_out32(a, v)			mmio_write_32((uintptr_t)(a),\
							bswap32(v))
#elif defined(NXP_DDR_LE)
#define ddr_in32(a)			mmio_read_32((uintptr_t)(a))
#define ddr_out32(a, v)			mmio_write_32((uintptr_t)(a), v)
#else
#error Please define CCSR DDR register endianness
#endif

#define ddr_setbits32(a, v)		ddr_out32((a), ddr_in32(a) | (v))
#define ddr_clrbits32(a, v)		ddr_out32((a), ddr_in32(a) & ~(v))
#define ddr_clrsetbits32(a, c, s)	ddr_out32((a), (ddr_in32(a) & ~(c)) \
						  | (s))

#endif /*	DDR_IO_H	*/
