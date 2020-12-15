#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters
BOOT_MODE	:= 	flexspi_nor
BOARD		:=	lx2162aqds
WARM_BOOT	:=	yes
NXP_COINED_BB	:=	no
POVDD_ENABLE	:=	no

# DDR Errata
ERRATA_DDR_A050450 := 1

ifeq (${POVDD_ENABLE},yes)
$(eval $(call add_define,CONFIG_POVDD_ENABLE))
endif

include plat/nxp/soc-lx2160/lx2162aqds/platform.def
$(eval $(call add_define,CONFIG_${FLASH_TYPE}))
$(eval $(call add_define,NXP_FLEXSPI_FLASH_SIZE))
$(eval $(call add_define_val,CONFIG_FSPI_ERASE_4K,${CONFIG_FSPI_ERASE_4K}))
$(eval $(call add_define,NXP_FLEXSPI_FLASH_UNIT_SIZE))
$(eval $(call add_define_val,WARM_BOOT_FLAG_BASE_ADDR,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - 2 * ${NXP_XSPI_NOR_UNIT_SIZE}'))
$(eval $(call add_define_val,PHY_TRAINING_REGS_ON_FLASH,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - ${NXP_XSPI_NOR_UNIT_SIZE}'))

# Process ERRATA_DDR_A050450 flag
ifeq (${ERRATA_DDR_A050450}, 1)
$(eval $(call add_define,ERRATA_DDR_A050450))
endif

# get SoC common build parameters
include plat/nxp/soc-lx2160/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c	\
			${BOARD_PATH}/platform.c
