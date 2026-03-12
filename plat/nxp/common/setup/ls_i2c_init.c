#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <i2c.h>
#include <soc.h>

#define NXP_IIC1_ADDR	0x02000000
#define NXP_IIC2_ADDR	0x02010000
#define NXP_IIC3_ADDR	0x02020000
#define NXP_IIC4_ADDR	0x02030000
#define NXP_IIC5_ADDR	0x02040000
#define NXP_IIC6_ADDR	0x02050000
#define NXP_IIC7_ADDR	0x02060000
#define NXP_IIC8_ADDR	0x02070000

#define RCWSR12R			0x01e0012c
#define RCWSR12W			0x70010012c
#define RCWSR12_IIC2_PMUX_MASK		0x00000007 /* [0..2] */
#define RCWSR12_IIC2_PMUX_IIC2		0x00000000
#define RCWSR12_IIC2_PMUX_GPIO		0x00000001
#define RCWSR12_IIC3_PMUX_MASK		0x00000038 /* [3..5] */
#define RCWSR12_IIC3_PMUX_IIC3		0x00000000
#define RCWSR12_IIC3_PMUX_GPIO		0x00000008
#define RCWSR12_IIC4_PMUX_MASK		0x000001c0 /* [6..8] */
#define RCWSR12_IIC4_PMUX_IIC4		0x00000000
#define RCWSR12_IIC4_PMUX_GPIO		0x00000040
#define RCWSR12_IIC5_PMUX_MASK		0x00000e00 /* [9..11] */
#define RCWSR12_IIC5_PMUX_IIC5		0x00000000
#define RCWSR12_IIC5_PMUX_GPIO		0x00000200
#define RCWSR12_IIC6_PMUX_MASK		0x00007000 /* [12..14] */
#define RCWSR12_IIC6_PMUX_IIC6		0x00000000
#define RCWSR12_IIC6_PMUX_GPIO		0x00001000
#define RCWSR13R			0x01e00130
#define RCWSR13W			0x700100130
#define RCWSR12_SDHC2_DAT74_PMUX_MASK	0x00000003
#define RCWSR12_SDHC2_DAT74_PMUX_SDHC2	0x00000000
#define RCWSR12_SDHC2_DAT74_PMUX_IIC78	0x00000001
#define RCWSR14R			0x01e00134
#define RCWSR14W			0x700100134
#define RCWSR14_IIC1_PMUX_MASK		0x00000400 /* [10] */
#define RCWSR14_IIC1_PMUX_IIC1		0x00000000
#define RCWSR14_IIC1_PMUX_GPIO		0x00000400

static void ls_i2c_flush_pca9547(uint8_t busno, const char *busname, uint8_t address, uint8_t channels);
static void ls_i2c_flush(uint8_t busno, const char *busname);

/**
 * Flush i2c buses to make slave devices release sda,
 * in case the system was reset during a transaction.
 */
void bl2_i2c_init() {
	const uint8_t bus_flush_list[] = {CONFIG_LX2160_FLUSH_IIC};
	/*
	 * List of muxes and channels to flush. Takes 2D array:
	 * {<bus number>, <mux chip address>, <channel bitmask>},
	 */
	const uint8_t mux_flush_list[][3] = {CONFIG_LX2160_FLUSH_IIC_MUX};
	const uintptr_t iic_base_addr[] = {
		NXP_IIC1_ADDR,
		NXP_IIC2_ADDR,
		NXP_IIC3_ADDR,
		NXP_IIC4_ADDR,
		NXP_IIC5_ADDR,
		NXP_IIC6_ADDR,
	};
	const char *iic_name[] = {
		"IIC1",
		"IIC2",
		"IIC3",
		"IIC4",
		"IIC5",
		"IIC6",
	};
	int i;
	uint8_t busno;
	uint8_t address;
	uint8_t channels;

	/* flush i2c buses */
	for (i = 0; i < ARRAY_SIZE(bus_flush_list); i++) {
		busno = bus_flush_list[i] - 1;
		i2c_init(iic_base_addr[busno]);
		ls_i2c_flush(busno, iic_name[busno]);
	}

	/* flush muxes channels */
	for (i = 0; i < ARRAY_SIZE(mux_flush_list); i++) {
		busno = mux_flush_list[i][0] - 1;
		address = mux_flush_list[i][1];
		channels = mux_flush_list[i][2];
		i2c_init(iic_base_addr[busno]);
		ls_i2c_flush_pca9547(busno, iic_name[busno], address, channels);
	}
}

