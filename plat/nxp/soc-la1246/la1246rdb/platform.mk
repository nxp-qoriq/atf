#
# Copyright 2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Parth Girishkumar Bera <parthgirishkumar.bera@nxp.com>

# board-specific build parameters
BOOT_MODE	:= qspi
BOARD		:= rdb

 # get SoC common build parameters
include plat/nxp/soc-la1246/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

