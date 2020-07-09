#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Pankit Garg <pankit.garg@nxp.com>

# board-specific build parameters
BOOT_MODE	:= qspi
BOARD		:= adb

 # get SoC common build parameters
include plat/nxp/soc-ls1046/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