static void ls_i2c_flush_pca9547(uint8_t busno, const char *busname, uint8_t chip, uint8_t channels) {
	uint8_t channel, creg = 0;
	char buffer[64];
	int ret;

	/* try read configuration register */
	ret = i2c_read(chip, 0x00, 1, &creg, 1);
	if(ret != 0) {
		/* no device responding at address, skip */
		return;
	}

	/* after reset configuration register reads 0x08 */
	if(creg != 0x08) {
		/* probably not a pca9547, skip */
		return;
	}

	/* flush selected channels */
	for(uint8_t i = 8; i > 0; i--) {
		if (!(channels & (1 << (i-1))))
			continue;

		/* select channel i */
		channel = 0x08 | (i-1);
		i2c_write(chip, 0x00, 1, &channel, 1);

		/* flush channel */
		snprintf(buffer, sizeof(buffer), "%s mux@%02x channel %u", busname, chip, i-1);
		ls_i2c_flush(busno, buffer);
	}
}

static struct i2c_bus_info {
	uintptr_t pinmux_addr_r;
	uintptr_t pinmux_addr_w;
	uint32_t pinmux_mask;
	uint32_t pinmux_sel;
	uintptr_t gpio_addr;
	uint8_t gpio_scl;
	uint8_t gpio_sda;
} ls_i2c_bus_info[] = {
	{
		.pinmux_addr_r = RCWSR14R,
		.pinmux_addr_w = RCWSR14W,
		.pinmux_mask = RCWSR14_IIC1_PMUX_MASK,
		.pinmux_sel = RCWSR14_IIC1_PMUX_GPIO,
		.gpio_addr = NXP_GPIO1_ADDR,
		.gpio_scl = 3, /* GPIO1_DAT03 */
		.gpio_sda = 2, /* GPIO1_DAT02 */
	},
	{
		.pinmux_addr_r = RCWSR12R,
		.pinmux_addr_w = RCWSR12W,
		.pinmux_mask = RCWSR12_IIC2_PMUX_MASK,
		.pinmux_sel = RCWSR12_IIC2_PMUX_GPIO,
		.gpio_addr = NXP_GPIO1_ADDR,
		.gpio_scl = 31, /* GPIO1_DAT31 */
		.gpio_sda = 30, /* GPIO1_DAT30 */
	},
	{
		.pinmux_addr_r = RCWSR12R,
		.pinmux_addr_w = RCWSR12W,
		.pinmux_mask = RCWSR12_IIC3_PMUX_MASK,
		.pinmux_sel = RCWSR12_IIC3_PMUX_GPIO,
		.gpio_addr = NXP_GPIO1_ADDR,
		.gpio_scl = 29, /* GPIO1_DAT29 */
		.gpio_sda = 28, /* GPIO1_DAT28 */
	},
	{
		.pinmux_addr_r = RCWSR12R,
		.pinmux_addr_w = RCWSR12W,
		.pinmux_mask = RCWSR12_IIC4_PMUX_MASK,
		.pinmux_sel = RCWSR12_IIC4_PMUX_GPIO,
		.gpio_addr = NXP_GPIO1_ADDR,
		.gpio_scl = 27, /* GPIO1_DAT27 */
		.gpio_sda = 26, /* GPIO1_DAT26 */
	},
	{
		.pinmux_addr_r = RCWSR12R,
		.pinmux_addr_w = RCWSR12W,
		.pinmux_mask = RCWSR12_IIC5_PMUX_MASK,
		.pinmux_sel = RCWSR12_IIC5_PMUX_GPIO,
		.gpio_addr = NXP_GPIO1_ADDR,
		.gpio_scl = 25, /* GPIO1_DAT25 */
		.gpio_sda = 24, /* GPIO1_DAT24 */
	},
	{
		.pinmux_addr_r = RCWSR12R,
		.pinmux_addr_w = RCWSR12W,
		.pinmux_mask = RCWSR12_IIC6_PMUX_MASK,
		.pinmux_sel = RCWSR12_IIC6_PMUX_GPIO,
		.gpio_addr = NXP_GPIO1_ADDR,
		.gpio_scl = 23, /* GPIO1_DAT23 */
		.gpio_sda = 22, /* GPIO1_DAT22 */
	},
	{
		.pinmux_addr_r = RCWSR13R,
		.pinmux_addr_w = RCWSR13W,
		.pinmux_mask = RCWSR12_SDHC2_DAT74_PMUX_MASK,
		.pinmux_sel = RCWSR12_SDHC2_DAT74_PMUX_IIC78,
		.gpio_addr = NXP_GPIO2_ADDR,
		.gpio_scl = 16, /* GPIO2_DAT16 */
		.gpio_sda = 15, /* GPIO2_DAT15 */
	},
	{
		.pinmux_addr_r = RCWSR13R,
		.pinmux_addr_w = RCWSR13W,
		.pinmux_mask = RCWSR12_SDHC2_DAT74_PMUX_MASK,
		.pinmux_sel = RCWSR12_SDHC2_DAT74_PMUX_IIC78,
		.gpio_addr = NXP_GPIO2_ADDR,
		.gpio_scl = 18, /* GPIO2_DAT18 */
		.gpio_sda = 17, /* GPIO2_DAT17 */
	},
};

