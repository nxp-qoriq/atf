#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Rajesh Bhagat <rajesh.bhagat@nxp.com>

# board-specific build parameters
BOOT_MODE	:= 	flexspi_nor
BOARD		:=	rdb

 # get SoC common build parameters
include plat/nxp/soc-la1224/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

