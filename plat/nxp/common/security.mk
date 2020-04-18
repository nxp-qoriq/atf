#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#------------------------------------------------------------------------------
#
# Select the SECURITY files
#
# -----------------------------------------------------------------------------

ifeq ($(TZC_ID), TZC400)
SECURITY_SOURCES 	+=	drivers/arm/tzc/tzc400.c
else ifeq ($(TZC_ID), NONE)
    $(info -> No TZC present on platform)
else
    $(error -> TZC type not set!)
endif

#GET SEC_SOURCES defined
include $(PLAT_DRIVERS_PATH)/crypto/caam/caam.mk

SECURITY_SOURCES	+= ${SEC_SOURCES}

# -----------------------------------------------------------------------------

