#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------

SFP_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/sfp

PLAT_INCLUDES		+= -I$(SFP_DRIVERS_PATH)

PLAT_BL_COMMON_SOURCES	+= $(SFP_DRIVERS_PATH)/sfp.c

#------------------------------------------------
