#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#

SEC_DRIVERS_PATH	:=	plat/nxp/drivers/fsl_sec

ifeq (${TRUSTED_BOARD_BOOT},1)
AUTH_SOURCES +=  $(wildcard $(SEC_DRIVERS_PATH)/src/auth/*.c)
endif
