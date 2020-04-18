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
GIC_SOURCES		+=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/gicdv3_helpers.c	\
				drivers/arm/gic/v3/gicrv3_helpers.c	\
				plat/common/plat_gicv3.c		\
				${PLAT_COMMON_PATH}/ls_gicv3.c
else
    $(error -> GIC type not set!)
endif

# -----------------------------------------------------------------------------

