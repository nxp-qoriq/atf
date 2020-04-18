# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the GIC files
#
# -----------------------------------------------------------------------------

ifeq ($(GIC), GIC500)
include drivers/arm/gic/v3/gicv3.mk
GIC_SOURCES		+=	${GICV3_SOURCES}
GIC_SOURCES		+=	${PLAT_COMMON_PATH}/ls_gicv3.c	\
				plat/common/plat_gicv3.c
else
    $(error -> GIC type not set!)
endif

# -----------------------------------------------------------------------------

