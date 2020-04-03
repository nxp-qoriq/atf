#
# Copyright 2017-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------

# MMDC ddr cntlr driver files

DDR_DRIVERS_PATH	:=	drivers/nxp/ddr

DDR_CNTLR_SOURCES	=	${DDR_DRIVERS_PATH}/fsl-mmdc/fsl_mmdc.c
PLAT_INCLUDES		+=	-I$(DDR_DRIVERS_PATH)/nxp_ddr/ddr_io.h
PLAT_INCLUDES		+=	-I$(DDR_DRIVERS_PATH)/fsl-mmdc/fsl_mmdc.h
#------------------------------------------------

