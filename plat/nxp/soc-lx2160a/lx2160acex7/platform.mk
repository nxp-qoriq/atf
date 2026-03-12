#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters

BOOT_MODE	?= 	flexspi_nor
BOARD		?=	lx2160acex7
POVDD_ENABLE	:=	no
NXP_COINED_BB	:=	no

 # DDR Compilation Configs
NUM_OF_DDRC	:=	2
DDRC_NUM_DIMM	:=	2
DDRC_NUM_CS	:=	4
DDR_ECC_EN	:=	yes
 #enable address decoding feature
DDR_ADDR_DEC	:=	yes
APPLY_MAX_CDD	:=	yes

# S5 GPIO
LX2160A_S5_GPIO_ADDR := NXP_GPIO3_ADDR
LX2160A_S5_GPIO := 7

# I2C Bus Flushing: IIC1
LX2160_FLUSH_IIC := 1,
# I2C Mux Flushing: IIC1: PCA9547@77: Channel 0 (SPD EEPROM)
LX2160_FLUSH_IIC_MUX := { 1, 0x77, 0x01 },

# DDR Errata
ERRATA_DDR_A011396	:= 1
ERRATA_DDR_A050450	:= 1

 # On-Board Flash Details
FLASH_TYPE	:=	MT35XU512A
XSPI_FLASH_SZ	:=	0x10000000
NXP_XSPI_NOR_UNIT_SIZE		:=	0x20000
BL2_BIN_XSPI_NOR_END_ADDRESS	:=	0x100000
# CONFIG_FSPI_ERASE_4K is required to erase 4K sector sizes. This
# config is enabled for future use cases.
FSPI_ERASE_4K	:= 0

 # Platform specific features.
WARM_BOOT	:=	no

 # Adding Platform files build files
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c\
			${BOARD_PATH}/platform.c

SUPPORTED_BOOT_MODE	:=	flexspi_nor	\
				sd		\
				emmc		\
				auto

# Adding platform board build info
include plat/nxp/common/plat_make_helper/plat_common_def.mk

 # Adding SoC build info
include plat/nxp/soc-lx2160a/soc.mk
