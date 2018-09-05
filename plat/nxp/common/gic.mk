# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#
#------------------------------------------------------------------------------
#
# Select the GIC files
#
# -----------------------------------------------------------------------------

ifeq ($(GIC), GIC400)
GIC_SOURCES		+=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				${PLAT_COMMON_PATH}/ls_gicv2.c
else
ifeq ($(GIC), GIC500)
GIC_SOURCES		+=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				plat/common/plat_gicv3.c		\
				${PLAT_COMMON_PATH}/ls_gicv3.c
else
    $(error -> GIC type not set!)
endif
endif

# -----------------------------------------------------------------------------

