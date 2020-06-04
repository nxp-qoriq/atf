/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#include <common/debug.h>
#include <lib/mmio.h>

#include <platform_def.h>

/* GPIO Register offset */
#define GPDIR_REG_OFFSET                0x0
#define GPDAT_REG_OFFSET                0x8

#define ERROR_GPIO_SET         0x14
#define ERROR_GPIO_CLEAR       0x15

#ifdef NXP_GPIO_BE
#define gpio_read32(a)           bswap32(mmio_read_32((uintptr_t)(a)))
#define gpio_write32(a, v)       mmio_write_32((uintptr_t)(a), bswap32(v))
#elif defined(NXP_GPIO_LE)
#define gpio_read32(a)           mmio_read_32((uintptr_t)(a))
#define gpio_write32(a, v)       mmio_write_32((uintptr_t)(a), (v))
#else
#error Please define CCSR SFP register endianness
#endif


/* This function set GPIO pin for raising POVDD. */
int set_gpio_bit(uint32_t *gpio_base_addr,
		    uint32_t bit_num)
{
	uint32_t val = 0;
	uint32_t *gpdir = NULL;
	uint32_t *gpdat = NULL;

	gpdir = gpio_base_addr + GPDIR_REG_OFFSET;
	gpdat = gpio_base_addr + GPDAT_REG_OFFSET;

	/*
	 * Set the corresponding bit in direstion register
	 * to configure the GPIO as output.
	 */
	val = gpio_read32(gpdir);
	val = val | bit_num;
	gpio_write32(gpdir, val);

	/* Set the corresponding bit in GPIO data register */
	val = gpio_read32(gpdat);
	val = val | bit_num;
	gpio_write32(gpdat, val);

	val = gpio_read32(gpdat);

	if (!(val & bit_num))
		return ERROR_GPIO_SET;

	return 0;
}

/* This function reset GPIO pin set for raising POVDD. */
int clr_gpio_bit(uint32_t *gpio_base_addr,
		    uint32_t bit_num)
{
	uint32_t val = 0;
	uint32_t *gpdir = NULL;
	uint32_t *gpdat = NULL;

	gpdir = gpio_base_addr + GPDIR_REG_OFFSET;
	gpdat = gpio_base_addr + GPDAT_REG_OFFSET;

	/*
	 * Reset the corresponding bit in direction and data register
	 * to configure the GPIO as input.
	 */
	val = gpio_read32(gpdat);
	val = val & ~(bit_num);
	gpio_write32(gpdat, val);

	val = gpio_read32(gpdat);


	val = gpio_read32(gpdir);
	val = val & ~(bit_num);
	gpio_write32(gpdir, val);

	val = gpio_read32(gpdat);

	if (val & bit_num)
		return ERROR_GPIO_CLEAR;

	return 0;
}

uint32_t *select_gpio_n_bitnum(uint32_t povdd_gpio, uint32_t *bit_num)
{
	uint32_t *ret_gpio;
	uint32_t povdd_gpio_val = 0;
	uint32_t gpio_num = 0;

	/*
	 * Subtract 1 from fuse_hdr povdd_gpio value as
	 * for 0x1 value, bit 0 is to be set
	 * for 0x20 value i.e 32, bit 31 i.e. 0x1f is to be set.
	 * 0x1f - 0x00 : GPIO_1
	 * 0x3f - 0x20 : GPIO_2
	 * 0x5f - 0x40 : GPIO_3
	 * 0x7f - 0x60 : GPIO_4
	 */
	povdd_gpio_val = (povdd_gpio - 1) & GPIO_SEL_MASK;

	/* Right shift by 5 to divide by 32 */
	gpio_num = povdd_gpio_val >> 5;
	*bit_num = 1 << (31 - (povdd_gpio_val & GPIO_BIT_MASK));

	switch (gpio_num) {
#ifdef NXP_GPIO1_ADDR
	case 0:
		ret_gpio = (uint32_t *) NXP_GPIO1_ADDR;
		break;
#endif
#ifdef NXP_GPIO2_ADDR
	case 1:
		ret_gpio = (uint32_t *) NXP_GPIO2_ADDR;
		break;
#endif
#ifdef NXP_GPIO3_ADDR
	case 2:
		ret_gpio = (uint32_t *) NXP_GPIO3_ADDR;
		break;
#endif
#ifdef NXP_GPIO4_ADDR
	case 3:
		ret_gpio = (uint32_t *) NXP_GPIO4_ADDR;
		break;
#endif
	default:
		ret_gpio = NULL;
	}

	return ret_gpio;
}
