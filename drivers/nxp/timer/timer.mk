#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

TIMER_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/timer

PLAT_INCLUDES		+= -I$(TIMER_DRIVERS_PATH)
PLAT_BL_COMMON_SOURCES	+= drivers/delay_timer/delay_timer.c	\
			   $(PLAT_DRIVERS_PATH)/timer/nxp_timer.c

