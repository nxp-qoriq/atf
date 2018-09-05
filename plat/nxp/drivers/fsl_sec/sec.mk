#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#

SEC_DRIVERS_PATH	:=	plat/nxp/drivers/fsl_sec

SEC_SOURCES +=  $(wildcard $(SEC_DRIVERS_PATH)/src/*.c)

PLAT_INCLUDES			+= -I$(SEC_DRIVERS_PATH)/include
