#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${SFP_ADDED},)

SFP_ADDED		:= 1
$(eval $(call add_define, NXP_SFP_ENABLED))

SFP_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/sfp

PLAT_INCLUDES		+= -I$(SFP_DRIVERS_PATH)

PLAT_BL_COMMON_SOURCES	+= $(SFP_DRIVERS_PATH)/sfp.c

ifeq (${FUSE_PROG}, 1)
BL2_SOURCES		+= $(SFP_DRIVERS_PATH)/fuse_prov.c
endif

endif
#------------------------------------------------
