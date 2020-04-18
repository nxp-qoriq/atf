# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the Interconnect files
#
# -----------------------------------------------------------------------------

ifeq (, $(filter $(INTERCONNECT), CCI400 CCN502 CCN504 CCN508))
    $(error -> Interconnect type not set!)
else
ifeq ($(INTERCONNECT), $(filter $(INTERCONNECT), CCN502 CCN504 CCN508))
INTERCONNECT_SOURCES	:= 	drivers/arm/ccn/ccn.c 		\
				${PLAT_COMMON_PATH}/ls_ccn.c
else
ifeq ($(INTERCONNECT), CCI400)
    $(error -> Interconnect type not supported!)
endif
endif
endif

# -----------------------------------------------------------------------------

