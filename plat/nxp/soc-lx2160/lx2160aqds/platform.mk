#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters
BOOT_MODE	:= 	flexspi_nor
BOARD		:=	aqds

include plat/nxp/soc-lx2160/lx2160aqds/platform.def
$(eval $(call add_define,CONFIG_${FLASH_TYPE}))
$(eval $(call add_define,NXP_FLEXSPI_FLASH_SIZE))
$(eval $(call add_define_val,CONFIG_FSPI_ERASE_4K,${CONFIG_FSPI_ERASE_4K}))

# get SoC common build parameters
include plat/nxp/soc-lx2160/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c
