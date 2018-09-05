#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#
#------------------------------------------------------------------------------
#
# Select the CORE files
#
# -----------------------------------------------------------------------------

ifeq ($(CONSOLE), NS16550)
CONSOLE_SOURCES		:= 	$(PLAT_DRIVERS_PATH)/uart/16550_console.S	\
				drivers/console/aarch64/console.S		\
				$(PLAT_COMMON_PATH)/console_16550.c
else
ifeq ($(CONSOLE), PL011)
CONSOLE_SOURCES  	:=      drivers/arm/pl011/aarch64/pl011_console.S	\
                                ${PLAT_COMMON_PATH}/console_pl011.c
else
    $(error -> CONSOLE not set!)
endif
endif

# -----------------------------------------------------------------------------

