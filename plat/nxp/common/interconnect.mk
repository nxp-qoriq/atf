# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#
#------------------------------------------------------------------------------
#
# Select the Interconnect files
#
# -----------------------------------------------------------------------------

ifeq (, $(filter $(INTERCONNECT), CCI400 CCN502 CCN504 CCN508))
    $(error -> Interconnect type not set!)
else
ifeq ($(INTERCONNECT), CCI400)
INTERCONNECT_SOURCES	:= 	drivers/arm/cci/cci.c 		\
				${PLAT_COMMON_PATH}/ls_cci.c
else
ifeq ($(INTERCONNECT), $(filter $(INTERCONNECT), CCN502 CCN504 CCN508))
INTERCONNECT_SOURCES	:= 	drivers/arm/ccn/ccn.c 		\
				${PLAT_COMMON_PATH}/ls_ccn.c
endif
endif
endif

# -----------------------------------------------------------------------------

