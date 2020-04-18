#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the CORE files
#
# -----------------------------------------------------------------------------

ifeq ($(CONSOLE), PL011)
CONSOLE_SOURCES  	:=      drivers/arm/pl011/aarch64/pl011_console.S	\
                                ${PLAT_COMMON_PATH}/console_pl011.c
else
    $(error -> CONSOLE not set!)
endif

# -----------------------------------------------------------------------------

