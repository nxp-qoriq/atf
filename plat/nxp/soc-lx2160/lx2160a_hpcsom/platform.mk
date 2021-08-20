#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters
BOOT_MODE	:= 	sd
BOARD		:=	lx2160a_hpcsom
POVDD_ENABLE	:=	no

ifeq (${POVDD_ENABLE},yes)
$(eval $(call add_define,CONFIG_POVDD_ENABLE))
endif

include plat/nxp/soc-lx2160/lx2160a_hpcsom/platform.def

# get SoC common build parameters
include plat/nxp/soc-lx2160/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c	\
			${BOARD_PATH}/platform.c

