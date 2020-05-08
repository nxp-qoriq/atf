#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

FLEXSPI_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/flexspi/nor

PLAT_XSPI_INCLUDES	+= -I$(FLEXSPI_DRIVERS_PATH)

XSPI_BOOT_SOURCES	+= $(FLEXSPI_DRIVERS_PATH)/flexspi_nor.c	\
			   ${FLEXSPI_DRIVERS_PATH}/fspi.c
ifeq ($(DEBUG),1)
XSPI_BOOT_SOURCES	+= ${FLEXSPI_DRIVERS_PATH}/test_fspi.c
endif

PLAT_XSPI_INCLUDES	+= -Iinclude/drivers/nxp/flexspi