/*
 * Flush the i2c bus through any muxes with 9 clock cycles
 * to ensure all slave devices release their locks on SDA.
 * This is a work-around for i2c slave devices locking SDA,
 * when the system has been reset during a transaction.
 *
 * The implementation is inspired by LX2160A Chip Errata 07/2020 A-010650.
 */
static void ls_i2c_flush(uint8_t busno, const char *busname) {
	struct i2c_bus_info *info;
	uintptr_t gpdir_addr, gpodr_addr, gpdat_addr;
	uint32_t pinmux, gpdir, gpodr, gpdat;
	struct {
		uint32_t pinmux, gpdir, gpodr, gpdat;
	} backup;
	uint32_t scl_mask, sda_mask;

	if(busno >= 8) {
		ERROR("failed to flush i2c bus %u %s: invalid bus number!\n", busno, busname);
		return;
	}
	/* load i2c bus specific information */
	info = &ls_i2c_bus_info[busno];
	gpdir_addr = info->gpio_addr + 0x0;
	gpodr_addr = info->gpio_addr + 0x4;
	gpdat_addr = info->gpio_addr + 0x8;
	scl_mask = 0x80000000 >> info->gpio_scl;
	sda_mask = 0x80000000 >> info->gpio_sda;

	/* backup configuration registers */
	pinmux = backup.pinmux = mmio_read_32(info->pinmux_addr_r);
	gpdir = backup.gpdir = mmio_read_32(gpdir_addr);
	gpodr = backup.gpodr = mmio_read_32(gpodr_addr);
	gpdat = backup.gpdat = mmio_read_32(gpdat_addr);

	/* configure SCL+SDA as GPIOs */
	pinmux = (pinmux & ~info->pinmux_mask) | info->pinmux_sel;
	mmio_write_32(info->pinmux_addr_w, pinmux);

	/* configure SCL+SDA as output open drain */
	gpdir |= scl_mask | sda_mask;
	gpodr |= scl_mask | sda_mask;
	gpdat |= scl_mask | sda_mask;
	mmio_write_32(gpdat_addr, gpdat);
	mmio_write_32(gpodr_addr, gpodr);
	mmio_write_32(gpdir_addr, gpdir);

	/* allow short delay for changes to propagate */
	udelay(10);

	/*
	 * reliable detection of blocked bus is hard
	 * because sda depends on the last sent bit.
	 * Flush unconditionally instead.
	 */

	VERBOSE("flushing i2c bus %u (%s)\n", busno, busname);

	/* toggle clock 9 times */
	for(uint8_t i = 0; i < 9; i++) {
		mmio_write_32(gpdat_addr, gpdat & ~scl_mask);
		udelay(10);
		mmio_write_32(gpdat_addr, gpdat | scl_mask);
		udelay(10);
	}

	/* restore configuration registers */
	mmio_write_32(gpdir_addr, backup.gpdir);
	mmio_write_32(gpodr_addr, backup.gpodr);
	mmio_write_32(gpdat_addr, backup.gpdat);
	mmio_write_32(info->pinmux_addr_w, backup.pinmux);

	return;
}
