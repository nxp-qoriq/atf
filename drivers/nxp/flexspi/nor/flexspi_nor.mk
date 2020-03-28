#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

FLEXSPI_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/flexspi

PLAT_INCLUDES		+= -I$(FLEXSPI_DRIVERS_PATH)/nor

PLAT_BL_COMMON_SOURCES	+= $(FLEXSPI_DRIVERS_PATH)/nor/flexspi_nor.c
